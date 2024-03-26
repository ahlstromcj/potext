/*
 *  This file is part of potext.
 *
 *  potext is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  potext is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with potext; if not, write to the Free Software Foundation, Inc., 59
 *  Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  See this project, from which this module was derived.
 *
 *      https://github.com/laurent22/simple-gettext
 */

/**
 * \file          moparser.cpp
 *
 *      The moparser class is a refactoring of simple-gettext::MoParser.
 *
 * \library       potext
 * \author        simple-gettext; refactoring by Chris Ahlstrom
 * \date          2024-03-25
 * \updates       2024-03-26
 * \license       See above.
 *
 * Format of the .mo File:
 *
 *      This list is distilled from the diagram on page 120 of the GNU
 *      Gettext PDF manual. Each data item is 4 bytes long, i.e. a word
 *      or int32_t value.
 *
 *          -   M = Magic number 0x950412de or swapped value.
 *          -   R = File format revision number.
 *          -   N = Number of message strings in the catalog.
 *          -   O = Offset of the table with the original message strings.
 *          -   T = Offset of the table with the translated message strings.
 *          -   Hs = Size of hashing table. (We ignore this hashing table.)
 *          -   Ho = Offset of hashing table. (We ignore this hashing table.)
 *
 *      At the offset O pointing to the original messages are N pairs of
 *      values:
 *
 *          -   Length of the string
 *          -   Offset of the string (the string is null-terminated)
 *
 *      At the offset T pointing to the translated messages are N pairs of
 *      values:
 *
 *          -   Length of the string
 *          -   Offset of the string (the string is null-terminated)
 */

#include <cctype>                       /* std::toupper() etc.              */
#include <cstring>                      /* std::strlen() etc.               */
#include <iostream>                     /* std::istream, std::ostream       */
#include <fstream>

#include "po/dictionary.hpp"            /* po::dictionary class             */
#include "po/extractor.hpp"             /* po::extractor class              */
#include "po/logstream.hpp"             /* po::logstream log-access funcs   */
#include "po/pluralforms.hpp"           /* po::pluralforms class            */
#include "po/moparser.hpp"              /* po::moparser class               */

#if defined _MSC_VER_DISABLED           /* someone else can try this        */

/*
 * C4345: behavior change: an object of POD type constructed with an
 * initializer of the form () will be default-initialized
 *
 * C4996: 'std::basic_string<_Elem,_Traits,_Ax>::copy': Function call with
 * parameters that may be unsafe - this call relies on the caller to check
 * that the passed values are correct. To disable this warning, use
 * -D_SCL_SECURE_NO_WARNINGS. See documentation on how to use Visual C++
 * 'Checked Iterators'
 */

#pragma warning(disable: 4996)
#pragma warning(disable: 4345)

#endif

/**
 *  We cannot enable translation in this module, because it leads to
 *  recursion and a stack overflow. Oh well, it was worth a try.
 *  Can still grab messages for a dictionary, if desired.
 */

#if ! defined PO_HAVE_GETTEXT_RECURSIVE
#define _(str)      str
#endif

namespace po
{

static const size_t c_file_size_sanity_check = 100;

/**
 *  The first two words serve the identification of the file. The magic number
 *  will always signal GNU MO files. The number is stored in the byte order
 *  used when the MO file was generated, so the magic number really is two
 *  numbers: 0x950412de and 0xde120495.
 */

const moparser::word moparser::sm_magic    = 0x950412de;
const moparser::word moparser::sm_magic_swapped = 0xde120495;

/**
 *  Principal constructor
 */

moparser::moparser
(
    const std::string & filename,
    std::istream & in,
    dictionary & dic,
    bool usefuzzy
) :
    pomoparserbase      (filename, in, dic, usefuzzy),
    m_swapped_bytes     (false),
    m_mo_header         (),
    m_mo_data           (),
    m_translations      (),
    m_charset           (),
    m_charset_parsed    (false),
    m_ready             (false)
{
    // no code
}

void
moparser::clear ()
{
    m_mo_data.clear();
    m_charset.clear();
    m_mo_header.magic = 0;                  /* anything better than this?   */
    m_swapped_bytes = false;
    m_charset_parsed = false;
    m_translations.clear();
    m_ready = false;
}

/**
 *  This function is exactly like extractor::swap().
 */

moparser::word
moparser::swap (word ui) const
{
	return m_swapped_bytes ?
        (ui << 24) | ((ui & 0xff00) << 8) | ((ui >> 8) & 0xff00) | (ui >> 24) :
            ui ;
}

/**
 *  Static function to parse a given binary mo file.
 */

bool
moparser::parse_mo_file
(
    const std::string & filename,
    std::istream & in,
    dictionary & dic
)
{
    moparser parser(filename, in, dic);
    return parser.parse_file(filename);
}

bool
moparser::parse_file (const std::string & /* filename */)
{
    bool result = false;
    try
    {
        (void) in_stream().seekg(0, in_stream().end);   /* seek to file end */

        std::size_t sz = in_stream().tellg();           /* get end offset   */
        if (sz > c_file_size_sanity_check)
        {
            in_stream().seekg(0, std::ios::beg);        /* seek to start    */
            m_mo_data.resize(sz);                       /* allocate buffer  */
            in_stream().read((char *)(&m_mo_data[0]), sz);

            /*
             * Let the caller close it.
             */

            result = parse();
        }
    }
    catch (...)
    {
        m_mo_data.clear();
    }
    return result;
}

/**
 *  This function assumes all the data from the .mo file had been read into
 *  the m_mo_data vector and we are at the beginning of that vector.
 */

bool
moparser::parse ()
{
    extractor xtract(m_mo_data);                    /* our access object    */
    header * hp = RECAST_PTR(header, xtract.ptr()); /* point to first byte  */
    if                                              /* invalid header       */
    (
        hp->magic != sm_magic && hp->magic != sm_magic_swapped
    )
    {
        clear();
        return false;
    }
    else
    {
        /*
         *  If magic bytes are swapped, all other numbers will need swapping.
         */

        m_swapped_bytes = hp->magic == sm_magic_swapped;
        m_mo_header.magic = swap(hp->magic);
        m_mo_header.revision = swap(hp->revision);
        m_mo_header.string_count = swap(hp->string_count);
        m_mo_header.offset_original = swap(hp->offset_original);
        m_mo_header.offset_translated = swap(hp->offset_translated);
        m_mo_header.size_hash_table = swap(hp->size_hash_table);
        m_mo_header.offset_hash_table = swap(hp->offset_hash_table);

        /*
         *  Read only for lookup. Transations not stored.
         */

        std::string cs = charset();
        m_ready = ! cs.empty();
        if (m_ready)
            m_ready = load();

        return m_ready;
    }
}

/**
 *  Extracts the character set name for the .mo file. This occurs in a section
 *  deep in the file:
 *
 *      Report-Msgid-Bugs-To:
 *      PO-Revision-Date: 2019-06-24 07:40+0000
 *      Last-Translator: Casper casper
 *      Language-Team: Spanish (http://www.transifex.com/xfce/.../language/es/)
 *      MIME-Version: 1.0
 *      Content-Type: text/plain; charset=UTF-8
 *      Content-Transfer-Encoding: 8bit
 *      Language: es
 *      Plural-Forms: nplurals=2; plural=(n != 1);
 *
 *  The offset structure holds the length of a string and "pointer" to it.
 */

std::string
moparser::charset () // const
{
    static std::string s_content_type{"Content-Type: text/plain; charset="};
    static std::size_t s_content_size{s_content_type.length()};     /* 34   */
    extractor xtract(m_mo_data);                    /* binary data access   */
    if (m_swapped_bytes)
        xtract.set_swapped_bytes();

    std::string result;
    if (m_charset_parsed && ! m_charset.empty())    /* && m_ready           */
    {
        return m_charset;
    }
    else
    {
        m_charset_parsed = true;
        std::size_t pos = xtract.find_offset(s_content_type);
        if (xtract.valid_offset(pos))
        {
            std::string cstemp = xtract.get_delimited(pos + s_content_size);
            if (! cstemp.empty())
            {
                if (m_charset == "CHARSET")
                {
                    // m_charset.clear();
                    // warning(_("Charset not found for .mo; fallback to UTF-8"));
                    m_charset = "UTF-8";
                }
                else
                    m_charset = cstemp;

                /*
                 * To lowercase. Why?
                 *
                 * for (auto & ch : m_charset)
                 *      ch = std::tolower(ch);
                 */

                result = m_charset;
                (void) converter().set_charsets(m_charset, dict().get_charset());
            }
        }
    }
    return result;
}

std::string
moparser::find (const std::string & target)
{
    std::string result;
    for (const auto & trpair : m_translations)
    {
        if (trpair.original == target)
        {
            result = trpair.translated;
            break;
        }
    }
    return result;
}

/**
 *  This function tries to get all of the message and translation pairs
 *  from the .mo file. See the translate() function for some comments
 *  about internal details.
 */

bool
moparser::load ()
{
    bool result = true;
    extractor xtract(m_mo_data);                    /* translation data     */
    if (m_swapped_bytes)
        xtract.set_swapped_bytes();

    /*
     * TODO: use word offset instead of ptr.
     */

    extractor::offset * orig =                      /* msg table start      */
        xtract.offset_ptr(m_mo_header.offset_original);

    extractor::offset * tran =                      /* translation start    */
        xtract.offset_ptr(m_mo_header.offset_translated);

    int count = int(m_mo_header.string_count);
    for (int index = 0; index < count; ++index, ++orig, ++tran)
    {
#if 0
        word olength = xtract.get_word(orig, 0);    /* pointer plus 0 words */
        word ooffset = xtract.get_word(orig, 1);    /* pointer plus 1 words */
        translation tranpair;
        tranpair.original = xxxx;
        tranpair.translated = xxxx;
#endif
    }
    return result;
}

/**
 *  This function assumes that parsing had been done and the value from the
 *  header have been extracted.
 *
 *  First check if the string has already been looked up, in which case it
 *  is in the m_translations vector. If found, return it and exit.
 */

std::string
moparser::translate (const std::string & original)
{
    std::string result;
    if (m_ready && ! original.empty())
    {
        std::string lookup = find(original);
        if (lookup.empty())
        {
            extractor xtract(m_mo_data);            /* translation data     */
            if (m_swapped_bytes)
                xtract.set_swapped_bytes();

            /*
             *  Look for the original message. The start of the original-string
             *  table is stored in header::offset_original. The start of
             *  the translated table is in header::offset_translated.
             */

            extractor::offset * orig_offset =
                xtract.offset_ptr(m_mo_header.offset_original);

            /*
             *  This modifies the translation data!
             *
             *      orig_offset->length = swap(orig_offset->length);
             *      orig_offset->offset = swap(orig_offset->offset);
             *
             */

//          word olength = swap(orig_offset->o_length);
//          word ooffset = swap(orig_offset->o_offset);
            int count = int(m_mo_header.string_count);
            bool found = false;
            int stringindex;
            for (stringindex = 0; stringindex < count; ++stringindex)
            {
                if (xtract.match(original, orig_offset->o_offset))
                {
                    found = true;
                    break;
                }

                /*
                 *  Next little original-offset structure pointer.
                 *  Don't modify the original.
                 *
                 *      orig_offset->length = swap(orig_offset->length);
                 *      orig_offset->offset = swap(orig_offset->offset);
                 */

                ++orig_offset;
//              olength = swap(orig_offset->o_length);
//              ooffset = swap(orig_offset->o_offset);
            }

            translation tranpair;
            tranpair.original = original;
            if (! found)                        /* no such original string  */
            {
                m_translations.push_back(tranpair);
                return result;                  /* empty                    */
            }

            /*
             * At this point, stringindex is the index of the message and
             * orig_offset points to the original message data. Get the
             * translated string and build and the output message structure.
             */

            extractor::offset * tran_offset =
                xtract.offset_ptr(m_mo_header.offset_translated);

            tran_offset += stringindex;
            word tlength = swap(tran_offset->o_length);
            word toffset = swap(tran_offset->o_offset);

            std::string translated = xtract.get(toffset, tlength);
            tranpair.translated = translated;
            m_translations.push_back(tranpair);
            result = translated;
            return result;
        }
        else
            result = lookup;
    }
    return result;
}

}               // namespace po

/*
 * moparser.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
