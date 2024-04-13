#if ! defined POTEXT_PO_POMOPARSERBASE_HPP
#define POTEXT_PO_POMOPARSERBASE_HPP

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
 */

/**
 * \file          pomoparserbase.hpp
 *
 *      A base class for po::poparser and po::moparser.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-03-26
 * \updates       2024-04-12
 * \license       See above.
 *
 */

#include <exception>                    /* std::exception base class        */
#include <iosfwd>                       /* ostringstream forward reference  */

#include "platform_macros.h"            /* PLATFORM_WINDOWS macro           */
#include "po/iconvert.hpp"              /* po::iconvert (IConv) class       */
#include "po/po_types.hpp"              /* po:phraselist vector             */

namespace po
{

class dictionary;

/**
 *  Purely for catch errors. Doesn't even inherit from std::exception.
 *
 *  Should upgrade that at some point.
 */

class parser_error : public std::exception
{

private:

    std::string m_message;

public:

    parser_error () : m_message{"No message supplied!"}
    {
        // no code
    }

    parser_error (const std::string & message) : m_message{message}
    {
        if (m_message.empty())
            m_message = "No message supplied!";
    }

    virtual ~parser_error () = default;

    /**
     *  Returns the thrown error message string.
     */

    virtual const std::string & message () const
    {
        return m_message;
    }

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
        bool usefuzzy = true
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

protected:

    iconvert & converter ()
    {
        return m_converter;
    }

    phraselist convert_list (const phraselist & source);
    std::string fix_message (const std::string & msg);

};              // class pomoparserbase

}               // namespace po

#endif          // POTEXT_PO_POMOPARSERBASE_HPP

/*
 * pomoparserbase.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
