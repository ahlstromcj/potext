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
 * \updates       2024-04-12
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

/**
 *  Constructor.
 */

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

/**
 *  Writes an error to the error logstream, and then throws a parser_error().
 */

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
            << _("error") << ": " << m_filename << " "
            << _("line") << " " << pos << ": "
            << msg << std::endl
            ;
    }
    throw parser_error(msg);
}

/**
 *  Writes an error to the warning logstream, and then throws a
 *  parser_error().
 */

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

/**
 *  Applies the iconvert character-set conversion to all of the phrases
 *  in the phrase-list. Also calls fix_message() in case there are
 *  "\n" characters in the phrases.
 */

phraselist
pomoparserbase::convert_list (const phraselist & source)
{
    phraselist result;
    for (const auto & msg : source)
    {
        std::string cmsg = converter().convert(fix_message(msg));
        result.push_back(cmsg);
    }
    return result;
}

/**
 *  Fixes a message string by look for "\\n" sequences and changing them
 *  to actual newlines. This should be used on both the original message and
 *  the translated message.
 *
 *  Currently this is used in poparser, which converts the string "\n" to
 *  the the single-character string "\".
 */

std::string
pomoparserbase::fix_message (const std::string & msg)
{
    std::string result = msg;
    auto nlpos = result.find("\\");                     /* length is one    */
#if defined PLATFORM_DEBUG_TMI
    bool show = (msg.length() > 50 && nlpos != std::string::npos);
    if (show)
       std::cout << "Message in: '" << msg << std::endl;
#endif
    while (nlpos != std::string::npos)
    {
        result.replace(nlpos, 1, 1, '\n');              /* set to newline   */
        nlpos = result.find("\\", nlpos + 1);           /* find next po sep */
    }
#if defined PLATFORM_DEBUG_TMI
    if (show)
       std::cout << "Message out: '" << result << std::endl;
#endif
    return result;
}

}               // namespace po

/*
 * pomoparserbase.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
