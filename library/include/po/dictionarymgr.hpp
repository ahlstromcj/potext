#if ! defined  POTEXT_PO_DICTIONARYMGR_HPP
#define POTEXT_PO_DICTIONARYMGR_HPP

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
 * \file          dictionarymgr.hpp
 *
 *      A refactoring of tinygettext::DictionaryManager.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-30
 * \license       See above.
 *
 *  Some additions have been made:
 *
 *      -   Storage of nlsbindings, for the gettext module.
 */

#include <deque>                        /* std::deque<> container template  */
#include <memory>                       /* std::unique_ptr<> template       */
#include <set>                          /* std::set<> template              */
#include <string>                       /* std::string                      */
#include <unordered_map>                /* std::unordered_map<> template    */

#include "po_types.hpp"                 /* observer_ptr<> template alias    */
#include "dictionary.hpp"               /* po::dictionary (Dictionary)      */
#include "language.hpp"                 /* po::language (Language)          */
#include "nlsbindings.hpp"              /* po::nlsbindings class            */

namespace po
{

class filesystem;

/**
 *  Provides the hash for the unordered map with language as the key.
 */

struct langhash
{
    size_t operator () (const language & v) const
    {
        return reinterpret_cast<size_t>(v.m_language_spec);
    }
};

/**
 *  Manager class for dictionaries, you give it a bunch of directories with
 *  .po files and it will then automatically load the right file on demand
 *  depending on which language was set.
 */

class dictionarymgr
{

private:

    /**
     *  Provides a shared pointer for dictionaries. Using a bare pointer
     *  is clumsy, and using a unique pointer is too problematic.
     */

    using dictpointer = std::shared_ptr<dictionary>;

    /**
     *  Provides a hashed map/list of dictionary pointers.
     */

    using dictionaries = std::unordered_map<language, dictpointer, langhash>;

    /**
     *  Provides a deque list of directories to search.
     */

    using searchpath = std::deque<std::string>;

    /**
     *  Increase the readability a little.
     */

    using fspointer = std::unique_ptr<filesystem>;

    /**
     *  The set of loaded dictionaries. Currently, they are added as needed by
     *  the get_dictionary() functions.
     */

    dictionaries m_dictionaries;

    /**
     *  Bindings read it for the gettext module. There will always be at least
     *  one.
     */

    nlsbindings m_nlsbindings;

    /**
     *  The current binding in force.  Also see m_current_language.
     */

    nlsbindings::binding m_current_binding;

    /**
     *  A stack of paths to search.  Added in add_directory()l Used in
     *  get_dictionary() to open a directory and get a list of all the first
     *  in it, and create a dictionary from the best match for the language
     *  object provided.
     */

    searchpath m_search_path;

    /**
     *  Indicates the character encoding in force. "UTF-8" by default.
     */

    std::string m_charset;

    /**
     *  Process any "fuzzy" indicators in the .po file and use them.
     */

    bool m_use_fuzzy;

    /**
     *  The current domain (package name or language).
     */

    std::string m_current_domain;

    /**
     *  The previous domain (package name or language).
     */

    std::string m_previous_domain;

    /**
     *  The current language in force.
     */

    language m_current_language;

    /**
     *  The current dictionary to use in get-text functions that do not accept
     *  a domain parameter.
     *
     *  This pointer is not shared. This pointer is owned by dictionaries
     *  m_dictionaries.
     *
     *      dictpointer m_current_dict;
     *
     *  See po_types.hpp.
     */

    observer_ptr<dictionary> m_current_dict;

    /**
     *  Provides a "pointer" to a po::filesystem object used to open a
     *  directory (using std::filesystem::directory_iterator(), and open a
     *  file.
     */

    fspointer m_filesystem;

private:

    dictionarymgr
    (
        fspointer filesystem,
        const std::string & chset = "UTF-8"
    );

public:

    dictionarymgr (const std::string & chset = "UTF-8");
    dictionarymgr (const dictionarymgr &) = delete;
    dictionarymgr (dictionarymgr &&) = delete;
    dictionarymgr & operator = (const dictionarymgr &) = delete;
    ~dictionarymgr ();

    bool empty () const
    {
        return m_dictionaries.empty();
    }

    dictionary & get_dictionary ();
    dictionary & get_dictionary (const language & lang);
    const dictionary & get_dictionary (const std::string & domainname) const;

    nlsbindings & get_bindings ()
    {
        return m_nlsbindings;
    }

    const nlsbindings & get_bindings () const
    {
        return m_nlsbindings;
    }

    const std::string & current_domain () const
    {
        return m_current_domain;
    }

    const std::string & previous_domain () const
    {
        return m_previous_domain;
    }

    /**
     *  Set a language based on a four? letter country code.
     */

    void set_language (const language & lang);

    /**
     *  Returns the (normalized) country code of the currently used language.
     */

    language get_language () const
    {
        return m_current_language;
    }

    void set_use_fuzzy (bool t)
    {
        clear_cache();
        m_use_fuzzy = t;
    }

    bool get_use_fuzzy () const
    {
        return m_use_fuzzy;
    }

    /**
     *  Set a charset that will be set on the returned dictionaries.
     */

    void set_charset (const std::string & charset)
    {
        clear_cache();              /* changing charset invalidates cache   */
        m_charset = charset;
    }

    void add_directory (const std::string & pathname, bool precedence = false);
    void remove_directory (const std::string & pathname);
    std::set<language> get_languages ();

    /*
     * Additional functions for potext.
     */

    bool add_dictionary_file (const std::string & fname);
    bool add_dictionaries
    (
        const std::string & dirname,
        const std::string & defaultdomain = ""
    );
    std::string textdomain (const std::string & domainname);
    std::string bindtextdomain
    (
        const std::string & domainname,
        const std::string & dirname
    );
    std::string bind_textdomain_codeset
    (
        const std::string & domainname,
        const std::string & codeset
    );

private:

    std::string filename_to_language (const std::string & s_in) const;
    void clear_cache ();
    dictpointer make_dictionary
    (
        language & polang,
        const std::string & pofile,
        const std::string & dirname
    );

};

}               // namespace po

#endif          // POTEXT_PO_DICTIONARYMGR_HPP

/*
 * dictionarymgr.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
