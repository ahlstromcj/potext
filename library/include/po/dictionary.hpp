#if ! defined  POTEXT_PO_DICTIONARY_HPP
#define POTEXT_PO_DICTIONARY_HPP

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
 * \file          dictionary.hpp
 *
 *  Macros that depend upon the build platform.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-04-09
 * \license       See above.
 *
 */

#include <string>

#include "c_macros.h"                   /* not_nullptr() macro function     */
#include "platform_macros.h"            /* OS & debug macroes etc.          */
#include "po_build_macros.h"            /* feature macros with explanations */
#include "po/po_types.hpp"              /* po::phraselist string-vector     */
#include "po/pluralforms.hpp"           /* po::pluralforms class            */

#if defined POTEXT_USE_UNORDERED_DICTIONARY
#include <unordered_map>
#else
#include <map>
#endif

namespace po
{

/**
 *  A simple dictionary class that mimics gettext() behaviour. Each
 *  dictionary only works for a single language, for managing multiple
 *  languages and .po files at once use the dictionarymgr.
 */

class dictionary
{

private:

    /**
     *  We want to be able to store the plural message ID. So instead
     *  of just a phrase list, we add that string and use this structure.
     */

    using entry = struct
    {
        std::string msgid_plural;
        phraselist phrase_list;
    };

    /*
     * Replacing this with a construct that lets us capture the
     * msgid_plural.
     *
     *      using entries = std::unordered_map<std::string, phraselist>;
     */

#if defined POTEXT_USE_UNORDERED_DICTIONARY
    using entries = std::unordered_map<std::string, entry>;
    using ctxtentries = std::unordered_map<std::string, entries>;
#else
    using entries = std::map<std::string, entry>;
    using ctxtentries = std::map<std::string, entries>;
#endif

    /**
     *  Holds a set of phraselists, each of which is a vector of message
     *  strings keyed by a string.
     */

    entries m_entries;

    /**
     *  Holds a list of sets of entries tied to a specific translation.
     */

    ctxtentries m_ctxt_entries;

    /**
     *  The character encoding to apply (if not UTF-8) to translated output.
     */

    std::string m_charset;

    /**
     *  Provides an object holding a count of plurals and the function used
     *  for accessing one of the plurals.  See the from_string() function
     *  in pluralforms.cpp, and also see GNU gettext manual section 12.6.
     */

    pluralforms m_plural_forms;

    /**
     *  Indicates whether a fall-back dictionary has been logged, or not.
     */

    bool m_has_fallback;

    /**
     *  Points to a dictionary to use when a lookup fails for a dictionary.
     *  If not null, then that dictionary will attempt to translate a
     *  message.
     */

    dictionary * m_fallback;

public:

    /**
     *  Constructs a dictionary converting to the specified \a charset (default
     *  UTF-8).
     */

    dictionary (const std::string & charset = "UTF-8");
    dictionary (const dictionary &) = delete;
    dictionary (dictionary &&) = delete;
    dictionary & operator = (const dictionary &) = delete;
    ~dictionary () = default;

    void clear ();

    bool empty () const
    {
        return m_entries.empty();
    }

    std::string get_charset () const
    {
        return m_charset;
    }

    /**
     *  Registers the plural forms object for this dictionary. Recall that the
     *  Plural-Forms function is specified in the *.po file's header section.
     *
     * \param plural_forms
     *      The pluralforms object to register. Note that pluralforms supports
     *      copy construction and assignment.
     */

    void set_plural_forms (const pluralforms & pf)
    {
        m_plural_forms = pf;
    }

    /**
     *  I think we can make this return a reference.
     */

    const pluralforms & get_plural_forms () const
    {
        return m_plural_forms;
    }

    std::string translate (const std::string & msgid) const;
    std::string translate_plural
    (
        const std::string & msgid,
        const std::string & msgidplural,
        int num
    ) const;
    std::string translate_ctxt
    (
        const std::string & msgctxt,
        const std::string & msgid
    ) const;
    std::string translate_ctxt_plural
    (
        const std::string & msgctxt,
        const std::string & msgid,
        const std::string & msgidplural,
        int num
    ) const;

    bool add
    (
        const std::string & msgid,
        const std::string & msgid_plural,
        const phraselist & msgstrs
    );
    bool add
    (
        const std::string & msgctxt,
        const std::string & msgid,
        const std::string & msgid_plural,
        const phraselist & msgstrs
    );
    bool add
    (
        const std::string & msgid,
        const std::string & msgstr
    );
    bool add
    (
        const std::string & msgctxt,
        const std::string & msgid,
        const std::string & msgstr
    );
    bool add_fallback_dictionary (dictionary * fallback)
    {
        m_has_fallback = not_nullptr(fallback);
        m_fallback = fallback;
        return m_has_fallback;
    }

    /**
     *  Iterate over all messages, FUNC is of type:
     *
     *      void func
     *      (
     *          const std::string & msgid,
     *          const std::string & msgid_plural,
     *          const std::vector<std::string> & msgstrs
     *      )
     *
     *  std::vector<std::string> is po::phraselist
     */

    template<class FUNC>
    FUNC foreach (FUNC func)
    {
        for (const auto & e : m_entries)
        {
            func(e.first, e.second.msgid_plural, e.second.phrase_list);
        }
        return func;
    }

    /**
     *  Iterate over all messages with a context, FUNC is of type:
     *
     *      void func
     *      (
     *          const std::string & ctxt,
     *          const std::string & msgid,
     *          const phraselist & msgstrs
     *      )
     */

    template<class FUNC>
    FUNC foreach_ctxt (FUNC func)
    {
        for (const auto & i : m_ctxt_entries)
        {
            for (const auto & j : i.second)
            {
                func
                (
                    i.first, j.first, j.second.msgid_plural,
                    j.second.phrase_list
                );
            }
        }
        return func;
    }

private:

    std::string translate
    (
        const entries & dict,
        const std::string & msgid
    ) const;
    std::string translate_plural
    (
        const entries & dict,
        const std::string & msgid,
        const std::string & msgidplural,
        int num
    ) const;

};              // class dictionary

}               // namespace po

#endif          // POTEXT_PO_DICTIONARY_HPP

/*
 * dictionary.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
