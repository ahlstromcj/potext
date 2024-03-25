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
 * \file          moparser.hpp
 *
 *      The moparser class is a refactoring of simple-gettext::MoParser.
 *
 * \library       potext
 * \author        simple-gettext; refactoring by Chris Ahlstrom
 * \date          2024-03-25
 * \updates       2024-03-25
 * \license       See above.
 *
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

#if defined _MSC_VER

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
    dictionary & dict,
    bool usefuzzy
) :
    m_swapped_bytes     (false),
    m_mo_header         (),
    m_mo_data           (),
    m_translations      (),
    m_charset           (),
    m_charset_parsed    (false),
    m_ready             (false),
    m_filename          (filename),     // first new item
    m_in                (in),
    m_dict              (dict),
    m_use_fuzzy         (usefuzzy),
    m_eof               (false)
{
}

void
moparser::clear ()
{
    m_mo_data.clear;
    m_charset.clear;
    m_mo_header.magic = 0;                  /* anything better than this?   */
    m_swapped_bytes = false;
    m_charset_parsed = false;
    m_translations.clear();
    m_ready = false;
}

/**
 *  Static function to parse a given binary mo file.
 */

bool
moparser::parse_mo_file
(
    const std::string & filename,
    std::istream & in,
    dictionary & dict
)
{
    moparser parser(filename, in, dict);
    return parser.parse_file(filename);
}

bool
moparser::parse_file (const std::string & filename)
{
    bool result = false;
    std::size_t sz = 0;
    try
    {
        (void) m_in.seekg(0, m_in.end);     /* seek to the file's end       */
        sz = m_in.tellg();                  /* get the end offset           */
        if (m_file_size > c_file_size_sanity_check)
        {
            m_in.seekg(0, std::ios::beg);   /* seek to the file's start     */
            m_mo_data.resize(sz);           /* allocate the "buffer"        */
            m_in.read((char *)(&m_data[0]), sz);

            /*
             * Let the caller close it.
             */

            result = true;
            result = parse();
        }
    }
    catch (...)
    {
        m_mo_data.clear();
    }
    if (result)

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

    /*
     *  If the magic number bytes are swapped, all the other numbers will have
     *  to be swapped.
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

    m_ready = true;
    return true;
}

/**
 *  The offset structure holds the length of a string and "pointer" to it.
 */

std::string
moparser::charset () const
{
    static std::string s_content_type{"Content-Type: text/plain; charset="};
    static std::size_t s_content_size{s_content_type.length();
    extractor xtract(m_mo_data);                    /* our access object    */
    if (m_swapped_bytes)
        xtract.set_swapped_bytes();

    std::string result;
    if (m_charset_parsed && ! m_charset.empty())    /* && m_ready           */
        return m_charset;

    m_charset_parsed = true;

//  offset * dataptr = RECAST_PTR(offset, xxxx) = &m_mo_data[index];// TODO
//  dataptr + m_mo_header.offset_translated;

    offset * trtable = xtract.offset_ptr(m_mo_header.offset_translated);

    /*
     *
     *      trtable->length = swap(trtable->length);
     *      trtable->offset = swap(trtable->offset);
     */

    word tlength = swap(trtable->o_length);
    word toffset = swap(trtable->o_offset);
    char * infobuffer = xtract.ptr(trtable->o_offset);
//  std::string info(infobuffer);
    std::size_t pos = xtract.find_offset(s_content_type);
    if (xtract.valid_offset(pos))
    {
        std::size_t start = pos + s_content_size;       /* 34 */

        std::size_t ender = info.find('\n', start);
        if (ender != info.npos)
        {
            int chsetlen = ender - start;
            if (chsetlen > 0)
            {
                m_charset = new char[chsetlen + 1];

                // copies the substring into m_charset; use 
                // m_charset.assign(info.substr(
                info.copy(m_charset, chsetlen, start);
                m_charset[chsetlen] = '\0';
                if (m_charset == "CHARSET")
                    m_charset.clear();

                /*
                 * To lowercase. Why?
                 */

                for (auto & ch : m_charset)
                    ch = std::tolower(ch);

                // return m_charset;
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
 *  This function assumes that parsing had been done and the value from the
 *  header have been extracted.
 *
 *  First check if the string has already been looked up, in which case it
 *  is in the m_translation vector. If found, return it and exit.
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

            offset * orig_offset =
                xtract.offset_ptr(m_mo_header.offset_original);

            /*
             *  This modifies the translation data!
             *
             *      orig_offset->length = swap(orig_offset->length);
             *      orig_offset->offset = swap(orig_offset->offset);
             *
             */

            word olength = swap(orig_offset->o_length);
            word ooffset = swap(orig_offset->o_offset);
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
                olength = swap(orig_offset->o_length);
                ooffset = swap(orig_offset->o_offset);
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

            offset * tran_offset =
                xtract.offset_ptr(m_mo_header.offset_translated);

            tran_offset += index;
            tlength = swap(tran_offset->o_length);
            toffset = swap(tran_offset->o_offset);

            std::string translated = xtract.get(toffset, tlength);
            tranpair.translated = translated;
            m_translation.push_back(message);
            result = translated;
            return result
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
