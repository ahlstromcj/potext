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
 * \updates       2024-03-27
 * \license       See above.
 *
 */

#include <iosfwd>                       /* ostringstream forward reference  */
#include <vector>                       /* std::vector<> template           */

#include "po/extractor.hpp"             /* po::extractor class              */
#include "po/pomoparserbase.hpp"        /* po::pomoparserbase class         */

namespace po
{

/**
 *  A class to work with the GNU binary "mo" translation file.
 */

class moparser final : public pomoparserbase
{

private:

    using word = extractor::word;

    using translation = struct
    {
        std::string original;
        std::string translated;
    };

    using translations = std::vector<translation>;

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

private:

    static const word sm_magic;
    static const word sm_magic_swapped;

private:                // ORIGINAL

    /**
     *  Indicates we had to swap bytes based on the magic number found in the
     *  GNU .mo file.
     */

    bool m_swapped_bytes;           /* FIXME */

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
     *  Holds the character-set name from in the .mo file.
     */

    mutable std::string m_charset;

    /**
     *  Indicates we have already looked up the character-set.
     */

    mutable bool m_charset_parsed;

    /**
     *  Holds the plural-forms string from in the .mo file.
     */

    mutable std::string m_plural_forms;

    /**
     *  Indicates we have already looked up the character-set.
     */

    mutable bool m_plural_forms_parsed;

    /**
     *  Indicates the items are ready.
     */

    bool m_ready;

private:

    moparser () = delete;
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
    ~moparser () = default;

    virtual bool parse () override;

    bool parse_file (const std::string & filename);
    void clear ();
    word swap (word ui) const;
    std::string find (const std::string & target);
    std::string translate (const std::string & original);

    bool load_translations ();
    std::string load_charset ();
    std::string load_plural_forms ();

    const translations & get_translations() const
    {
        return m_translations;
    }

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

};              // class moparser

}               // namespace po

#endif          // POTEXT_PO_MOPARSER_HPP

/*
 * moparser.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
