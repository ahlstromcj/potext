#if ! defined POTEXT_PO_POMOPARSER_HPP
#define POTEXT_PO_POMOPARSER_HPP

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
 * \file          pomoparserbase.hpp
 *
 *      A refactoring of tinygettext::POParser.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-03-26
 * \updates       2024-03-26
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
 *  Purely for catch errors. Doesn't even inherit from std::exception.
 *
 *  Should upgrade that at some point.
 */

class internal_parser_error
{
    // nothing!
};

/**
 *  A class to work with the GNU "po" translation file.
 */

class pomoparserbase
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

    std::istream & m_in_stream;

    /**
     *  The dictionary to receive the results of the parsing.
     */

    dictionary & m_dict;

    /**
     *  Used in converting a translated message string to a specific character
     *  set.
     */

    iconvert m_converter;

    /**
     *  The .po file has a "fuzzy" directive, but we still need to figure
     *  out exactly what this means.
     */

    bool m_use_fuzzy;

public:

    /*
     *  Default constructor deleted because of reference members.
     */

    pomoparserbase () = delete;
    pomoparserbase
    (
        const std::string & filename,
        std::istream & in,
        dictionary & dict,
        bool usefuzzy       = true
    );
    pomoparserbase (const pomoparserbase &) = delete;
    pomoparserbase (pomoparserbase &&) = delete;
    pomoparserbase & operator = (const pomoparserbase &) = delete;
    ~pomoparserbase () = default;

    virtual bool parse () = 0;

#if defined PLATFORM_WINDOWS_32
    void error (const std::string & msg, std::size_t pos = 0);
#else
    void error
    (
        const std::string & msg, std::size_t pos = 0
    ) __attribute__((__noreturn__));
#endif
    void warning (const std::string & msg, std::size_t pos = 0);

public:

#if 0
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
#endif

    static void loose ()
    {
        sm_pedantic = false;
    }

    static bool pedantic ()
    {
        return sm_pedantic;
    }

    const std::string & filename () const
    {
        return m_filename;
    }

    std::istream & in_stream ()
    {
        return m_in_stream;
    }

    dictionary & dict ()
    {
        return m_dict;
    }

    bool use_fuzzy () const
    {
        return m_use_fuzzy;
    }

    iconvert & converter ()
    {
        return m_converter;
    }

};              // class pomoparserbase

}               // namespace po

#endif          // POTEXT_PO_POMOPARSER_HPP

/*
 * pomoparserbase.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
