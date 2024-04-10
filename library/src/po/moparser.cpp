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
 *  See this project, from which this module was derived and extended.
 *
 *      https://github.com/laurent22/simple-gettext
 */

/**
 * \file          moparser.cpp
 *
 *      The moparser class is a refactoring and extension of simple-gettext's
 *      MoParser to somewhat match the handling used by po::poparser.
 *
 * \library       potext
 * \author        simple-gettext; refactoring by Chris Ahlstrom
 * \date          2024-03-25
 * \updates       2024-04-10
 * \license       See above.
 *
 * Format of the .mo File:
 *
 *      This list is distilled from the diagram on page 120 of the GNU
 *      Gettext PDF manual. Also see the GNU Gettext project header file
 *      gettext-runtime/intl/gmo.h.  Also see the file
 *      library/tests/mo/es/newt.hex, which breaks down the contents of the
 *      small newt.mo file.
 *
 *      Header:
 *
 *          Each data item is 4 bytes long, i.e. a word or int32_t value;
 *
 *          -   M = Magic number 0x950412de or swapped value 0xde130495.
 *          -   R = File format major+minor revision number.
 *          -   N = Number of message strings in the catalog.
 *          -   O = Offset of the table with the original message strings.
 *          -   T = Offset of the table with the translated message strings.
 *          -   Hs = Size of hashing table. (We ignore this hashing table.)
 *          -   Ho = Offset of hashing table. (We ignore this hashing table.)
 *          -   In the future there might might be more entries. In gmo.h
 *              we see that if the minor revision is > 0, then there are 5
 *              more words in the header.
 *
 *      At the offset O pointing to the original messages are N pairs of
 *      values:
 *
 *          -   Length of the string, does not count the NUL terminator
 *              unless the NUL separates a singular and plural original
 *              string.
 *          -   Offset of the string (normally NUL-terminated).
 *
 *          If the original string has a context, then the string holds the
 *          following, concatenated:
 *
 *              -   Context string
 *              -   EOT byte (0x04, U+0004)
 *              -   Original string
 *
 *          Plurals forms are stored as:
 *
 *              -   Original:
 *                  -   Original string msgid
 *                  -   NUL byte
 *                  -   Plural string msgid (but not used in lookup).
 *              -   Translated:
 *                  -   Translated string (singular)
 *                  -   NUL byte
 *                  -   Any number of:
 *                      -   Plural form
 *                      -   NUL byte
 *
 *          The length value for the string includes context, separator bytes,
 *          the original string, and the plural form(s).
 *
 *          The encoding of the strings can be any ASCII-compatible encoding
 *          (e.g. UTF-8, ISO-8859-1), which must be stated in Content-Type.
 *          Embedded NULs are useless, and currently .mo files do not use
 *          wide characters.
 *
 *      At the offset T pointing to the translated messages are N pairs of
 *      values:
 *
 *          -   Length of the string
 *          -   Offset of the string (the string is null-terminated)
 *
 * First message:
 *
 *      The first original message string can be empty. In the file
 *      library/tests/mo/es/newt.mo (see newt.hex for readability),
 *      the first message is empty and translates to information about the
 *      .mo file: "Project-Id-Version: ... X-Generator: Zanata3.8.2\n".
 *      The empty string in a PO file GNU gettext is usually translated into
 *      some system information attached to that particular MO file,
 *
 * Original plural form:
 *
 *      Plural forms are stored by letting the plural of the original string
 *      follow the singular of the original string, separated through a NUL
 *      byte. The length which appears in the string descriptor includes
 *      both. However, only the singular of the original string takes part in
 *      the hash table lookup.
 */

#include <iostream>                     /* std::istream, std::ostream       */

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
    pomoparserbase          (filename, in, dic, usefuzzy),
    m_swapped_bytes         (false),
    m_mo_header             (),
    m_mo_data               (),
    m_translations          (),
    m_charset               (),
    m_charset_parsed        (false),
    m_plural_forms          (),
    m_plural_forms_parsed   (false),
    m_ready                 (false)
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
    m_plural_forms_parsed = false;
    m_translations.clear();
    m_ready = false;
}

/**
 *  This function is exactly like extractor::swap(), for now.
 *
 *  Little-endian means that least-significant bytes (LSB) come first
 *  in memory. The x86 and x86_64 processors are little-endian.
 *
 *  Big-endian means that most-significant bytes (MSB) come first
 *  in memory. This includes the 68000, PowerPC, and Sparc processors.
 *
 *  The test file library/test/mo/es/newt.mo has values in little-endian.
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
 *
 *      -   Character-set. The load_charset_name() function sets m_charset
 *          and uses it to set the iconvert's character-set using
 *          dict().get_charset().
 *      -   Plural-forms. The load_plural_form_name() function sets the
 *          m_plural_forms string and calls dict().set_plural_forms().
 *      -   Translation table. The load_translations() function
 *          populates the m_translations vector. We need to add them all to the
 *          dictionary.
 *
 *  Questions:
 *
 *      -   What about the plural-forms translations? How are they loaded?
 *          We might need something better than a vector, or make
 *          transation::translated a vector!
 *      -   What about context?
 *
 *  What we decided was to have the translation structure contain the original
 *  and translated strings, plus the context (if present), plus a phraselist
 *  for the plurals, if present.
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
    bool result = parser.parse_file(filename);      /* fills m_translations */
    if (result)
    {
        for (const auto & tquad : parser.get_translations())
        {
            bool have_context = ! tquad.context.empty();
            bool have_plurals = ! tquad.plurals.empty();
            const iconvert & cvt = parser.converter();
            const std::string & msgid = tquad.original;

            /*
             * Not part of the lookup, but we can use it for reporting.
             */

            const std::string & pluralmsgid = tquad.original_plural;
            if (have_context && have_plurals)
            {
                const std::string & ctxt = tquad.context;
                phraselist msglist = parser.convert_list(tquad.plurals);
                result = dic.add(ctxt, msgid, pluralmsgid, msglist);
            }
            else if (have_context)
            {
                const std::string & ctxt = tquad.context;
                const std::string & msgstr = cvt.convert(tquad.translated);
                result = dic.add(ctxt, msgid, msgstr);
            }
            else if (have_plurals)
            {
                phraselist msglist = parser.convert_list(tquad.plurals);
                result = dic.add(msgid, pluralmsgid, msglist);
            }
            else
            {
                /*
                 * Like poparser, do the character-set conversion here.
                 */

                std::string converted = cvt.convert(tquad.translated);
                result = dic.add(msgid, converted);

#if defined PLATFORM_DEBUG_TMI
                std::cout
                    << "Added: '" << tquad.original << "' and '"
                    << converted << "'"
                    << std::endl
                    ;
#endif
                if (! result)
                    break;
            }
        }
    }
    return result;
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
    if                                              /* invalid header?      */
    (
        hp->magic != sm_magic &&
        hp->magic != sm_magic_swapped
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

        std::string cs = load_charset_name();
        bool result = ! cs.empty();
        if (result)
        {
            std::string pf = load_plural_form_name();
            result = ! pf.empty();
        }
        if (result)
            result = load_translations();       /* plus context and plurals */

        m_ready = true;                         /* don't try it again       */
        return result;
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
 *
 *  "ebcdic-international-500+euro" is the longest character-set name we
 *  found at
 *
 *      https://www.iana.org/assignments/character-sets/character-sets.xhtml
 *
 *  It's 29 characters.
 */

std::string
moparser::load_charset_name ()
{
    static std::string s_content_type{"Content-Type: text/plain; charset="};
    static std::size_t s_content_size{s_content_type.length()};     /* 34   */
    extractor xtract(m_mo_data);                    /* binary data access   */
    if (m_swapped_bytes)
        xtract.set_swapped_bytes();

    std::string result;
    if (m_charset_parsed && ! m_charset.empty())    /* && ready()           */
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
                    m_charset = "UTF-8";
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

/**
 *  Obtain the Plural-Forms string. It comes three lines after "Content-Type,
 */

std::string
moparser::load_plural_form_name ()
{
    /*
     *  Too much.
     *
     * static std::string s_plural_forms{"Plural-Forms: nplurals="};
     * static std::size_t s_plural_size{s_plural_forms.length()};      // 23
     */

    extractor xtract(m_mo_data);                    /* binary data access   */
    if (m_swapped_bytes)
        xtract.set_swapped_bytes();

    std::string result;
    if (m_plural_forms_parsed && ! m_plural_forms.empty())
    {
        return m_plural_forms;
    }
    else
    {
        static std::string s_plural_forms{"nplurals="};
        std::size_t pos = xtract.find_offset(s_plural_forms);
        if (xtract.valid_offset(pos))
        {
            /*
             * We want to grab the "nplurals=" as well
             *
             * std::string pftemp = xtract.get_delimited(pos + s_plural_size);
             */

            std::string pftemp = xtract.get_delimited(pos);
            if (! pftemp.empty())
            {
                /**
                 * Remove spaces from plural-forms description string.
                 * This matches the behavior in the poparser class.
                 */

                m_plural_forms.clear();
                for (auto c : pftemp)
                {
                    if (! std::isspace(c))
                        m_plural_forms += c;
                }
            }
            else
                m_plural_forms = "nplurals=1;plural=0";

            result = m_plural_forms;
            if (! result.empty())
            {
                pluralforms plural_forms = pluralforms::from_string(result);
                if (! plural_forms)
                {
                    warning(_("Unknown .mo Plural-Forms"));
                }
                else
                {
                    if (! dict().get_plural_forms())
                    {
                        dict().set_plural_forms(plural_forms);
                    }
                    else
                    {
                        if (dict().get_plural_forms() != plural_forms)
                        {
                            warning
                            (
                                _("Plural-Forms mismatch between .mo "
                                "file and dictionary")
                            );
                        }
                    }
                }
            }
        }
        m_plural_forms_parsed = true;
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
 *  about internal details. Also see the banner at the top of this module
 *  for how to get the context and plural translations.
 *
 *  The process for both the original and translated strings is:
 *
 *      -   Get the offset of the table.
 *      -   Starting at this offset, for each index in the string-count,
 *          get the length of the string, then the offset of the string.
 *      -   At the string offset, grab the number of bytes specified.
 *      -   Increment to the next string. See the first for-loop below.
 *
 *  Note that the plural form of the original (msgid) string is stored
 *  with the original, but is not used for lookup. Also note that a context
 *  string plus an EOT character comes before the original string, if
 *  present.
 */

bool
moparser::load_translations ()
{
    static const char s_NUL = 0x00;
    static const char s_EOT = 0x04;
    bool result = true;
    extractor xtract(m_mo_data);                    /* translation data     */
    if (m_swapped_bytes)
        xtract.set_swapped_bytes();

    extractor::offset * orig =                      /* msg table start      */
        xtract.offset_ptr(m_mo_header.offset_original);

    extractor::offset * tran =                      /* translation start    */
        xtract.offset_ptr(m_mo_header.offset_translated);

    int count = int(m_mo_header.string_count);
    for (int index = 0; index < count; ++index, ++orig, ++tran)
    {
        /*
         * The full original length and offset, which can start with a context
         * string terminated by an EOT character. We save the maximum offset
         * in order to avoid bleeding into the next original string.
         */

        word ooffset = swap(orig->o_offset);        /* see extractor.hpp    */
        word olength = swap(orig->o_length);
        std::size_t omax = std::size_t(ooffset + olength);

        word toffset = swap(tran->o_offset);
        word tlength = swap(tran->o_length);
        translation tranquad;

        /*
         * Context comes first, if present. If so, we get the context string
         * and change the offset and length to get the "original" string.
         */

        std::size_t eotpos = xtract.find_character(s_EOT, ooffset, olength);
        if (xtract.checked_offset(eotpos, omax))    /* get context string   */
        {
            std::size_t ctxtlength = eotpos - ooffset;
            tranquad.context = xtract.get(ooffset, ctxtlength);
            ooffset = word(eotpos) + 1;
            olength -= word(ctxtlength);
        }

        /*
         *  Check for the presence of the plural form of the original
         *  string.
         */

        std::size_t nulpos = xtract.find_character(s_NUL, ooffset, olength);
        if (nulpos < std::size_t(ooffset + olength - 1))
        {
            std::size_t len1 = nulpos - ooffset;
            tranquad.original = xtract.get(ooffset, len1);   /* original */
            ooffset = nulpos + 1;
            olength -= len1 + 1;
            tranquad.original_plural = xtract.get(ooffset, olength);
        }
        else
            tranquad.original = xtract.get(ooffset, olength);   /* original */

        /*
         *  Get the original string and the first (singular) translation.
         *  If the length of the translation is less than the specified
         *  length of the translation, then get the plural-forms.
         */

        std::string translated = xtract.get(toffset, tlength);  /* singular */
        int translength = int(translated.length());
        if (translength > 0)
        {
            tranquad.translated = translated;
            if (translength < tlength)
            {
                phraselist & plist{tranquad.plurals};
                toffset += translength + 1;
                tlength -= translength + 1;
                for (;;)
                {
                    std::size_t range_end = std::size_t(toffset + tlength);
                    std::size_t nulpos = xtract.find_character
                    (
                        s_NUL, toffset, tlength
                    );
                    if (nulpos < range_end)
                    {
                        std::size_t len = nulpos - toffset;
                        std::string plural = xtract.get(toffset, len);
                        if (plural.empty())
                            break;
                        else
                            plist.push_back(plural);

                        translength = int(plural.length());
                        toffset += translength + 1;
                        tlength -= translength + 1;
                    }
                    else
                        break;
                }
            }
        }
        m_translations.push_back(tranquad);
    }
    return result;
}

/**
 *  This function assumes that parsing had been done and the value from the
 *  header have been extracted.
 *
 *  First check if the string has already been looked up, in which case it
 *  is in the m_translations vector. If found, return it and exit.
 *
 *  Thus function is not all that useful for potext. See the
 *  load_translations() function instead.
 */

std::string
moparser::translate (const std::string & original)
{
    std::string result;
    if (m_ready && ! original.empty())
    {
        std::string lookup = find(original);        /* look in translations */
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
