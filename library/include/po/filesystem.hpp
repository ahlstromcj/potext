#if ! defined POTEXT_PO_FILESYSTEM_HPP
#define POTEXT_PO_FILESYSTEM_HPP

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
 * \file          filesystem.hpp
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

#include <iosfwd>                       /* class forward for std::istream<> */
#include <memory>                       /* std::unique_ptr<> template       */
#include <string>                       /* std::string                      */

#include "po/po_types.hpp"

namespace po
{

/**
 *  Used often enough to be worth an alias.
 */

using uistream_ptr = std::unique_ptr<std::istream>;

/**
 *  An abstract base class for po-related filesystem management.
 */

class filesystem
{

public:

    filesystem () = default;
    filesystem (const filesystem &) = delete;
    filesystem (filesystem &&) = delete;
    filesystem & operator = (const filesystem &) = delete;
    virtual ~filesystem () = default;

    virtual phraselist open_directory (const std::string & pathname) = 0;
    virtual uistream_ptr open_file
    (
        const std::string & filename
    ) = 0;

};              // class po::filesystem

}               // namespace po

#endif          // POTEXT_PO_FILESYSTEM_HPP

/*
 * filesystem.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

