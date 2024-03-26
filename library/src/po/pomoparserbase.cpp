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
 * \file          pomoparserbase.cpp
 *
 *      The po::pomoparserbase class is a base class for po::poparser and
 *      po::moparser.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-03-26
 * \updates       2024-03-26
 * \license       See above.
 *
 */

#include <cstring>                      /* std::strlen() etc.               */
#include <iostream>                     /* std::istream, std::ostream       */

#include "po/dictionary.hpp"            /* po::dictionary class             */
#include "po/logstream.hpp"             /* po::logstream log-access funcs   */
#include "po/pluralforms.hpp"           /* po::pluralforms class            */
#include "po/pomoparserbase.hpp"        /* po::pomoparserbase class         */

/**
 *  We cannot enable translation in this module, because it leads to
 *  recursion and a stack overflow. Oh well, it was worth a try.
 *  Can still grab messages for a dictionary, if desired.
 */

#if ! defined PO_HAVE_GETTEXT_RECURSIVE
#define _(str)      str
#endif

namespace po
{

/**
 *  An internal flag to handle present-but-empty "msgctxt" tags.
 */

#define MSGCTXT_EMPTY_FLAG      "-"

/**
 *  Currently always true.
 */

bool pomoparserbase::sm_pedantic = true;

pomoparserbase::pomoparserbase
(
    const std::string & filename,
    std::istream & in,
    dictionary & dic,
    bool usefuzzy
) :
    m_filename      (filename),
    m_in_stream     (in),
    m_dict          (dic),
    m_converter     (filename),
    m_use_fuzzy     (usefuzzy)
{
    // no code
}

void
pomoparserbase::error (const std::string & msg, std::size_t pos)
{
    std::ostream & err = logstream::error();
    if (msg.empty())
    {
        err << _("error msg empty") << std::endl;
    }
    else
    {
        err
            << m_filename << ":" << pos << ":\n"
            << _("error") << ": "
            << msg << std::endl
            ;
    }
    throw internal_parser_error();
}

void
pomoparserbase::warning (const std::string & msg, std::size_t pos)
{
    std::ostream & warn = logstream::warning();
    if (msg.empty())
    {
        warn << _("warning msg empty") << "!" << std::endl;
    }
    else
    {
        warn
            << m_filename << ":" << pos << ":\n"
            << "    " << _("warning") << ": "
            << msg << std::endl
            ;
    }
}

}               // namespace po

/*
 * pomoparserbase.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
