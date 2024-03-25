#if ! defined POTEXT_PO_POPARSER_HPP
#define POTEXT_PO_POPARSER_HPP

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
 * \file          poparser.hpp
 *
 *      A refactoring of tinygettext::POParser.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-11
 * \license       See above.
 *
 */

#include <iosfwd>                       /* ostringstream forward reference  */

#include "platform_macros.h"            /* PLATFORM_WINDOWS macro           */
#include "po/iconvert.hpp"              /* po::iconvert (IConv) class       */

namespace po
{

class dictionary;

/**
 *  A class to work with the GNU "po" translation file.
 */

class poparser
{

private:

    /**
     *  If true (the default), a warning is emitted when the gettext keyword
     *  and message string are not separated by a single space. Useful?
     */

    static bool sm_pedantic;

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

    /**
     *  Indicates that the "from" character set is "BIG5".
     *  Again, we need to figure out exactly what this means.
     */

    bool m_big5;

    /**
     *  Provides the current line number being processed. Incremented
     *  by the next_line() function.
     */

    int m_line_number;

    /**
     *  The text of the current line being processed.
     */

    std::string m_current_line;

    /**
     *  Used in converting a message string to a specific character set.
     */

    iconvert m_conv;

private:

    /*
     *  Default constructor deleted because of reference members.
     */

    poparser () = delete;
    poparser
    (
        const std::string & filename,
        std::istream & in,
        dictionary & dict,
        bool usefuzzy       = true
    );
    poparser (const poparser &) = delete;
    poparser (poparser &&) = delete;
    poparser & operator = (const poparser &) = delete;
    ~poparser();

    bool parse_header (const std::string & header);
    bool parse ();
    void next_line ();
    std::string get_string (std::size_t skip);
    void get_string_line (std::ostringstream & str, std::size_t skip);
    bool is_empty_line ();
    bool prefix_match (const char *);

#if defined PLATFORM_WINDOWS_32
    void error (const std::string & msg);
#else
    void error (const std::string & msg) __attribute__((__noreturn__));
#endif
    void warning (const std::string & msg);

    bool get_msgid_plural
    (
        bool fuzzy,
        const std::string & msgctxt,
        const std::string & msgid
    );
    bool get_msgstr
    (
        bool fuzzy,
        const std::string & msgctxt,
        const std::string & msgid
    );
    const std::string & line () const
    {
        return m_current_line;
    }
    char line (std::size_t i)
    {
        return m_current_line[i];
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

    static bool parse_po_file
    (
        const std::string & filename,
        std::istream & in,
        dictionary & dict
    );

    static void loose ()
    {
        sm_pedantic = false;
    }

};              // class poparser

}               // namespace po

#endif          // POTEXT_PO_POPARSER_HPP

/*
 * poparser.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
