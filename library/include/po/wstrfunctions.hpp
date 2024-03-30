#if ! defined POTEXT_PO_WSTRFUNCTIONS_HPP
#define POTEXT_PO_WSTRFUNCTIONS_HPP

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
 * \file          wstrfunctions.hpp
 *
 *      Some functions for dealing with wide strings.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-03-30
 * \updates       2024-03-30
 * \license       See above.
 *
 */

#include <string>                       /* std::string & std::wstring       */

namespace po
{

/*
 *  Free functions in the po namespace to work around narrow versus wide
 *  strings. Not just for Windows.
 */

extern std::wstring widen_ascii_string (const std::string & source);
extern std::string narrow_ascii_string (const std::wstring & wsource);
extern std::string pack_wide_string (const std::wstring & wsource);
extern std::wstring unpack_wide_string (const std::string & source);
extern std::string wstring_to_utf8 (std::wstring const & str);
extern std::wstring utf8_to_wstring (std::string const & str);

}               // namespace po

#endif          // POTEXT_PO_WSTRFUNCTIONS_HPP

/*
 * wstrfunctions.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

