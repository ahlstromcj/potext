#if ! defined POTEXT_PO_GETTEXTEX_HPP
#define POTEXT_PO_GETTEXTEX_HPP

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
 * \file          gettext.hpp
 *
 *  Universal types for the potext library
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-02-16
 * \updates       2024-02-29
 * \license       See above.
 *
 *  This module adds support functions for additional gettext functions and
 *  macros.
 */

#include <string>                       /* std::string class                */

#include "po_build_macros.h"            /* build (and platform) macros      */

namespace po
{

/*
 * These are additional functions that might not be necessary unless
 * we cannot implement them in the dictionary manager.
 *
 * See the GNU Gettext manual, section 11.2.5.
 */

std::string pgettext_aux
(
    const std::string & domain,
    const std::string & msgctxt,
    const std::string & msgid,
    int category
);
std::string npgettext_aux
(
    const std::string & domain,
    const std::string & msg_ctxt_id,
    const std::string & msgid,
    const std::string & msgid_plural,
    unsigned long int n,
    int category
);
std::string dcpgettext_expr
(
    const std::string & domain,
    const std::string & msgctxt,
    const std::string & msgid,
    int category
);
std::string dcnpgettext_expr
(
    const std::string & domain,
    const std::string & msgctxt,
    const std::string & msgid,
    const std::string & msgid_plural,
    unsigned long int n,
    int category
);

}               // namespace po

#endif          // POTEXT_PO_GETTEXTEX_HPP

/*
 * gettextex.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

