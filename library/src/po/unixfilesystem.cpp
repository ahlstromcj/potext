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
 * \file          unixfilesystem.cpp
 *
 *      Provides access to std::filesystem.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2025-05-22
 * \license       See above.
 *
 *  This class useds the static function std::filesystem::directory_iterator().
 *
 *  Note: std::filesystem requires C++17, which needs gcc 9, but our
 *  current Mingw framework on Windows 11 has only gcc 8.
 */

#include <fstream>

#include "platform_macros.h"            /* PLATFORM_WINDOWS (WIN32) macro   */

#if defined PLATFORM_WINDOWS

#if defined PLATFORM_WINDOWS_32
#include "po/dirent.h"
#endif

#else

#include <filesystem>                   /* std::filesystem v po::filesystem */
#include "po/unixfilesystem.hpp"
#endif

namespace po
{

unixfilesystem::unixfilesystem () : filesystem()
{
    // no code
}

phraselist
unixfilesystem::open_directory (const std::string & pathname)
{
    phraselist files;
    for (const auto & p : std::filesystem::directory_iterator(pathname))
    {
        files.push_back(p.path().filename().string());
    }
    return files;
}

uistream_ptr
unixfilesystem::open_file (const std::string & filename)
{
    return uistream_ptr(new std::ifstream(filename));
}

}               // namespace po

/*
 * unixfilesystem.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
