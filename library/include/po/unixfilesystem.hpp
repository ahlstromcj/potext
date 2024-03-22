#if !defined POTEXT_PO_UNIXFILESYSTEM_HPP
#define POTEXT_PO_UNIXFILESYSTEM_HPP

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
 * \file          unixfilesystem.hpp
 *
 *  Macros that depend upon the build platform.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-05
 * \license       See above.
 *
 */

#include "filesystem.hpp"               /* po::filesystem v std::filesystem */

namespace po
{

class unixfilesystem : public filesystem
{
public:

    unixfilesystem ();
    unixfilesystem (const unixfilesystem &) = delete;
    unixfilesystem (unixfilesystem &&) = delete;
    unixfilesystem & operator = (const unixfilesystem &) = delete;
    virtual ~unixfilesystem () = default;

    phraselist open_directory (const std::string & pathname) override;
    uistream_ptr open_file
    (
        const std::string & filename
    ) override;

};              // class unixfilesystem

}               // namespace po

#endif          // POTEXT_PO_UNIXFILESYSTEM_HPP

/*
 * unixfilesystem.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

