#if !defined POTEXT_PO_LANGUAGE_HPP
#define POTEXT_PO_LANGUAGE_HPP

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
 * \file          language.hpp
 *
 *      Provides a representation for languages used in translation.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-11
 * \license       See above.
 *
 */

#include <string>

#include "platform_macros.h"            /* OS & debug macroes etc.          */

namespace po
{

/**
 *  Defined in po/languagespecs.hpp now.
 */

struct languagespec;
class language;

/**
 *  Light-weight wrapper around the languagespec structure.
 */

class language
{
    friend bool operator < (const language & lhs, const language & rhs);

    friend struct langhash;             /* from the dictionarymgr module    */

private:

    const languagespec * m_language_spec;

private:

    language (const languagespec * lang_spec);

public:

    /**
     *  Create an undefined language object.
     */

    language ();
    language (const language &) = default;
    language (language &&) = default;
    language & operator = (const language &) = default;
    ~language () = default;

    bool operator == (const language & rhs) const;
    bool operator != (const language & rhs) const;

    const languagespec & spec () const;

public:

    static language from_spec
    (
        const std::string & lang,
        const std::string & country = "",
        const std::string & modifier = ""
    );
    static language from_name (const std::string & str);
    static language from_env (const std::string & env);
    static int match (const language & lhs, const language & rhs);

    explicit operator bool () const
    {
        return m_language_spec != nullptr;
    }

    std::string get_language () const;
    std::string get_country () const;
    std::string get_modifier () const;
    std::string get_name () const;
    std::string get_localized_name () const;
    std::string to_string () const;

};

/**
 *  Free functions.
 */

inline bool
operator < (const language & lhs, const language & rhs)
{
    return lhs.m_language_spec < rhs.m_language_spec;
}

}               // namespace po

#endif          // POTEXT_PO_LANGUAGE_HPP

/*
 * language.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
