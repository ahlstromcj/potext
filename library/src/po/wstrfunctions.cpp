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
 * \file          wstrfunctions.cpp
 *
 *      Functions for dealing with wide versus narrow strings.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-03-30
 * \updates       2024-04-02
 * \license       See above.
 *
 *      Functions to work around narrow versus wide strings. Not just for
 *      Windows.
 */

#include <cerrno>                       /* errno                            */
#include <climits>                      /* PATH_MAX                         */
#include <clocale>                      /* std::setlocale()                 */
#include <codecvt>                      /* std::codecvt()                   */
#include <cstdlib>                      /* std::getenv()                    */
#include <cstring>                      /* std::strerror()                  */
#include <locale>                       /* std::wstring_convert<>           */
#include <map>                          /* std::map<>                       */

#include "po/wstrfunctions.hpp"         /* external wide-related functions  */

/**
 *  For the most part, these limits are sanity checks.
 */

#if defined PLATFORM_WINDOWS            /* Microsoft platform               */
#define PATH_SLASH      "\\"
#define PATH_SLASH_CHAR '\\'
#else
#include <unistd.h>
#define PATH_SLASH      "/"
#define PATH_SLASH_CHAR '/'
#endif

namespace po
{

/**
 *  Tests if lhs ends with rhs.
 *
 *  Can we use operator <=> here? :-D
 */

bool
has_suffix (const std::string & lhs, const std::string & rhs)
{
    if (lhs.length() < rhs.length())
    {
        return false;
    }
    else
    {
        size_t diff = lhs.length() - rhs.length();
        return lhs.compare(diff, rhs.length(), rhs) == 0;
    }
}

/**
 *  Tests if the path has a file extension. Useful for .po and .mo files
 *  specified as the only dictionary file to use. Returns true if a dot
 *  appears near the end (between two and three characters).
 */

bool
has_file (const std::string & fullpath)
{
    bool result = false;
    auto dpos = fullpath.find_last_of(".");
    bool ok = dpos != std::string::npos;
    if (ok)
        result = (fullpath.length() - dpos) >= 2;

    return result;
}

/**
 *  Extracts the path from the beginning to the last slash.
 *
 *      -   No slash or only a slash at the beginning.
 *          -   If no '.', return the whole parameter.
 *          -   Otherwise, return an empty string.
 *      -   Otherwise, return everything from the beginning to
 *          the last slash, inclusive.
 *
 *  TODO: a wide-string version returning std::string.
 */

std::string
filename_path (const std::string & fullpath)
{
    std::string result;
    auto spos = fullpath.find_last_of(PATH_SLASH);
    bool ok = spos != std::string::npos;
    if (ok)
    {
        if (spos == 0)                                  /* starting slash   */
        {
            auto dpos = fullpath.find_last_of(".");
            bool nodot = dpos == std::string::npos;
            if (nodot)
                result = fullpath;
        }
        else
        {
            auto len = spos + 1;
            result = fullpath.substr(0, len);           /* include slash    */
        }
    }
    else                                                /* no slash         */
    {
        auto dpos = fullpath.find_last_of(".");
        bool nodot = dpos == std::string::npos;
        if (nodot)
            result = fullpath;
    }
    return result;
}

/**
 *  Tests for "LC_", "locale", or ".mo" in a path/file specification.
 */

bool
is_mo_path (const std::string & fullpath)
{
    return
    (
        (fullpath.find("LC_") != std::string::npos) ||
        (fullpath.find("locale") != std::string::npos) ||
        (fullpath.find(".mo") != std::string::npos)
    );
}

/**
 *  A simpler check.
 */

bool
is_pm_file (const std::string & fullpath)
{
    return fullpath.find(".mo") == (fullpath.length() - 3);
}

/**
 *  Looks for "LC_" in a path like "/usr/share/locale/es/LC_MESSAGES..." and
 *  extracts the "es" portion, i.e. the domain name. This function is meant
 *  for use with .mo files, not .po files.
 *
 * \param fullpath
 *      Provides the path to the .mo file. For consistency, this path should
 *      also include the name of the .mo file.
 *
 * \return
 *      Returns the domain name. If empty, the full path is ill-formed.
 */

std::string
extract_mo_domain (const std::string & fullpath)
{
    std::string result;
    auto lcpos = fullpath.find("LC_");
    bool ok = lcpos != std::string::npos;
    if (ok)
    {
        auto spos1 = fullpath.rfind(PATH_SLASH_CHAR, lcpos);
        ok = spos1 != std::string::npos;
        if (ok)
        {
            auto spos0 = fullpath.rfind(PATH_SLASH_CHAR, spos1 - 1);
            ok = spos0 != std::string::npos;
            if (ok)
                result = fullpath.substr(spos0 + 1, spos1 - spos0);
        }
    }
    return result;
}

/**
 *  Tests for "/po" or ".po" in a path/file specification.
 */

bool
is_po_path (const std::string & fullpath)
{
    return
    (
        (fullpath.find("/po/") != std::string::npos) ||
        (fullpath.find("\\po\\") != std::string::npos) ||
        (fullpath.find(".po") != std::string::npos)
    );
}

/**
 *  A simpler check.
 */

bool
is_po_file (const std::string & fullpath)
{
    return fullpath.find(".po") == (fullpath.length() - 3);
}

/**
 *  Looks for ".po" in a path like ".../anything/es.po" and extracts the "es"
 *  portion, i.e. the domain name. This function is meant for use with .po
 *  files, not .mo files.
 *
 * \param fullpath
 *      Provides the path to the .po file. This path must include the name
 *      of the .po file. It can also be simply the base name of the .po file,
 *      i.e. no path.
 *
 * \return
 *      Returns the domain name. If empty, the full path is ill-formed.
 */

std::string
extract_po_domain (const std::string & fullpath)
{
    std::string result;
    auto popos = fullpath.find(".po");
    bool ok = popos != std::string::npos;
    if (ok)
    {
        auto spos0 = fullpath.rfind(PATH_SLASH_CHAR, popos - 1);
        bool haveslash = spos0 != std::string::npos;
        if (haveslash)
            result = fullpath.substr(spos0 + 1, popos - spos0);
        else
            result = fullpath.substr(0, popos);
    }
    return result;
}

#if defined POTEXT_WIDE_STRING_SUPPORT

/**
 *  Converts a string, which must be ASCII, to a wide string.  Useful for basic
 *  translation of system path names in Linux, Windows, and other operating
 *  systems.
 */

std::wstring
widen_ascii_string (const std::string & source)
{
    std::wstring result;
    result.assign(source.begin(), source.end());
    return result;
}

/**
 *  Converts a wide string consisting of values less than or equal to 255
 *  (extended ASCII) to a normal string.
 */

std::string
narrow_ascii_string (const std::wstring & wsource)
{
    std::string result;
    for (auto wch : wsource)
    {
        int ch = int(wch);
        result += char(ch);
    }
    return result;
}

/**
 *  Stores the bytes of a wide string into a normal string; no character
 *  conversion involved.  Useful only for shipping wide strings around
 *  on the same computer.
 */

std::string
pack_wide_string (const std::wstring & wsource)
{
    const char * rawdata = reinterpret_cast<const char *>(wsource.data());
    std::size_t sz = wsource.size() * sizeof(wsource[0]);
    std::string result;
    for (std::size_t i = 0; i < sz; ++i)
        result.push_back(*rawdata++);

    return result;
}

std::wstring
unpack_wide_string (const std::string & source)
{
    const wchar_t * rawdata = reinterpret_cast<const wchar_t *>(source.data());
    std::size_t sz = source.size() / sizeof(wchar_t);
    std::wstring result;
    for (std::size_t i = 0; i < sz; ++i)
        result.push_back(*rawdata++);

    return result;
}

/*
 *  Conversions between wide-strings and UTF-8.
 *
 *      https://stackoverflow.com/questions/4358870/
 *          convert-wstring-to-string-encoded-in-utf-8
 *
 *  Converts between UTF-16 and UCS-2/UCS-4:
 *
 *      codecvt_utf16: C++11; deprecated in C++17; removed in C++26
 *
 *  Converts between UTF-8 and UTF-16:
 *
 *      codecvt_utf8_utf16: C++11; deprecated in C++17; removed in C++26
 *
 *  Note that this code, as stated above, is living on borrowed time.
 */

#if defined PLATFORM_MSVC
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

std::string
wstring_to_utf8 (std::wstring const & wstr)
{
    std::wstring_convert<std::conditional_t<
        sizeof(wchar_t) == 4,
        std::codecvt_utf8<wchar_t>,
        std::codecvt_utf8_utf16<wchar_t>>> converter;

    return converter.to_bytes(wstr);
}

std::wstring
utf8_to_wstring (std::string const & str)
{
    std::wstring_convert<std::conditional_t<
        sizeof(wchar_t) == 4,
        std::codecvt_utf8<wchar_t>,
        std::codecvt_utf8_utf16<wchar_t>>> converter;

    return converter.from_bytes(str);
}

#if defined PLATFORM_MSVC
#pragma warning(pop)
#endif

#endif          // defined POTEXT_WIDE_STRING_SUPPORT

}               // namespace po

/*
 * wstrfunctions.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
