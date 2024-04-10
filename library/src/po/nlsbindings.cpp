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
 * \file          nlsbindings.cpp
 *
 *      A reimplementation of gettext domain/directory binding functions.
 *
 * \library       potext
 * \author        gettext; refactoring by Chris Ahlstrom
 * \date          2024-02-20
 * \updates       2024-04-01
 * \license       See above.
 *
 *  Defines setbinding(), a replacement for set_binding_values in the
 *  gettext module bindtextdom.c; tentative
 *
 *  CURRENTLY for REASEARCH ONLY.
 *
 *  set_binding_values():
 *
 *      -   Provides string and wide-string parameters to be used, or to
 *          signal that values are to be returned.
 *      -   Sanity checks on the parameters were made, but aren't needed
 *          with reference parameters.
 *      -   Iterates through the set of domain-bindings, exiting if the
 *          desired domain is found or not (a null binding flags the latter
 *          case.
 *      -   If not found, then if the parameters are null then the default
 *          dirname is returned in that parameter.
 *      -   If they're not null, then a new binding has to be created.
 *      -   Otherwise, the found binding, dirname or wdirname (whichever is
 *          not null) is checked against the binding to see if it needs to be
 *          updated.
 *
 *  Bindings and operations:
 *
 *      Each binding contains a pointer to the "next" binding. We currently
 *      believe we can use std::forward_list<>.
 *
 *      -   Find the provided domainname in the linked-list.
 *          -   Iterate through the list.
 *          -   Match the name with a binding in the list.
 *          -   Use the iterator to access the binding, or list.end().
 *      -   Creating a new binding.
 */

#include <clocale>                      /* std::setlocale()                 */
#include <cstdlib>                      /* realpath() and _fullpath()       */
#include <iostream>                     /* std::cout, etc.                  */

/**
 *  Since we're using C++17, we could use std::filesystem::path and it's
 *  current_path() function, but we have to learn a bit about that first.
 */

#if defined PLATFORM_WINDOWS            /* Microsoft platform               */
#include <dir.h>                        /* file-name info and getcwd()      */
#define p_getcwd    _getcwd             /* Microsoft/Mingw's getcwd()       */
#else
#include <unistd.h>
#define p_getcwd    getcwd              /* ISO/POSIX/BSD getcwd()           */
#endif

#include "c_macros.h"                   /* not_nullptr(), etc.              */
#include "po/nlsbindings.hpp"           /* po::nlsbindings class            */
#include "po/wstrfunctions.hpp"         /* po::wide-to-narrow functions     */

namespace po
{

/**
 *  Provides storage for the default locale directory, usually
 *  /usr/share/locale on UNIXen.
 *
 *  Also see init_lib_locale() and init_app_locale().
 *
 * TODO: Set the following for the private nlsbindings object.
 *
 *      nlsbindings::sm_default_dirname
 *      nlsbindings::sm_default_wdirname
 */

std::string nlsbindings::sm_default_dirname = "/usr/share/locale";

/**
 *  Provides storage for the default locale directory on strict Windows
 *  32 (without Cygwin).
 *
 *  Probably need to get the application's top path and append "/locale" to
 *  it.
 *
 *  Another option is "C:/Users/username/AppData/Local/PACKAGE/locale".
 *
 *  Also see init_lib_locale() and init_app_locale().
 */

std::wstring nlsbindings::sm_default_wdirname = L"C:/";  // TODO TODO

/**
 *  Constructor
 */

nlsbindings::nlsbindings () :
    m_container     (),
    m_last_binding  (m_container.begin()),
    m_count         (0)
{
    // no code yet
}

/**
 *  Search for a binding by domain name.
 */

nlsbindings::container::iterator
nlsbindings::find (const std::string & domainname)
{
    auto result = m_container.end();
    if (! domainname.empty())
    {
        for (auto bit = m_container.begin(); bit != m_container.end(); ++bit)
        {
            if (domainname == bit->b_domainname)
            {
                result = bit;
                break;
            }
        }
    }
    return result;
}

nlsbindings::binding *
nlsbindings::create_binding
(
    const std::string & domainname,
    const std::string & dirname
)
{
    binding * result = new (std::nothrow) binding();
    bool ok = not_nullptr(result);
    if (ok)
    {
        std::string tmpname = dirname.empty() ? sm_default_dirname : dirname ;
        result->b_dirname = tmpname;
        result->b_wdirname.clear();
        result->b_codeset.clear();
        result->b_domainname = domainname;
    }
    return result;
}

#if defined POTEXT_WIDE_STRING_SUPPORT

nlsbindings::binding *
nlsbindings::create_binding_wide
(
    const std::string & domainname,
    const std::wstring & wdirname
)
{
    binding * result = new (std::nothrow) binding();
    bool ok = not_nullptr(result);
    if (ok)
    {
        std::wstring tmpname = wdirname.empty() ?
            sm_default_wdirname : wdirname ;

        result->b_dirname.clear();
        result->b_wdirname = tmpname;
        result->b_codeset.clear();
        result->b_domainname = domainname;
    }
    return result;
}

#endif          // defined POTEXT_WIDE_STRING_SUPPORT

/**
 *  Specifies the directory name *dirname, the directory name *wdirnamep
 *  (only on native Windows), and the output codeset *codesetp to be used
 *  for the domainname message catalog.
 *
 *  If *dirname or *wdirnamep or *codesetp is NULL, the corresponding attribute
 *  is not modified, only the current value is returned.
 *
 *      For C++, the referenced string has to be empty.
 *
 * Old stuff:
 *
 *  If dirname or wdirnamep or codesetp is NULL, the corresponding attribute is
 *  neither modified nor returned, except that setting wdirname erases dirname
 *  and vice versa.
 *
 *      For C++, the referenced string has to be set to "?".
 *
 * Key differences:
 *
 *  -   No locking is done.
 *  -   We don't bother with memory allocation failures except to return false;
 *      the caller must abort.
 *  -   Sanity checks are not needed with the usage of reference parameters.
 *  -   GNU has to support older C compilers; we do not.
 *  -   We can use standard C++ containers to support the linked-list.
 *  -   This version is strictly for the use of bindtextdomain().
 *  -   Caches not flushed. We assume that the work is done once, in main().
 *  -   This function ignores codeset and wide-strings.
 *
 *  bindtextdomain():
 *
 *      set_binding_values(domainname, &dirname, NULL, NULL);
 */

bool
nlsbindings::set_binding_values
(
    const std::string & domainname,
    std::string & dirname
)
{
    bool result = false;
    auto bit = find(domainname);
    if (bit != m_container.end())
    {
        if (dirname.empty())                        /* get current binding  */
        {
            dirname = bit->b_dirname;               /* set this parameter   */
            result = true;
        }
        else
        {
            /*
             * The specified domain is already bound. If the new value and
             * the old one are equal we simply do nothing. Otherwise replace
             * the old binding.
             */

            std::string current = bit->b_dirname;
            if (dirname != current)
            {
                bit->b_dirname = current;
                result = true;
            }
        }
    }
    else if (dirname.empty())                       /* just return default  */
    {
        dirname = sm_default_dirname;
    }
    else                                            /* create a new binding */
    {
        binding * new_binding = create_binding(domainname, dirname);
        if (not_nullptr(new_binding))
        {
            auto start = m_last_binding;
            if (start == m_container.end())
                m_container.push_front(*new_binding);
            else
                m_last_binding = m_container.insert_after(start, *new_binding);

            ++m_count;
        }
        result = true;
    }
    return result;
}

/**
 *  bind_textdomain_codeset():
 *
 *      set_binding_values (domainname, NULL, NULL, &codeset);
 */

bool
nlsbindings::set_binding_codeset
(
    const std::string & domainname,
    std::string & codeset
)
{
    bool result = false;
    auto bit = find(domainname);
    if (bit != m_container.end())
    {
        if (codeset.empty())
        {
            codeset = bit->b_codeset;
            result = true;
        }
        else
        {
	        /*
             * The domain is already bound. If the new value and the old one
             * are equal we simply do nothing. Otherwise replace the old
             * binding.
             */

            std::string current = bit->b_codeset;
            if (codeset != current)
            {
                bit->b_codeset = current;
                result = true;
            }
        }
    }
    return result;
}

#if defined POTEXT_WIDE_STRING_SUPPORT

/**
 * libintl_wbindtextdomain():
 *      set_binding_values(domainname, NULL, &wdirname, NULL);
 */

bool
nlsbindings::set_binding_wide
(
    const std::string & domainname,
    std::wstring & wdirname
)
{
    bool result = false;
    auto bit = find(domainname);
    if (bit != m_container.end())
    {
        if (wdirname.empty())                        /* get current binding  */
        {
            wdirname = bit->b_wdirname;               /* set this parameter   */
            result = true;
        }
        else
        {
            /*
             * The specified domain is already bound. If the new value and
             * the old one are equal we simply do nothing. Otherwise replace
             * the old binding.
             */

            std::wstring wcurrent = bit->b_wdirname;
            if (wdirname != wcurrent)
            {
                bit->b_wdirname = wcurrent;
                result = true;
            }
        }
    }
    else if (wdirname.empty())                      /* just return default  */
    {
        wdirname = sm_default_wdirname;
    }
    else                                            /* create a new binding */
    {
        binding * new_binding = create_binding_wide(domainname, wdirname);
        if (not_nullptr(new_binding))
        {
            new_binding->b_wdirname = wdirname;

            auto start = m_last_binding;
            m_last_binding = m_container.insert_after(start, *new_binding);
        }
        result = true;
    }
    return result;
}

#endif          // defined POTEXT_WIDE_STRING_SUPPORT

static bool
name_has_root_path (const std::string & filename)
{
#if defined PLATFORM_WINDOWS
    static std::string s_rootchars = "~/\\:";
#else
    static std::string s_rootchars = "~/";      /* "~" == "/home/usr"       */
#endif
    auto pos = filename.find_first_of(s_rootchars);
    bool result = pos != std::string::npos;
    if (result)
    {
#if defined PLATFORM_WINDOWS
        bool colon = pos == 1 && filename[pos] == ':';
        result = false;
        if (colon && std::isalpha(filename[0])) /* e.g. "C:"                */
            result = true;
        else
            result = pos == 0;                  /* starts with ~, /, or \   */
#else
        result = pos == 0;                      /* path starts with ~ or /  */
#endif
    }
    return result;
}

static std::string
get_full_path (const std::string & path)
{
    std::string result;                         /* default empty result     */
    if (! path.empty())
    {
#if defined PLATFORM_WINDOWS
        char * resolved_path = NULL;            /* what a relic!            */
        char temp[256];
        resolved_path = _fullpath(temp, path.c_str(), 256);
        if (not_NULL(resolved_path))
            result = resolved_path;
#else
        char * resolved_path = NULL;            /* what a relic!            */
        resolved_path = realpath(path.c_str(), NULL);
        if (not_NULL(resolved_path))
        {
            result = resolved_path;
            free(resolved_path);
        }
#endif
    }
    return result;
}

/**
 *  Looks up the locale directory associated with a domain name.
 *
 *  TODO: FIXME!!!!!!!!
 */

std::string
nlsbindings::get_binding
(
    const std::string & domainname,
    const std::string & dirname
)
{
    std::string result = sm_default_dirname;
    auto bit = find(domainname);
    if (bit != m_container.end())
    {
        if (! name_has_root_path(dirname))      /* relative path, root it   */
        {
            char buffer[1024];                  /* we'll fix it later       */
            const char * b = p_getcwd(buffer, 1024);
            if (not_nullptr(b))
                result = get_full_path(std::string(b));
        }
    }
    return result;
}

#if defined POTEXT_WIDE_STRING_SUPPORT

/**
 *  This needs some work, unfortunately.
 */

std::wstring
nlsbindings::get_binding_wide
(
    const std::string & domainname,
    const std::wstring & wdirname
)
{
    std::wstring result = sm_default_wdirname;
    auto bit = find(domainname);
    if (bit != m_container.end())
    {
        std::string dirname = wstring_to_utf8(wdirname);
        if (! name_has_root_path(dirname))       /* relative path, root it   */
        {
            char buffer[1024];                   /* we'll fix it later       */
            const char * b = p_getcwd(buffer, 1024);
            if (not_nullptr(b))
            {
                dirname = get_full_path(std::string(b));
                result = utf8_to_wstring(dirname);
            }
        }
    }
    return result;
}

#endif          // defined POTEXT_WIDE_STRING_SUPPORT

}               // namespace po

/*
 * nlsbindings.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
