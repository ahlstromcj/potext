#if ! defined POTEXT_PO_NLSBINDINGS_HPP
#define POTEXT_PO_NLSBINDINGS_HPP

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
 * \file          nlsbindings.hpp
 *
 *      An attempt to implement bindtextdomain().
 *
 * \library       potext
 * \author        gettext; refactoring by Chris Ahlstrom
 * \date          2024-02-20
 * \updates       2024-03-04
 * \license       See above.
 *
 *  This module defines set_binding_values() and an nlsbindings class.
 *
 *  NLS means "native language support".
 */

#include <forward_list>                 /* std::forward_list<> template     */

#include "platform_macros.h"            /* PLATFORM_WINDOWS macro           */

namespace po
{

/**
 *  A wrapper class to make gettext-like code easier to write.
 */

class nlsbindings
{

public:

    /**
     *  A set of settings bound to a message domain.  Used to store settings
     *  from bindtextdomain() and bind_textdomain_codeset().
     *
     *  We changed the name from "binding" to "nlbinding" for a little more
     *  clarity.
     */

    using binding = struct
    {
        std::string b_dirname;
#if defined PLATFORM_WIN32_STRICT                   /* WIN32 and not Cygwin */
        std::wstring b_wdirname;
#endif
        std::string b_codeset;
        std::string b_domainname;
    };

private:

    using container = std::forward_list<binding>;

private:

    /**
     *  Provides storage for the default locale directory.
     */

    static std::string sm_default_dirname;

#if defined PLATFORM_WIN32_STRICT

    /**
     *  Similar to the above, for Windows without Cygwin.
     */

    static std::wstring sm_default_wdirname;

#endif

    /**
     *  Holds a list of all of the NLS bindings.
     */

    container m_container;                          /* _nl_domain_bindings  */

    /**
     *  Holds an iterator to the last-inserted binding for use in
     *  std::forward_list<>::insert_after().
     */

    container::iterator m_last_binding;             /* last binding added   */

    /**
     *  For efficiency that matches C-coded linked lists, std::forward_list<>
     *  has no size function. So we provided a similar member and function,
     *  purely so we can report on the size of the bindings list.
     */

    int m_count;

public:

    nlsbindings ();
    nlsbindings (const nlsbindings &) = delete;
    nlsbindings (nlsbindings &&) = delete;
    nlsbindings & operator = (const nlsbindings &) = delete;
    ~nlsbindings() = default;

    int count () const
    {
        return m_count;                 /* int(m_container.size()) :-(      */
    }

    bool set_binding_values             /* bindtextdomain() version         */
    (
        const std::string & domainname,
        std::string & dirname
    );
    std::string get_binding
    (
        const std::string & domainname,
        const std::string & dirname
    );

    bool set_binding_codeset
    (
        const std::string & domainname,
        std::string & codeset
    );

#if defined PLATFORM_WIN32_STRICT
    bool set_binding_wide
    (
        const std::string & domainname,
        std::wstring & wdirname
    );
    std::string get_binding
    (
        const std::string & domainname,
        const std::wstring & wdirname
    );
#endif

private:

    container::iterator find (const std::string & domainname);

    binding * create_binding
    (
        const std::string & domainname,
        const std::string & dirname
    );

#if defined PLATFORM_WIN32_STRICT
    binding * create_binding_wide
    (
        const std::string & domainname,
        const std::wstring & dirname
    );
#endif

};              // class nlsbindings

}               // namespace po

#endif          // POTEXT_PO_NLSBINDINGS_HPP


/*
 * nlsbindings.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
