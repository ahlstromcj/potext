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
 * \file          gettextex.cpp
 *
 *      Extra gettext functions; unused.
 *
 * \library       potext
 * \author        GNU gettext.h; refactoring by Chris Ahlstrom
 * \date          2024-02-29
 * \updates       2024-03-01
 * \license       See above.
 *
 * Note: Currently, these functions are not used.
 */

#include "po/gettext.hpp"               /* basic gettext functions          */

namespace po
{

std::string
pgettext_aux
(
    const std::string & domain,
    const std::string & msgctxt,
    const std::string & msgid,
    int category
)
{
    std::string translation = dcgettext(domain, msgctxt, category);
    return translation == msgctxt ? msgid : translation ;
}

std::string
npgettext_aux
(
    const std::string & domain,
    const std::string & msg_ctxt_id,
    const std::string & msgid,
    const std::string & msgid_plural,
    unsigned long int n,
    int category
)
{
    std::string translation = dcngettext
    (
        domain, msg_ctxt_id, msgid_plural, n, category
    );
    if (translation == msg_ctxt_id || translation == msgid_plural)
        return n == 1 ? msgid : msgid_plural;
    else
        return translation;
}

/**
 *  The original implementaton would allocate a msg_ctxt_id array of
 *  size of a temporary buffer or a larger bit of memory. We don't
 *  have to worry about that with std::string.
 */

std::string
dcpgettext_expr
(
    const std::string & domain,
    const std::string & msgctxt,
    const std::string & msgid,
    int category
)
{
    std::string msg_ctxt_id = msgctxt;
    msg_ctxt_id[msg_ctxt_id.length() - 1] = '\004';
    msg_ctxt_id += msgid;

    std::string translation = dcgettext(domain, msg_ctxt_id, category);
    bool found_translation = translation != msg_ctxt_id;
    if (found_translation)
        return translation;

    return msgid;
}

std::string
dcnpgettext_expr
(
    const std::string & domain,
    const std::string & msgctxt,
    const std::string & msgid,
    const std::string & msgid_plural,
    unsigned long int n,
    int category
)
{
    bool found_translation;
    std::string msg_ctxt_id = msgctxt;
    msg_ctxt_id[msg_ctxt_id.length() - 1] = '\004';
    msg_ctxt_id += msgid;

    std::string translation = dcngettext
    (
        domain, msg_ctxt_id, msgid_plural, n, category
    );
    found_translation = !
    (
        translation == msg_ctxt_id || translation == msgid_plural
    );
    if (found_translation)
        return translation;

    return n == 1 ? msgid : msgid_plural;
}

}               // namespace po

/*
 * gettextex.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
