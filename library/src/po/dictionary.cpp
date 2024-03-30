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
 * \file          dictionary.cpp
 *
 *      A refactoring of tinygettext::Dictionary
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-29
 * \license       See above.
 *
 */

#include "po/po_types.hpp"              /* po::phraselist vector            */
#include "po/dictionary.hpp"            /* po::dictionary class             */
#include "po/logstream.hpp"             /* po::logstream::error(), etc.     */

#if defined PLATFORM_DEBUG_TMI
#include <iostream>                     /* std::cout                        */
#endif

/**
 *  We cannot enable translation in this module, because it leads to
 *  recursion and a stack overflow. Oh well, it was worth a try.
 *  Can still grab messages for a dictionary, if desired.
 */

#if ! defined PO_HAVE_GETTEXT_RECURSIVE /* removed: po/gettext.hpp          */
#define _(str)      str
#endif

#if defined POTEX_USE_STATIC_OPERATOR_OSTREAM

/**
 *  Writes a list of phrases to the given output stream.
 *
 * \param o
 *      Provides the output stream to which to write.
 *
 * \param v
 *      Provides the list of phrases to write.
 *
 * \return
 *      Returns a reference to the output stream to facilitate a set of
 *      concatenated "<<" operators.
 */

static std::ostream &
operator << (std::ostream & o, const po::phraselist & v)
{
    int count = 0;
    for (const auto & it : v)
    {
        if (count > 0)
            o << ", ";

        o << "'" << it << "'";
        ++count;
    }
    return o;
}

#endif      // defined POTEX_USE_STATIC_OPERATOR_OSTREAM

/*
 *  The main namespace of this library. We could have followed Boost
 *  and name it "locale", we suppose.
 */

namespace po
{

/**
 *  Creates an empty dictionary.
 *
 * \param charset
 *      Provides the default character encoding ("codeset") to use.
 *      The default is "UTF-8".
 */

dictionary::dictionary (const std::string & charset) :
    m_entries           (),
    m_ctxt_entries      (),
    m_charset           (charset),
    m_plural_forms      (),
    m_has_fallback      (false),
    m_fallback          ()
{
    // no other code
}

/**
 *  Clears the normal message entries and the contextual message entries.
 */

void
dictionary::clear ()
{
    m_entries.clear();
    m_ctxt_entries.clear();
}

std::string
dictionary::translate (const std::string & msgid) const
{
    return translate(m_entries, msgid);
}

std::string
dictionary::translate (const entries & dict, const std::string & msgid) const
{
    entries::const_iterator i = dict.find(msgid);
    if (i != dict.end() && !i->second.empty())
    {
        return i->second[0];
    }
    else
    {
        logstream::warning()
            << _("Could not translate") << ": '" << msgid << "'"
            << std::endl
            ;
        if (m_has_fallback)
            return m_fallback->translate(msgid);
        else
            return msgid;
    }
}

/**
 *  Translate the string \a msgid to its correct plural form, based on the
 *  number of items given by \a num. \a msgid_plural is \a msgid in plural
 *  form.
 */

std::string
dictionary::translate_plural
(
    const std::string & msgid,
    const std::string & msgid_plural,
    int N
) const
{
    return translate_plural(m_entries, msgid, msgid_plural, N);
}

/**
 *  Handles a section like this in a po file:
 *
 *      msgid "Hello World"
 *      msgid_plural "Hello Worlds"
 *      msgstr[0] "Hallo Welt (singular)"
 *      msgstr[1] "Hallo Welt (plural)"
 *
 * \param dict
 *      The list of phraselists to use to do the lookup. See the
 *      translate_plural() overload above.
 *
 * \param msgid
 *      Provides the singular form of the message ID.
 *
 * \param msgid_plural
 *      Provides the plural form of the message ID.
 *
 * \param N
 *      This count parameter starts at 1, not 0. and indicates which plural
 *      form is desired. If no message catalog is found, msgid is returned
 *      if N == 1, otherwise msgid_plural is returned.
 */

std::string
dictionary::translate_plural
(
    const entries & dict,
    const std::string & msgid,
    const std::string & msgid_plural,
    int N
) const
{
    entries::const_iterator it = dict.find(msgid);
    if (it != dict.end())
    {
        unsigned n = m_plural_forms.get_plural(N);
        const phraselist & msgstrs = it->second;
        if (n >= unsigned(msgstrs.size()))
        {
            logstream::error()
                << _("Plural index exceeds existing translations")
                << ": '"
                << msgid << ", #" << n << std::endl
                ;
            return msgid;
        }
        if (! msgstrs[n].empty())
            return msgstrs[n];
        else if (N == 1)                /* default to english rules         */
            return msgid;
        else
            return msgid_plural;
    }
    else
    {
        logstream::warning()
            << _("Could not translate plural for") << ": '" << msgid << "'\n"
            << _("Candidates") << ":\n"
            ;
        for (const auto & it : dict)
            logstream::info() << "'" << it.first << "'" << std::endl;

        if (N == 1)                     /* default to english rules         */
            return msgid;
        else
            return msgid_plural;
    }
}

/**
 *  Translate the string \a msgid that is in context \a msgctx. A context
 *  is a way to disambiguate msgids that contain the same letters, but
 *  different meaning. For example "exit" might mean to quit doing
 *  something or it might refer to a door that leads outside (i.e.
 *  'Ausgang' vs 'Beenden' in german).
 */

std::string
dictionary::translate_ctxt
(
    const std::string & msgctxt,
    const std::string & msgid
) const
{
    auto i = m_ctxt_entries.find(msgctxt);
    if (i != m_ctxt_entries.end())
    {
        return translate(i->second, msgid);
    }
    else
    {
        logstream::warning()
            << _("Could not translate in context") <<
            " '" << msgctxt << "': '" << msgid
            << "'" << std::endl
            ;
        return msgid;
    }
}

std::string
dictionary::translate_ctxt_plural
(
    const std::string & msgctxt,
    const std::string & msgid,
    const std::string & msgidplural,
    int num
) const
{
    auto i = m_ctxt_entries.find(msgctxt);
    if (i != m_ctxt_entries.end())
    {
        return translate_plural(i->second, msgid, msgidplural, num);
    }
    else
    {
        logstream::warning()
            << _("Could not translate in context") << " '"
            << msgctxt << "': '" << msgid
            << "' " << _("and") << " '" << msgidplural << "'"
            << std::endl
            ;
        if (num != 1)                       /* default to English */
            return msgidplural;
        else
            return msgid;
    }
}

#if defined PLATFORM_DEBUG_TMI

/**
 *  For debugging only.
 */

static void
show_pair (const std::string & msg1, const std::string & msg2)
{
    std::string m1 = msg1.substr(0, 16);
    std::string m2 = msg2.substr(0, 16);
    std::cout
        << "  \"" << m1 << "\" & \"" << m2 << "\"" << std::endl;
}

static void
show_phraselist (const phraselist & msgstrs)
{
    std::cout << "  Phrases: ";
    for (const auto & msg : msgstrs)
    {
        std::string m1 = msg.substr(0, 16);
        std::cout << " \"" << m1 << "\";";
    }
    std::cout << std::endl;
}

#endif

/**
 *  Add a translation from \a msgid to \a msgstr to the dictionary.
 *  This is the basic add() function. Remember that m_entries is a map
 *  of phraselists. Each message ID (key value) is associated with one or
 *  more translations. Here, only one translation is possible.
 */

bool
dictionary::add
(
    const std::string & msgid,
    const std::string & msgstr
)
{
#if defined USE_OLD_CODE
    phraselist & vec = m_entries[msgid];
    if (vec.empty())
    {
        vec.push_back(msgstr);
    }
    else if (vec[0] != msgstr)
    {
        logstream::warning()
            << _("Collision in") << " add(): '"
            << msgid << "' -> '" << msgstr << "' vs '" << vec[0] << "'"
            << std::endl
            ;
        vec[0] = msgstr;
    }
#else
    bool result = false;
    auto it = m_entries.find(msgid);
    if (it != m_entries.end())
    {
        logstream::warning()
            << _("Collision in") << " add(" << msgid << ", " << msgstr << ")"
            << std::endl
            ;
    }
    else
    {
        phraselist msgstrs;
        msgstrs.push_back(msgstr);
        m_entries[msgid] = msgstrs;
        result = true;
#if defined PLATFORM_DEBUG_TMI
        show_pair(msgid, msgstr);
#endif
    }
    return result;
#endif
}

/**
 *  Add a translation from \a msgid to \a msgstr to the dictionary, where \a
 *  msgid is the singular form of the message, msgid_plural the plural form
 *  and msgstrs a table of translations. The right translation will be
 *  calculated based on the \a num argument to translate().
 *
 *  This function is meant to add phraselist (i.e. vector<string>) to
 *  m_entries.  m_entries is an unordered_map<string, vector<string>>, where
 *  the key is the message ID.
 *
 *  We have changed the method of insertion to avoid "collisions", though we
 *  want to tell the user about them.
 *
 *  The given msgid parameter is looked up.
 */

bool
dictionary::add
(
    const std::string & msgid,
    const std::string & msgid_plural,
    const phraselist & msgstrs
)
{
#if defined USE_OLD_CODE
    phraselist & vec = m_entries[msgid];
    if (vec.empty())
    {
        vec = msgstrs;
    }
    else if (vec != msgstrs)
    {
        logstream::warning()
            << _("Collision in") << " add(): '"
            << msgid << "', '" << msgid_plural
            << "' -> [" << vec << "] vs [" << msgstrs << "]" << std::endl
            ;
        vec = msgstrs;
    }
#else
    bool result = false;
    auto it = m_entries.find(msgid);
    if (it != m_entries.end())
    {
        logstream::warning()
            << _("Collision in plural") << " add(" << msgid << ", "
            << msgid_plural << ")" << std::endl
            ;
    }
    else
    {
        m_entries[msgid] = msgstrs;
        result = true;
#if defined PLATFORM_DEBUG_TMI
        show_pair(msgid, msgid_plural);
        show_phraselist(msgstrs);
#endif
    }
    return result;
#endif
}

/**
 *  Message context.
 */

bool
dictionary::add
(
    const std::string & msgctxt,
    const std::string & msgid,
    const std::string & msgstr
)
{
    phraselist & phrases = m_ctxt_entries[msgctxt][msgid];
    if (phrases.empty())
    {
        phrases.push_back(msgstr);
    }
    else if (phrases[0] != msgstr)
    {
        logstream::warning()
            << _("Collision in context") << " add("
            << msgctxt << ", " << msgid << ", " << msgstr << ")"
            << std::endl
            ;
        phrases[0] = msgstr;
    }
    return true;
}

/**
 *  Message context and plural forms. Note that each value of
 *  m_ctxt_entries[ctxt] is a list of the entries (phraselists keyed by the
 *  message ID) for that context string. Then m_ctxt_entries[ctxt][id]
 *  is the list of phrases for that context and message ID.
 */

bool
dictionary::add
(
    const std::string & msgctxt,
    const std::string & msgid,
    const std::string & msgid_plural,
    const phraselist & msgstrs
)
{
    phraselist & phrases = m_ctxt_entries[msgctxt][msgid];
    if (phrases.empty())
    {
        phrases = msgstrs;
    }
    else if (phrases != msgstrs)
    {
        logstream::warning()
            << _("Collision in context plural") << " add("
            << msgctxt << ", " << msgid << ", " << msgid_plural << ")"
            << std::endl
            ;
        phrases = msgstrs;
    }
    return true;
}

}           // namespace po

/*
 * dictionary.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
