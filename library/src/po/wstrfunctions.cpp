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
 *  See tinydoc/LICENSE.md for the original tinywstrfunctions licensing statement.
 *  If you do not like the changes or the GPL licensing, use the original
 *  tinywstrfunctions project, available at GitHub:
 *
 *      https://github.com/tinywstrfunctions/tinywstrfunctions
 */

/**
 * \file          wstrfunctions.cpp
 *
 *      Functions for dealing with wide versus narrow strings.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-03-30
 * \updates       2024-03-30
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

namespace po
{

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

}               // namespace po

/*
 * wstrfunctions.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
