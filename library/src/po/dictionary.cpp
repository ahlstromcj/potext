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
 * \updates       2024-04-13
 * \license       See above.
 *
 */

#include "po/po_types.hpp"              /* po::phraselist vector            */
#include "po/dictionary.hpp"            /* po::dictionary class             */
#include "po/logstream.hpp"             /* po::logstream::error(), etc.     */

#if defined PLATFORM_DEBUG_TMI
#include <iostream>                     /* std::cout                        */
#endif

#if defined POTEXT_DICTIONARY_CREATE_PO_DUMP
#include <cstring>                      /* std::memset()                    */
#include <ctime>                        /* std::strftime()                  */
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
    m_file_mode         (mode::none),
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
    m_file_mode = mode::none;
    m_entries.clear();
    m_ctxt_entries.clear();
}

/**
 *  Translate an entry in this dictionary. Short overload.
 */

std::string
dictionary::translate (const std::string & msgid) const
{
    return translate(m_entries, msgid);
}

/**
 *  Translate an entry in this dictionary without regard to context or plural
 *  forms.
 */

std::string
dictionary::translate (const entries & ents, const std::string & msgid) const
{
    entries::const_iterator it = ents.find(msgid);
    if (it != ents.end() && ! it->second.phrase_list.empty())
    {
        return it->second.phrase_list[0];
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
 *
 * \param msgid
 *      The English version of the string, passed to a Gettext function.
 *
 * \param msgid_plural
 *      The plural form to display in warnings.  Provides the plural form
 *      of the message ID.
 *
 * \param N
 *      The index (starting at 1) to the plural form to look up.
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
 *  Note that the number of translations in the phrase list.......
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
 *      This count parameter starts at 1, not 0, and indicates which plural
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
        const phraselist & msgstrs = it->second.phrase_list;
        unsigned sz = unsigned(msgstrs.size());
        if (n >= sz)
        {
            logstream::error()
                << _("Plural index") << " " << n << " "
                << _("exceeds translation count")
                << " " << sz << ": '"
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
 *  'Ausgang' vs 'Beenden' in German).
 *
 *  In translate(it->second, msgid), the iterator points to a ctxtentry,
 *  and second is an entries map.
 */

std::string
dictionary::translate_ctxt
(
    const std::string & msgctxt,
    const std::string & msgid
) const
{
    auto it = m_ctxt_entries.find(msgctxt);
    if (it != m_ctxt_entries.end())
    {
        return translate(it->second, msgid);
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
    auto it = m_ctxt_entries.find(msgctxt);
    if (it != m_ctxt_entries.end())
    {
        return translate_plural(it->second, msgid, msgidplural, num);
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

#if defined POTEXT_DICTIONARY_CREATE_PO_DUMP

/**
 *  Gets the current date/time. Copped from
 *
 *          seq66/libseq66/src/util/filefunctions.cpp
 *
 * \return
 *      Returns the current date and time as a string.
 */

static std::string
current_date_time ()
{
    static char s_temp[64];
    static const char * const s_format = "%Y-%m-%d %H:%M:%S";
    time_t t;
    std::memset(s_temp, 0, sizeof s_temp);
    time(&t);

    struct tm * tm = localtime(&t);
    std::strftime(s_temp, sizeof s_temp - 1, s_format, tm);
    return std::string(s_temp);
}

/**
 *  The first message ID is empty and the translations is the header
 *  information in a .po or .mo file. In the .po file the first msgid
 *  and msgstr are empty, and the header follows, with each line quoted
 *  and ending with the literal escape-sequence string "\n".
 *
 *  Here we add a double-quote at the beginning of each line, and
 *  we insert the literal string '\n"' before the newline character.
 */

static std::string
enquote_string (const std::string & msg)
{
    auto nlpos = msg.find_first_of("\n");
    if (nlpos != std::string::npos)
    {
        std::string result;
        std::string::size_type index = 0;
        while (nlpos != std::string::npos)
        {
            result += "\"";                         /* add starting quote   */
            result += msg.substr(index, nlpos - index);
            result += "\\n\"\n";                    /* append '\n"' + "\n"  */
            index = nlpos + 1;
            nlpos = msg.find_first_of("\n", index);
        }
        return result;
    }
    else
        return msg;
}

/**
 *  Converts a newline to the \n escape sequence, in place. Also prepends
 *  a backslash to each double-quote character.
 *
 *  These changes are needed to match what a pofile should contain.
 */

static void
fix_escapes (std::string & msg)
{
    if (! msg.empty())
    {
        if (msg.back() == '\n')
        {
            msg.pop_back();
            msg += "\\n";
        }

        auto qpos = msg.find_first_of("\"");
        while (qpos != std::string::npos)
        {
            msg.insert(qpos, 1, '\\');          /* moves the quote up one   */
            qpos = msg.find_first_of("\"", qpos + 2);
        }
    }
}

/**
 *  Writes a single translation stanza (entry) to a string. The items
 *  are quoted. Any newlines are converted to the "\n" escape sequence.
 */

std::string
dictionary::message_entry
(
    const std::string & msgid,
    const entry & ent
) const
{
    std::string msgid_copy{msgid};
    std::string msgid_plural{ent.msgid_plural};
    const po::phraselist & msgstrs{ent.phrase_list};
    std::string result{"msgid \""};
    fix_escapes(msgid_copy);
    result += msgid_copy;
    result += "\"\n";
    if (! msgid_plural.empty())
    {
        fix_escapes(msgid_plural);
        result += "msgid_plural \"";
        result += msgid_plural;
        result += "\"\n";
    }
    if (! msgstrs.empty())
    {
        if (msgstrs.size() == 1)
        {
            std::string msgstr = msgstrs[0];
            if (msgid.empty())
            {
                result += "msgstr \"\"\n";
//              if (mo_file_mode())
                    msgstr = enquote_string(msgstr);

                result += msgstr;
            }
            else
            {
                fix_escapes(msgstr);
                result += "msgstr \"";
                result += msgstr;
                result += "\"\n";
            }
        }
        else
        {
            std::size_t count = 0;
            for (auto msg : msgstrs)        /* makes a copy */
            {
                fix_escapes(msg);
                result += "msgstr[";
                result += std::to_string(count++);
                result += "] \"";
                result += msg;
                result += "\"\n";
            }
        }
    }
    return result;
}

/**
 *  Writes a single translation stanza (entry) with context to a string.
 */

std::string
dictionary::message_ctxt_entry
(
    const std::string & ctxt,
    const std::string & msgid,
    const entry & ent
) const
{
    std::string result{"msgctxt \""};
    result += ctxt;
    result += "\"\n";
    message_entry(msgid, ent);
    return result;
}

/**
 *  Iterates through the dictionary entries, appending them, with
 *  suitable modification, to a string. This provides a reasonable
 *  recreation of a .po file.
 *
 *  Tested in library/tests/potext_test using the "list-msgstrs" ("lm"
 *  for short) option.
 */

std::string
dictionary::create_po_dump () const
{
    std::string result{"# Data created by dictionary::create_po_dump()\n"};
    result += "# ";
    result += current_date_time();
    result += "\n\n";
    for (const auto & e : m_entries)
    {
        result += message_entry(e.first, e.second);
        result += "\n";
    }

    for (const auto & i : m_ctxt_entries)               /* ctxtentries list */
    {
        for (const auto & j : i.second)                 /* entries list     */
        {
            message_ctxt_entry(i.first, j.first, j.second);
            result += "\n";
        }
    }
    return result;
}

#endif  // defined POTEXT_DICTIONARY_CREATE_PO_DUMP

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
        entry en;                       /* empty plural ID plus phrase list */
        en.phrase_list.push_back(msgstr);
        m_entries[msgid] = en;
        result = true;
#if defined PLATFORM_DEBUG_TMI
        show_pair(msgid, msgstr);
#endif
    }
    return result;
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
 *
 * TODO: Shouldn't we make a new phraselist by converting the charset of each
 * phrase?  Or is this done in poparser?
 *
 * \param msgid
 *      The original search string.
 *
 * \param msgid_plural
 *      Used only for warning output, same as in tinygettext. It refers to the
 *      English version of the plural. The selection of the plural form(s) is
 *      triggered by explicit function calls. Only the singular form takes
 *      part in the lookup.
 *
 * \param msgstrs
 *      Provides a list of the various plural forms translations of the plural
 *      message ID.
 */

bool
dictionary::add
(
    const std::string & msgid,
    const std::string & msgid_plural,
    const phraselist & msgstrs
)
{
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
        entry en;                       /* empty plural ID plus phrase list */
        en.msgid_plural = msgid_plural;
        en.phrase_list = msgstrs;
        m_entries[msgid] = en;
        result = true;
#if defined PLATFORM_DEBUG_TMI
        show_pair(msgid, msgid_plural);
        show_phraselist(msgstrs);
#endif
    }
    return result;
}

/**
 *  Message context. The new entry has an empty msgid_plural string.
 */

bool
dictionary::add
(
    const std::string & msgctxt,
    const std::string & msgid,
    const std::string & msgstr
)
{
    entry & ent = m_ctxt_entries[msgctxt][msgid];
    phraselist & phrases = ent.phrase_list;
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
 *
 *      entries & cents = m_ctxt_entries[msgctxt];
 *      entry & ent = m_ctxt_entries[msgctxt][msgid];
 *      phraselist & phrases = m_ctxt_entries[msgctxt][msgid].phrase_list;
 *
 * \param msgctxt
 *      Provides the context string to use to select the proper translation.
 *
 * \param msgid
 *      The English form of the message.
 *
 * \param msgid_plural
 *      No longer only for warning output, as done in tinygettext. It refers
 *      to the English version of the plural. The selection of the plural
 *      form(s) is triggered by explicit function calls. Only the singular
 *      form takes part in the lookup.
 *
 * \param msgstrs
 *      Provides a list of the singular form plus the various plural forms
 *      translations of the plural message ID.
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
    entry & ent = m_ctxt_entries[msgctxt][msgid];
    phraselist & phrases = ent.phrase_list;
    if (phrases.empty())                    /* i.e. a new ctxt/msgid combo  */
    {
        ent.msgid_plural = msgid_plural;
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
