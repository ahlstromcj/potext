#if ! defined POTEXT_PO_MOPARSER_HPP
#define POTEXT_PO_MOPARSER_HPP

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
 *  See tinydoc/LICENSE.md for the original tinygettext licensing statement.
 *  If you do not like the changes or the GPL licensing, use the original
 *  tinygettext project, available at GitHub:
 *
 *      https://github.com/tinygettext/tinygettext
 */

/**
 * \file          moparser.hpp
 *
 *      A refactoring of tinygettext::POParser.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-03-24
 * \updates       2024-03-24
 * \license       See above.
 *
 */

#include <iosfwd>                       /* ostringstream forward reference  */
#include <vector>                       /* std::vector<> template           */

#include "platform_macros.h"            /* PLATFORM_WINDOWS macro           */
#include "po/extractor.hp"              /* po::extractor class              */

namespace po
{

class dictionary;

/*
struct GettextMessage
{
    char* string;
    int length;
};
 */

class dictionary;

/**
 *  A class to work with the GNU binary "mo" translation file.
 */

class moparser
{

private:

    using word = extractor::word;

    using translation = struct
    {
        std::string original;
        std::string translated;
    };

    using translations = std::vector<translation>

    using header = struct
    {
        word magic;                 // +00:  magic id
        word revision;              // +04:  revision
        word string_count;          // +08:  number of strings in the file
        word offset_original;       // +0C:  start of original string table
        word offset_translated;     // +10:  start of translated string table
        word size_hash_table;       // +14:  hash table size
        word offset_hash_table;     // +18:  offset of hash table start
    };

#if 0
    using offset = struct
    {
        word length;    // length of the string
        word offset;    // pointer to the string
    };
#endif

private:

    static const word sm_magic;
    static const word sm_magic_swapped;

private:                // ORIGINAL

    /**
     *  Indicates we had to swap bytes based on the magic number found in the
     *  GNU .mo file.
     */

    bool m_swapped_bytes;

    /**
     *  Holds the data found in the header of the .mo file.
     */

    header m_mo_header;

    /**
     *  The binary data and text read from the .mo file.
     */

    std::string m_mo_data;

    /**
     *  Holds the transation pairs found in the .mo file.
     */

    translations m_translations;

    /**
     *  Hold the character-set name from in the .mo file.
     */

    mutable std::string m_charset;

    /**
     *  Indicates we have already looked up the character-set.
     */

    mutable bool m_charset_parsed;

    /**
     *  Indicates the items are ready.
     */

    bool m_ready;

private:                // NEW

    /**
     *  Provides the name of the .po file to be parsed to create a dictionary
     *  object. Used only for warning and error messages.
     */

    std::string m_filename;

    /**
     *  The input file stream from which the .po data is to be read.
     */

    std::istream & m_in;

    /**
     *  The dictionary to receive the results of the parsing.
     */

    dictionary & m_dict;

    /**
     *  The .po file has a "fuzzy" directive, but we still need to figure
     *  out exactly what this means.
     */

    bool m_use_fuzzy;

    /**
     *  Indicates that no more lines can be read from the .po file.
     */

    bool m_eof;

    // NOTE: additional poparser members not yet included

public:

    moparser () = default;
    moparser
    (
        const std::string & filename,
        std::istream & in,
        dictionary & dict,
        bool usefuzzy       = true
    );
    moparser (const moparser &) = delete;
    moparser (moparser &&) = delete;
    moparser & operator = (const moparser &) = delete;

    ~moparser ()
    {
        clear();
    }

    bool parse_file (const std::string & filename);
    bool parse (char * moData);
    void clear ();
    std::string translate (const std::string & original);
    std::string charset () const;

    bool ready () const
    {
        return m_ready;
    }

public:

    /**
     * \param filename
     *      Name of the istream, only used in error messages.
     *
     * \param in
     *      Stream from which the PO file is read.
     *
     * \param dict
     *      Dictionary to which the strings are written.
     */

    static bool parse_mo_file
    (
        const std::string & filename,
        std::istream & in,
        dictionary & dict
    );

private:

    word swap (word ui) const;
    std::string find (const std::string & target);

};              // class moparser

}               // namespace po

#endif          // POTEXT_PO_MOPARSER_HPP

/*
 * moparser.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
