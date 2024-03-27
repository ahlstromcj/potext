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
 * \file          dictionarymgr.cpp
 *
 *      A refactoring of tinygettext::DictionaryManager.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-27
 * \license       See above.
 *
 */

#include <algorithm>
#include <cctype>
#include <fstream>

#include "po/dictionarymgr.hpp"         /* po::dictionarymgr class          */
#include "po/logstream.hpp"             /* po::logstream::error(), etc.     */
#include "po/moparser.hpp"              /* po::moparser class               */
#include "po/poparser.hpp"              /* po::poparser class               */
#include "po/unixfilesystem.hpp"        /* po::unixfilesystem class         */

/**
 *  We cannot enable translation in this module, because it leads to
 *  recursion and a stack overflow. Oh well, it was worth a try.
 *  Can still grab messages for a dictionary, if desired.
 */

#if ! defined PO_HAVE_GETTEXT_RECURSIVE
#define _(str)      str
#endif

/**
 *  For the most part, these limits are sanity checks.
 */

#if defined PLATFORM_WINDOWS            /* Microsoft platform               */
#define p_max_path       260            /* _MAX_PATH: Microsoft's path size */
#else
#include <unistd.h>
#define p_max_path      1024            /* PATH_MAX: POSIX; 4096 Linux      */
#endif

namespace po
{

/**
 *  Tests if lhs ends with rhs.
 *
 *  Can we use operator <=> here?
 */

static bool
has_suffix (const std::string & lhs, const std::string & rhs)
{
    if (lhs.length() < rhs.length())
    {
        return false;
    }
    else
    {
        size_t diff = lhs.length() - rhs.length();
        return lhs.compare(diff, rhs.length(), rhs) == 0;
    }
}

/**
 *  A dictionary to return when none are available.
 */

static dictionary &
empty_dictionary ()
{
    static dictionary s_empty_dictionary;
    return s_empty_dictionary;
}

/**
 *  This constructor uses a C++11 feature called "constructor delegation".
 */

dictionarymgr::dictionarymgr (const std::string & charset) :
    dictionarymgr
    (
        fspointer(new unixfilesystem), charset
    )
{
    // no other code
}

dictionarymgr::dictionarymgr
(
    fspointer filesys,
    const std::string & charset
) :
    m_dictionaries      (),                     /* set of shared pointers   */
    m_nlsbindings       (),
    m_current_binding   (),
    m_search_path       (),
    m_charset           (charset),
    m_use_fuzzy         (true),
    m_current_domain    (),
    m_previous_domain   (),
    m_current_language  (),
    m_current_dict      (nullptr),              /* a single unique pointer  */
    m_filesystem        (std::move(filesys))
{
    // no other code
}

dictionarymgr::~dictionarymgr ()
{
    /*
     * No longer needed with shared (and purely internal) pointers.
     *
     *  for (auto i = m_dictionaries.begin(); i != m_dictionaries.end(); ++i)
     *      delete i->second;
     */
}

void
dictionarymgr::clear_cache ()
{
    /*
     *  for (auto i = m_dictionaries.begin(); i != m_dictionaries.end(); ++i)
     *      delete i->second;
     */

    m_dictionaries.clear();             /* destroys all the shared pointers */
    m_current_dict = nullptr;           /* nullify this observer_ptr<>      */
}

/**
 *  Return the currently active dictionary, if none is set, an empty
 *  dictionary is returned.
 *
 *  In effect, this is the "main dictionary".
 */

dictionary &
dictionarymgr::get_dictionary ()
{
    if (not_nullptr(m_current_dict))
    {
        return *m_current_dict;
    }
    else
    {
        if (m_current_language)
        {
            /*
             * Make sure this is on the heap!!!
             *
             * m_current_dict.reset(&get_dictionary(m_current_language));
             */

            m_current_dict = &get_dictionary(m_current_language);
            return *m_current_dict;
        }
        else
            return empty_dictionary();
    }
}

/**
 *  Get dictionary for language. If one is not found, make one.
 */

dictionary &
dictionarymgr::get_dictionary (const language & lang)
{
    auto di = m_dictionaries.find(lang);
    if (di != m_dictionaries.end())
    {
        return *di->second;
    }
    else                /* lang dictionary isn't loaded, so load it     */
    {
        /*
         * Had to refactor this code in order to make the change to a
         * shared pointer work. We also need the semantics of lookup or
         * insert and return the entity in the map.
         *
         *  dictpointer dict.reset(new (std::nothrow) dictionary(m_charset));
         *  m_dictionaries[lang] = dict;
         */

        dictpointer d = std::make_shared<dictionary>();
        auto ldp = std::make_pair(lang, std::move(d));
        auto inserted_item = m_dictionaries.insert(ldp);
        dictpointer dict;
        if (inserted_item.second)
        {
            auto di = m_dictionaries.find(lang);
            dict = di->second;
        }
        else
            return empty_dictionary();

        for (auto p = m_search_path.rbegin(); p != m_search_path.rend(); ++p)
        {
            /*
             * Could replace this with a helper function TBD.
             */

            phraselist files = m_filesystem->open_directory(*p);
            std::string best_filename;
            int best_score = 0;
            bool has_po = false;
            bool has_mo = false;
            for (const auto & fname : files)
            {
                /*
                 * Check if fname matches requested lang; ignore anything that
                 * isn't a .po file or an .mo file.
                 */

                has_po = has_suffix(fname, ".po");
                has_mo = has_suffix(fname, ".mo");
                if (has_po || has_mo)
                {
                    language po_lang = language::from_env
                    (
                        filename_to_language(fname)
                    );
                    if (! po_lang)
                    {
                        logstream::warning()
                            << fname << ": "
                            << _("ignoring unknown language") << std::endl
                            ;
                    }
                    else
                    {
                        int score = language::match(lang, po_lang);
                        if (score > best_score)
                        {
                            best_score = score;
                            best_filename = fname;
                        }
                    }
                }
            }
            if (! best_filename.empty())
            {
                std::string pofile = *p;        /* now can be .mo file too  */
                pofile += "/";
                pofile += best_filename;
                try
                {
                    uistream_ptr in = m_filesystem->open_file(pofile);
                    if (! in)
                    {
                        logstream::error()
                            << _("error") << ": " << _("failure opening")
                            << ": " << pofile << std::endl
                            ;
                    }
                    else
                    {
                        if (has_po)
                            (void) poparser::parse_po_file(pofile, *in, *dict);
                        else if (has_po)
                            (void) moparser::parse_mo_file(pofile, *in, *dict);
                    }
                }
                catch (const std::exception & e)
                {
                    logstream::error()
                        << _("error") << ": " << _("failure parsing")
                        << ": " << pofile << "\n"
                        << e.what() << "" << std::endl
                        ;
                }
            }
        }
        if (! lang.get_country().empty())
        {
            (void) dict->add_fallback_dictionary
            (
                &get_dictionary(language::from_spec(lang.get_language()))
            );
        }
        return *dict;
    }
}

/**
 *  We might need to pull out country and modifier from the domainname
 *  parameter.
 *
 *      dictionary & result = get_dictionary(lobj);
 *
 *  Note that this function assumes all dictionaries already have been
 *  loaded.
 */

const dictionary &
dictionarymgr::get_dictionary (const std::string & domainname) const
{
    std::string lang = domainname;
    std::string country;
    std::string modifier;
    language /*&*/ lobj = language::from_spec(lang, country, modifier);
    const auto di = m_dictionaries.find(lobj);
    return di != m_dictionaries.end() ? *di->second : empty_dictionary() ;
}

/**
 *  Return a set of the available languages in their country code.
 */

std::set<language>
dictionarymgr::get_languages ()
{
    std::set<language> langs;
    for
    (
        searchpath::iterator p = m_search_path.begin();
        p != m_search_path.end(); ++p
    )
    {
        phraselist files = m_filesystem->open_directory(*p);
        for (const auto & file : files)
        {
            if (has_suffix(file, ".po"))
            {
                std::string basename = file.substr(0, file.size() - 3);
                if (basename.empty())
                {
                    logstream::warning()
                        << _("empty language base-name") << std::endl
                        ;
                }
                else
                {
                    langs.insert(language::from_env(basename));

                    /*
                     * TODO? Add dictionary.
                     */
                }
            }
        }
    }
    return langs;
}

/**
 *  Why nullify the current dict?
 */

void
dictionarymgr::set_language (const language & lang)
{
    if (m_current_language != lang)
    {
        m_current_language = lang;
        m_current_dict = nullptr;
    }
}

/**
 *  Add a directory to the search path for dictionaries, earlier added
 *  directories have higher priority then later added ones.  Set
 *  precedence to true to invert this for a single addition.
 */

void
dictionarymgr::add_directory
(
    const std::string & pathname,
    bool precedence
)
{
    auto p = std::find(m_search_path.begin(), m_search_path.end(), pathname);
    if (p == m_search_path.end())
    {
        clear_cache();          // adding directories invalidates cache
        if (precedence)
            m_search_path.push_front(pathname);
        else
            m_search_path.push_back(pathname);
    }
}

/**
 *  Remove a directory from the search path.
 */

void
dictionarymgr::remove_directory (const std::string & pathname)
{
    searchpath::iterator it = find
    (
        m_search_path.begin(), m_search_path.end(), pathname
    );
    if (it != m_search_path.end())
    {
        clear_cache();          // removing directories invalidates cache
        m_search_path.erase(it);
    }
}

/**
 *  This function converts a .po filename (e.g. zh_TW.po) into a lang
 *  specification (zh_TW). On case insensitive file systems (think Windows) the
 *  filename and therefore the country specification is lower case (zh_tw). It
 *  Converts the lower case characters of the country back to upper case,
 *  otherwise potext (tinygettext) does not identify the country correctly.
 */

std::string
dictionarymgr::filename_to_language (const std::string & s_in) const
{
    std::string s;
    if(s_in.substr(s_in.size()-3, 3)==".po")
        s = s_in.substr(0, s_in.size()-3);
    else
        s = s_in;

    bool underscore_found = false;
    for (unsigned int i = 0; i < s.size(); ++i)
    {
        if (underscore_found)
        {
            /*
             * If we get a non-alphanumerical character/ we are done
             * (en_GB.UTF-8) - only convert the 'gb' part ... if we ever get
             * this kind of filename.
             */

            if (! std::isalpha(s[i]))
                break;

            s[i] = static_cast<char>(::toupper(s[i]));
        }
        else
            underscore_found = s[i]=='_';
    }
    return s;
}

/*--------------------------------------------------------------------------
 * Additional functions specific to potext
 *--------------------------------------------------------------------------*/

dictionarymgr::dictpointer
dictionarymgr::make_dictionary
(
    language & polang,
    const std::string & pofile,
    const std::string & dirname
)
{
    dictpointer result;
    try
    {
        uistream_ptr in = m_filesystem->open_file(pofile);
        if (! in)
        {
            logstream::error()
                << _("error") << ": " << _("failure opening")
                << ": " << pofile << std::endl
                ;
        }
        else
        {
            dictpointer d = std::make_shared<dictionary>();
            auto pdp = std::make_pair(polang, std::move(d));
            auto inserted_item = m_dictionaries.insert(pdp);
            bool ok = inserted_item.second;
            if (ok)
            {
                auto iter = inserted_item.first;
                auto dp = iter->second;
                std::string name = polang.get_language();
                bool ok = poparser::parse_po_file(pofile, *in, *dp);
                if (ok)
                {
                    std::string ncname = dirname;
                    if (get_bindings().set_binding_values(name, ncname))
                        result = dp;
                }
            }
        }
    }
    catch (const std::exception & e)
    {
        logstream::error()
            << _("error") << ": " << _("failure parsing") << ": "
            << pofile << "\n" << e.what() << "" << std::endl
            ;
    }
    return result;
}

/**
 *  This function is meant to read in all .po files from a given directory
 *  and create a dictionary and a binding entry for each one. It also adds
 *  the directory using the add_directory() function.
 *
 *  -----------------------------------------------------------------
 *  TODO: create only the dictionary matching the default domain.
 *  -----------------------------------------------------------------
 *
 *  Once this function is called (generally for only one locale directory),
 *  the get_dictionary() language overload should not need to do any
 *  dictionary insertions.
 *
 *  Open a directory and scan if for .po files.  Check if fname matches
 *  requested lang; ignore anything that isn't a .po file.
 *
 * \param dirname
 *      Provides the locale directory to be opened and processed.
 *
 * \param defaultdomain
 *      Indicates the desired domain (language name) to use to select
 *      the "current" dictionary. If empty, the first dictionary
 *      created is "current".
 *
 * \return
 *      Returns true if
 */

bool
dictionarymgr::add_dictionaries
(
    const std::string & dirname,
    const std::string & defaultdomain
)
{
    phraselist files = m_filesystem->open_directory(dirname);   /* vector   */
    bool result = files.size() > 0;
    if (! result)
        return false;

    bool logged_current_dict = false;
    for (const auto & fname : files)
    {
        if (has_suffix(fname, ".po"))
        {
            language polang = language::from_env(filename_to_language(fname));
            if (polang)
            {
                std::string pofile = dirname;
                pofile += "/";
                pofile += fname;
                dictpointer d = make_dictionary(polang, pofile, dirname);
                result = bool(d);
                if (result)
                {
                    if (is_nullptr(m_current_dict))
                    {
                        std::string name = polang.get_language();
                        if (defaultdomain.empty())
                        {
                            m_current_dict = d.get();
                            logged_current_dict = true;
                        }
                        else if (name == defaultdomain)
                        {
                            m_current_dict = d.get();
                            logged_current_dict = true;
                        }
                    }
                }
                else
                    break;
            }
            else
            {
                logstream::warning()
                    << fname << ": "
                    << _("ignoring unknown language") << std::endl
                    ;
            }
        }
    }
    if (result && ! logged_current_dict)
    {
        result = false;
        logstream::error()
            << _("Could not log dictionary for requested domain")
            << " " << defaultdomain << std::endl
            ;
    }
    return result;
}

/**
 *  This is a reimplementation of GNU's ::textdomain() for C++. That function
 *  allows changing/querying the current global domain for the LC_MESSAGE
 *  category. (Note that projects generally store their message catalog(s)
 *  in a directory called "LC_MESSAGES": "<localedir>/locale/LC_MESSAGES/").
 *
 *  This implementation does not support the thread locking of the original;
 *  it is meant to be called near the beginning of main(). It doesn't count
 *  message catalogs either.
 *
 * TODO:
 *
 *  Should not this function cause the domain's dictionary to be
 *  made the current dictionary?????????
 *
 *  From GNU's textdomain.c, when the new domain name matches the old domain:
 *  "This can happen and people will use it to signal that some environment
 *  variable changed."
 *
 *  TODO: "wide" (w) versions for Windows. Also see dia_get_lib_directory() in
 *        the Dia project: LIBDIR / "locale" and xpccut_text_domain().
 *
 * \param domainname
 *      Provides the base name of a message catalog, such as "en_US". It
 *      cannot be empty here. It must consist of characters legal in
 *      filenames. We support a value of "-" to reset the domain to
 *      "messages". If empty, the current domain is returned.
 *      The gettext manual section 11.2.1 says the domain name is normally
 *      the package name.
 *
 * \return
 *      Return the current domain name.
 */

std::string
dictionarymgr::textdomain (const std::string & domainname)
{
    if (! domainname.empty())
    {
        if (domainname == previous_domain())    /* could signal env change  */
        {
            m_current_domain = m_previous_domain;
        }
        else if (domainname != current_domain())
        {
            m_previous_domain = m_current_domain;
            if (domainname == "-")
                m_current_domain = "messages";
            else
                m_current_domain = domainname;
        }
    }
    return current_domain();
}

/**
 *  This is a reimplementation of GNU's ::bindtextdomain() for C++.
 *  That function allows one to change the file location of a message domain,
 *  useful when two domains have the same name (as in domain "es", for
 *  example).
 *
 *  Specify that the domainname message catalog will be found
 *  in dirname rather than in the system locale data base.
 *  Resolves the UNIXROOT enviroment variable (if it exists) into dirname
 *  if it is not resolved by os2compat.[ch].
 *
 *  Unlike the GNU version, we don't support __EMX__ and __KLIBC__.
 *
 *  TODO: If dirname is empty, but domainname is not, return the current
 *  domain. If both are null, return an empty string.
 *
 * /param domainname
 *      The name of the domain, either a package name or a language name.
 *
 * /param dirname
 *      The location of the package. Should be an absolute path, but
 *      we also support relative paths (for testing).
 */

std::string
dictionarymgr::bindtextdomain
(
    const std::string & domainname,
    const std::string & dirname
)
{
    std::string result;
    std::string saved_dirname = dirname;
    if (dirname[0] == '/' || dirname[0] == '\\')
    {
        const char * ur = std::getenv("UNIXROOT");
        if (not_nullptr(ur))
        {
            std::size_t len = dirname.length() + 3;
            if (len <= p_max_path)
            {
                bool isdrivename = ur[0] != 0 && ur[1] == ':' && ur[2] == 0;
                if (isdrivename)
                {
                    std::string dir_with_drive = ur;
                    dir_with_drive += dirname;
                    saved_dirname = dir_with_drive;
                }
            }
        }
    }
    if (get_bindings().set_binding_values(domainname, saved_dirname))
        result = saved_dirname;
    else
        result = dirname;

    return result;
}

std::string
dictionarymgr::bind_textdomain_codeset
(
    const std::string & domainname,
    const std::string & codeset
)
{
    std::string result = codeset;
    if (get_bindings().set_binding_codeset(domainname, result))
        return result;
    else
        return std::string("");
}

}           // namespace po

/*
 * dictionarymgr.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
