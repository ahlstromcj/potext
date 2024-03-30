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
 * \file          gettext.cpp
 *
 *  Macros that depend upon the build platform.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-30
 * \license       See above.
 *
 *  https://www.gnu.org/software/gettext/manual/ provides a 300-page manual in
 *  various formats. It includes additional advice for Windows, installing
 *  translations, the format of .po files, running xgettext, and much more.
 *
 * Locale (command-line, .login, or .profile):
 *
 *  Format:
 *
 *      ll_CC[.encoding]
 *      ll_CC[@variant]
 *
 *          ll = language code
 *          CC = country code
 *          encoding = type of character encoding, such as "UTF-8".
 *          variant = extra characteristics like "euro", "latin"
 *
 *      See the command "locale -a". Also note the special locale, "C",
 *      for POSIX messages.
 *
 *  Environment variables:
 *
 *      LANGUAGE, LC_ALL, LC_CTYPE, LC_NUMERIC, LC_TIME, LC_COLLATE,
 *      LC_MONETARY, LC_MESSAGES, ..., LANG
 *
 *      GNU gettext() will act as a no-op if the LC_MESSAGES and LC_CTYPE
 *      locale categories are not both set.
 *
 *  *.po Locations:
 *
 *      -   /usr/share/<appname>/po
 *      -   /usr/local/share/<appname>/po
 *      -   /home/user/.config/<appname>/po
 *
 * Basic process:
 *
 *      -   Set up the language environment.
 *          -   System-wide default language locale configuration.
 *              -   /etc/profile, etc., set LANG, but not LC_ALL
 *              -   LANGUAGE, a colon-separated list of languages.
 *              -   On our Arch Linux, LANGUAGE, LANG, and LC_ALL are all
 *                  set to "en_US.UTF-8".
 *              -   Windows: also "set OUTPUT_CHARSET=CP850".
 *          -   User's setup. LANG, LC_ALL
 *              -   setenv LANG de_DE (csh)
 *              -   export LANG; LANG=de_DE (sh)
 *              -   export LANG=de_DE (bash)
 *              -   LANG-de_DE programname (bash, precedes program invocation)
 *              -   For overriding locale without affecting LANG settings,
 *                  define TEXTDOMAIN and TEXTDOMAINDIR. Examples:
 *                  -   TEXTDOMAIN=en_US
 *                  -   TEXTDOMAINDIR=/usr/share/locale (iffy for .po files)
 *                  -   TEXTDOMAINDIR=/home/user/.config/PACKAGE/locale
 *          -   GUI setup. For example, KDE_LANG
 *          -   C setup: LANG="C", LC_ALL="C"
 *      -   Install the *.po files
 *
 * Implemented:
 *
 *      See the developer's guide section on Gettext.
 */

#include <cerrno>                       /* errno                            */
#include <climits>                      /* PATH_MAX                         */
#include <clocale>                      /* std::setlocale()                 */
#include <codecvt>                      /* std::codecvt()                   */
#include <cstdlib>                      /* std::getenv()                    */
#include <cstring>                      /* std::strerror()                  */
#include <locale>                       /* std::wstring_convert<>           */
#include <map>                          /* std::map<>                       */

#include "c_macros.h"                   /* not_nullptr(), etc.              */
#include "po/dictionarymgr.hpp"         /* po::dictionary, mgr, etc.        */
#include "po/gettext.hpp"               /* external gettext-related funcs   */
#include "po/logstream.hpp"             /* po::logstream::info(), etc.      */
#include "po/nlsbindings.hpp"           /* po::nlsbindings class            */
#include "po/wstrfunctions.hpp"         /* po::wide-to-narrow functions     */

/**
 *  Since we're using C++17, we could use std::filesystem::path and it's
 *  current_path() function, but we have to learn a bit about that first.
 */

#if defined PLATFORM_WINDOWS            /* Microsoft platform               */
#include <cctype>                       /* std::isalpha()                   */
#include <dir.h>                        /* file-name info and getcwd()      */
#define p_max_path      _MAX_PATH       /* Microsoft's path size (260!)     */
#define PATH_SLASH      "\\"
#define PATH_SLASH_CHAR '\\'
#define ENV_HOMEDRIVE   "HOMEDRIVE"
#define ENV_HOMEPATH    "HOMEPATH"
#define ENV_CONFIG      "LOCALAPPDATA"
#else
#include <unistd.h>
#define p_max_path      PATH_MAX        /* POSIX path size                  */
#define PATH_SLASH      "/"
#define PATH_SLASH_CHAR '/'
#define ENV_HOME        "HOME"
#define ENV_CONFIG      ".config"
#endif

namespace po
{

#if defined POTEXT_BUILD_CATEGORY_SUPPORT

/**
 *  Values copped from /usr/include/bit/locale.h.
 */

enum class LC
{
    CTYPE           = 0,                /* all categories > zero            */
    NUMERIC,                            /* numbers, numeric punctuation     */
    TIME,                               /* date and time                    */
    COLLATE,                            /* what is this?                    */
    MONETARY,                           /* money, it's a hit!               */
    MESSAGES,                           /* the most common case             */
    ALL,                                /* considered an invalid category   */
    PAPER,                              /* what is this?                    */
    NAME,                               /* proper nouns, names              */
    ADDRESS,                            /* street addresses?                */
    TELEPHONE,                          /* telephone numbers                */
    MEASUREMENT,                        /* measurements, what aspects?      */
    IDENTIFICATION,                     /* what is this?                    */
    LAST                                /* the invalid end of the list      */
};

/**
 *  Tests the category value against the LC_xxx values. If the test fails,
 *  then the "Germanic plural rule is used.
 */

bool
category_is_valid (int c)
{
    return
    (
        c >= 0 &&
        c < static_cast<int>(LC::LAST) &&
        c != static_cast<int>(LC::ALL)
    );
}

/**
 *  Quick lookup of the category name. Note that LC_MESSAGES is by far the
 *  most common category (at least on our Linux laptops).
 */

const std::string &
category_to_name (LC lc)
{
    using lc_pairs = std::map<LC, std::string>;

    static std::string s_invalid;
    static lc_pairs s_category_pairs
    {
        { LC::CTYPE,          "LC_CTYPE"                },
        { LC::NUMERIC,        "LC_NUMERIC"              },
        { LC::TIME,           "LC_TIME"                 },
        { LC::COLLATE,        "LC_COLLATE"              },
        { LC::MONETARY,       "LC_MONETARY"             },
        { LC::MESSAGES,       "LC_MESSAGES"             },
        { LC::ALL,            "LC_ALL"                  },
        { LC::PAPER,          "LC_PAPER"                },
        { LC::NAME,           "LC_NAME"                 },
        { LC::ADDRESS,        "LC_ADDRESS"              },
        { LC::TELEPHONE,      "LC_TELEPHONE"            },
        { LC::MEASUREMENT,    "LC_MEASUREMENT"          },
        { LC::IDENTIFICATION, "LC_IDENTIFICATION"       },
    };
    const auto lcit = s_category_pairs.find(lc);
    return lcit != s_category_pairs.end() ? lcit->second : s_invalid ;
}

#endif  // defined POTEXT_BUILD_CATEGORY_SUPPORT

/*
 *  Static function declarations.
 *
 *  These are being modified to use the dictionarymgr as much as possible.
 */

static dictionarymgr & dictionary_manager (const std::string & chset = "");
static dir_type directory_type (dir_type dt = dir_type::none);
static dir_type analyze_directory_type (const std::string & dirname);
static std::string installed_prefix (const std::string & arg0);
static std::string installed_po_path
(
    const std::string & arg0,
    const std::string & pkgname
);
static dictionary & main_dictionary ();
static std::string user_config_po (const std::string & appfolder);
static bool set_locale_info
(
    const std::string & arg0,
    const std::string & pkgname,
    std::string & domainname,
    std::string & dirname
);

#if defined USE_INIT_GETTEXT
static nlsbindings & bindings ();
static const nlsbindings & const_bindings ();
static bool init_gettext ();
static std::string current_domain (const std::string & cd = "");
static std::string old_domain (const std::string & od = "");
#endif

static bool s_use_mo_mode = false;

static void set_use_mo_mode ()
{
    s_use_mo_mode = true;
}

static bool use_mo_mode ()
{
    return s_use_mo_mode;
}

/**
 *  Indicates which locale directory processing is in force.
 */

static dir_type
directory_type (dir_type dt)
{
    static dir_type s_dir_type = dir_type::none;
    if (dt != dir_type::none)
        s_dir_type = dt;

    return s_dir_type;
}

/**
 *  Analyzes a directory name and sets the dir_type:
 *
 *      -   none.
 *      -   system. The path starts with "x:\" or "/".
 *      -   user.  The path contains "AppData" or ".config".
 *      -   freeform.
 *
 * \param dirname
 *      Provides the directory name to analyze.
 *
 *  \return
 *      Returns one of the values listed above, "none" if unable to be
 *      analyzed.
 */

static dir_type
analyze_directory_type (const std::string & dirname)
{
    dir_type result = dir_type::none;
    if (! dirname.empty())
    {
#if defined PLATFORM_WINDOWS
        std::string::size_type pos = dirname.find("AppData");
#else
        std::string::size_type pos = dirname.find(".config");
#endif
        if (pos != std::string::npos)
        {
            result = dir_type::none;
        }
        else
        {
#if defined PLATFORM_WINDOWS
            bool issystem = std::isalpha(dirname[0]) &&
                dirname[1] == ':' &&
                dirname[2] == PATH_SLASH_CHAR
                ;
            if (issystem)
            {
                result = dir_type::system;
            }
#else
            if (dirname[0] == PATH_SLASH_CHAR)
            {
                pos = dirname.find("/usr");
                if (pos == 0)
                    result = dir_type::system;
            }
#endif
            else
            {
                result = dir_type::freeform;
            }
        }
    }
    return result;
}

#if defined USE_INIT_GETTEXT

/**
 *  Provides the binding list needed for textdomain() and friends, plus a
 *  function to access it.
 */

static nlsbindings &
bindings ()
{
    dictionarymgr & dm = dictionary_manager();
    return dm.get_bindings();
}

static const nlsbindings &
const_bindings ()
{
    dictionarymgr & dm = dictionary_manager();
    return dm.get_bindings();
}

#endif

/**
 *  Figures out the install prefix ($prefix or $winprefix) based on how the
 *  command was run.
 *
 * \param arg0
 *      Provides what is assumed to be the path to the application's
 *      location taken from argv[0].
 *
 * \return
 *      Returns the main location of the application.
 */

static std::string
installed_prefix (const std::string & arg0)
{
    std::string result;
#if defined PLATFORM_WINDOWS
    std::string search1{"C:\\Program Files (x86)\\"};
    std::string search2{"C:\\Program Files\\"};
    search1[0] = search2[0] = arg0[0];              /* actual drive letter  */
#else
    std::string search1{"/usr/local/"};
    std::string search2{"/usr/"};
#endif

    std::string::size_type pos = arg0.find(search1);
    if (pos != std::string::npos && pos == 0)
    {
        result = search1;
    }
    else
    {
        std::string::size_type pos = arg0.find(search2);
        if (pos != std::string::npos && pos == 0)
            result = search2;
    }
    return result;
}

/**
 * Potext *.po (Dictionary) Locations:
 *
 *      While the user/programmer can put the .po files anywhere, here are
 *      the conventional locations potext supports, to keep them separate from
 *      the .mo files and keep it simple:
 *
 *      The "system" is for a system-wide install, the "user" for a
 *      user-only install.
 *
 *  -   System:
 *
 *      -   Windows
 *          -   $winprefix/<appname>/data/po (e.g. C:/Program Files/seq66/data/po
 *      -   Linux
 *          -   $prefix/<appname>/po (e.g. /usr/local/share/seq66-0.99/po)
 *
 *      Note the slight inconsistency between Windows and Linux. A historical
 *      mistake.  Note that we still need to survey FreeBSD.
 *
 *  -   User:
 *          -   $winhome/po (e.g C:/Users/name/AppData/Local/seq66/po)
 *          -   $home/po (e.g. /home/user/.config/<appname>/po)
 *
 *  -   Freeform:
 *
 *      -   Any directory specified by the user during the program run.
 *
 *  This function handles only the "system" cases.
 *
 * \param arg0
 *      Provides the name by which the program was called. This information
 *      is needed to determine where installed locale information would be
 *      stored.
 *
 * \param pkgname
 *      The name of the package (library or application) which is incorporated
 *      into the directory name if that is not supplied. It cannot be
 *      obtained from arg0 because the program name and package name might
 *      differ slightly.
 *
 *  \return
 *      Returns the path to the .po files. A slash is appended for easy
 *      subsequent concatenation.
 */

static std::string
installed_po_path
(
    const std::string & arg0,
    const std::string & pkgname
)
{
    std::string prefix = installed_prefix(arg0);    /* ends with separator  */
#if defined PLATFORM_WINDOWS
    prefix += pkgname;
    prefix += PATH_SLASH;
    prefix += "data\\po\\";
#else
    prefix += "share";
    prefix += PATH_SLASH;
    prefix += pkgname;
    prefix += PATH_SLASH;
    prefix += "po/";
#endif
    return prefix;
}

/**
 *  Provides a single dictionary for basic and simple usage of po::gettext().
 *  Uses the default character set, UTF-8.
 */

static dictionarymgr &
dictionary_manager (const std::string & chset)
{
    static dictionarymgr s_dictionarymgr;
    if (! chset.empty())
        s_dictionarymgr.set_charset(chset);

    return s_dictionarymgr;
}

static dictionary &
main_dictionary ()
{
    return dictionary_manager().get_dictionary();
}

/**
 *  Gets the user's $HOME (Linux) or $LOCALAPPDAT (Windows) directory from the
 *  current environment.
 *
 * getenv("HOME"):
 *
 *      -   Linux returns "/home/ahlstrom".  Append "/.config/appname/po".
 *      -   Windows returns "\Users\ahlstrom".  A better value than HOMEPATH
 *          is LOCALAPPDATA, which gives us most of what we want:
 *          "C:\Users\ahlstrom\AppData\Local", and then we append simply
 *          "seq66".  However, this inconsistency is annoying. So now
 *          we provide separate functions for home versus the standard
 *          configuration directory for a Windows or Linux user.
 *
 * \param appfolder
 *      If not empty (the default) then the parameter is appended to the
 *      path that is returned.
 *
 * \return
 *      Returns the values:
 *
 *      -   "/home/user/.config/app/po"
        -   "C:\...\AppData\Local\app\po"
 *
 *      Notice the lack of a terminating path-slash.  If std::getenv() fails,
 *      an empty string is returned.
 */

static std::string
user_config_po (const std::string & appfolder)
{
    std::string result;
#if defined PLATFORM_WINDOWS
    char * env = std::getenv(ENV_CONFIG);   /* C:\Users\user\AppData\Local  */
    if (not_nullptr(env))
    {
        result = std::string(env);
        result += PATH_SLASH;
    }
    else
        logstream::error()
            << "std::getenv() " << "failed" << ENV_CONFIG << std::endl;
#else
    char * env = std::getenv(ENV_HOME);
    if (not_nullptr(env))
    {
        result = std::string(env);          /* /home/user                   */
        result += PATH_SLASH;
        result += ".config";
        result += PATH_SLASH;
    }
    else
        logstream::error()
            << "std::getenv() " << "failed" << ENV_HOME << std::endl;
#endif
    if (! result.empty())
    {
        result += appfolder;
        result += PATH_SLASH;
        result += "po";
    }
    return result;
}

/**
 *  Detects if "LC_" is part of the locale directory name and turns on the
 *  "mo mode".
 *
 *  TODO: how to adjust installed_po-path().
 */

static bool
check_mo_mode (const std::string & dirname)
{
    bool result = false;
    std::size_t pos = dirname.find("LC_");
    if (pos != std::string::npos)
    {
        result = true;
    }
    return result;
}

/**
 *  Sets up and returns the domain and its locale directory.
 *
 * \param arg0
 *      Provides the name by which the program was called. This information
 *      is needed to determine where installed locale information would be
 *      stored, if the dirname parameter was not supplied.
 *
 * \param pkgname
 *      The name of the package (library or application) which is incorporated
 *      into the directory name if that is not supplied. It cannot be
 *      obtained from arg0 because the program name and package name might
 *      differ slightly.
 *
 * \param [inout] domainname
 *      The desired and determined domain name. If empty, then
 *      TEXTDOMAIN and LC_ALL are tried.
 *
 * \param [inout] dirname
 *      The desired and determined local directory name. If empty, then
 *      TEXTDOMAINDIR is tried. If still empty, then the "system" directory is
 *      set. If the value is "user", then the "user" (.config) directory is
 *      assembled.
 *
 *      TODO: if the value is "LC_MESSAGE" or even if it contains "LC" at the
 *      beginning, or just /usr/.../locale, then we assume that .mo files
 *      are to be used; these are never installed in the user's configuration
 *      directory.
 *
 * \return
 *      Returns true if both parameters were fine. If false, neither parameter
 *      is altered.
 */

static bool
set_locale_info
(
    const std::string & arg0,
    const std::string & pkgname,
    std::string & domainname,
    std::string & dirname
)
{
    bool result = false;
    std::string domname = domainname;
    if (domname.empty())
    {
        const char * d = std::getenv("TEXTDOMAIN");
        if (not_nullptr(d))
        {
            domname = std::string(d);
        }
        else
        {
            d = std::getenv("LC_ALL");
            if (not_nullptr(d))
            {
                domname = std::string(d);
            }
            else
            {
                d = std::getenv("LC_MESSAGES");
                if (not_nullptr(d))
                {
                    domname = std::string(d);
                }
                else
                {
                    d = std::getenv("LANG");
                    if (not_nullptr(d))
                        domname = std::string(d);
                }
            }
        }
    }
    if (! domname.empty())
    {
        std::string domdirname = dirname;
        if (domdirname.empty())
        {
            const char * d = std::getenv("TEXTDOMAINDIR");
            if (not_nullptr(d))
                domdirname = std::string(d);
            else                                /* produce system directory */
                domdirname = installed_po_path(arg0, pkgname);
        }
        else if (domdirname == "user")          /* assemble user .config    */
        {
            domdirname = user_config_po(pkgname);
        }
        else if (check_mo_mode(domdirname))
        {
            set_use_mo_mode();
        }
        if (! domname.empty() && ! domdirname.empty())
        {
            result = true;
            domainname = domname;
            dirname = domdirname;               /* TODO: check existence    */
            dir_type dt = analyze_directory_type(dirname);
            if (dt != dir_type::none)
                (void) directory_type(dt);
        }
    }
    return result;
}

/*
 *  Functions/macros to add, from the helloworld app:
 *
 *      pgettext()
 *      ngettext()
 *      npgettext()
 *
 *  The "d" functions below are not in tinygettext.
 */

std::string
gettext (const std::string & msgid)
{
    if (directory_type() == dir_type::none)
    {
        return msgid;
    }
    else
    {
        /*
         * dictionary_manager().get_dictionary();
         */

        const dictionary & dict = main_dictionary();
        return dict.translate(msgid);
    }
}

/**
 *  Implemented:
 *
 *      -   dgettext(). Get a message from a different domain.
 *      -   dngettext()
 *      -   dcngettext()
 *
 *  Unimplemented (currently):
 *
 *      -   dcgettext(). Get a message from a different domain and category.
 */

/**
 *  This function looks up a message in a different domain from the current
 *  one set by textdomain().
 *
 *  We need a dictionarymgr function that looks up a dictionary matching
 *  the domainname.
 */

std::string
dgettext
(
    const std::string & domainname,
    const std::string & msgid
)
{
    if (directory_type() == dir_type::none)
    {
        return msgid;
    }
    else
    {
        const dictionary & dict =
            dictionary_manager().get_dictionary(domainname);

        return dict.empty() ? msgid : dict.translate(msgid) ;
    }
}

/**
 *  We think we want to look in the LC category directory if it exists,
 *  otherwise use the default. The GNU gettext manual says that this might
 *  not be useful. So for now we treat it like dgettext().
 *
 *  What can we do to fix this?
 *
 *  It seems like the category is basically used to select a subdirectory
 *  of the locale directory. This would require loading additional directories
 *  ahead of time in the dictionarymgr.
 *
 *  But a looksee in /usr/lib/locale/C.utf8 shows a bunch of "LC_xxx" binary
 *  files of type "glibc locale file". And apart from LC_MESSAGES, there
 *  are not so many other locales in /usr/share/locale: LC_TIME is all.
 *  This might be a function of our development laptop (Arch Linux).
 *
 *  An example is coreutils.mo, which is a "GNU message catelog (little
 *  endian) ..." file.
 *
 *  Can we preload a bunch of bindings or dictionaries related to categories?
 *
 *  Anyway, in extras/code/dcigettext.cpp (a file we are slowly massaging),
 *  the path fragment is "categoryname/domainname.mo". Also, a
 *  known_translation_t structure is allocated and filled and inserted into
 *  a search tree.
 */

std::string
dcgettext
(
   const std::string & domain,
   const std::string & msgid,
   int /* category */
)
{
    return dgettext(domain, msgid);         /* MORE TO DO */
}

/**
 *  Returns the desired plural form.
 */

std::string
ngettext
(
   const std::string & msgid,
   const std::string & msgid2,
   unsigned long N
)
{
    if (directory_type() == dir_type::none)
    {
        return msgid;
    }
    else
    {
        const dictionary & dict = main_dictionary();
        return dict.translate_plural(msgid, msgid2, N);
    }
}

/**
 *  Returns the desired plural form based on a selected domain.
 *
 *  Uses the dictionary::translate_plural() function.
 */

std::string
dngettext
(
   const std::string & domainname,
   const std::string & msgid,
   const std::string & msgid2,
   unsigned long n
)
{
    if (directory_type() == dir_type::none)
    {
        return msgid;
    }
    else
    {
        const dictionary & dict =
            dictionary_manager().get_dictionary(domainname);

        return dict.empty() ? msgid : dict.translate_plural(msgid, msgid2, n);
    }
}

/**
 *  Returns the desired plural form based on a selected domain and a
 *  selected category. We still need to figure out the category parameter.
 */

std::string
dcngettext
(
   const std::string & domainname,
   const std::string & msgid,
   const std::string & msgid2,
   unsigned long n,
   int /* category */                   /* TODO */
)
{
    if (directory_type() == dir_type::none)
    {
        return msgid;
    }
    else
    {
        const dictionary & dict =
            dictionary_manager().get_dictionary(domainname);

        return dict.empty() ? msgid : dict.translate_plural(msgid, msgid2, n);
    }
}

/**
 *  Macros from GNU gettext.h; to implement them try to find a common function.
 *
 * const char * pgettext_expr (const char * msgctxt, const char * msgid)
 * const char * dpgettext_expr
 * (
 *      const char * domain_name, const char * msgctxt, const char * msgid
 * )
 * const char * dcpgettext_expr
 * (
 *      const char * domain_name, const char * msgctxt,
 *      const char * msgid, int category
 * );
 *
 */

/**
 *  Gets message text by context and message ID.
 */

std::string
pgettext
(
   const std::string & msgctxt,
   const std::string & msgid
)
{
    if (directory_type() == dir_type::none)
    {
        return msgid;
    }
    else
    {
        const dictionary & dict = main_dictionary();
        return dict.translate_ctxt(msgctxt, msgid);
    }
}

/**
 *  Gets message text by domain, context, and message ID.
 */

std::string
dpgettext
(
   const std::string & domainname,
   const std::string & msgctxt,
   const std::string & msgid
)
{
    if (directory_type() == dir_type::none)
    {
        return msgid;
    }
    else
    {
        const dictionary & dict =
            dictionary_manager().get_dictionary(domainname);

        return dict.empty() ? msgid : dict.translate_ctxt(msgctxt, msgid);
    }
}

/**
 *  Gets message text by domain, context, and message ID, for a given
 *  LC_x category.
 */

std::string
dcpgettext
(
   const std::string & domainname,
   const std::string & msgctxt,
   const std::string & msgid,
   int /* category */
)
{
    if (directory_type() == dir_type::none)
    {
        return msgid;
    }
    else
    {
        const dictionary & dict =
            dictionary_manager().get_dictionary(domainname);

        return dict.empty() ? msgid : dict.translate_ctxt(msgctxt, msgid);
    }
}

/**
 *  MORE TODO!!!!!
 *      npgettext()                         Dictionary::translate_ctxt_plural()
 *      dnpgettext()
 *      dcnpgettext()
 */

/**
 *  A message domain is a set of translatable msgid messages. Usually, every
 *  software package has its own message domain. The domain name is used to
 *  determine the message catalog where a translation is looked up; it  must
 *  be a non-empty string.
 *
 *  The current message domain is used by the gettext(), ngettext() functions,
 *  and by the dgettext(), dcgettext(), dngettext(), and dcngettext()
 *  functions  when called with a NULL domainname argument.
 *
 *  Note that the default domain "messages" comes from the gettext project
 *  macro MESSAGE_DOMAIN_DEFAULT.
 *
 *  One can have an array of domains; we do not support that concept here.
 */

#if defined USE_INIT_GETTEXT_ETC

static std::string
base_locale_directory (const std::string & bld = "")
{
    static std::string s_base_locale_directory;
    if (! bld.empty())
        s_base_locale_directory = bld;

    return s_base_locale_directory;
}

static std::string
old_domain (const std::string & od)
{
    static std::string s_old_domain;
    if (! od.empty())
        s_old_domain = od;

    return s_old_domain;
}

static std::string
current_domain (const std::string & cd)
{
    static std::string s_current_domain;
    if (! cd.empty())
        s_current_domain = cd;

    return s_current_domain;
}

/**
 *  Simply guarantees that all static values are initialized to defaults.
 *
 *  The current domain default should be messages, but we do not yet make
 *  sure all settings for it are made.
 */

static bool
init_gettext ()
{
    static bool s_uninitialized = true;
    if (s_uninitialized)
    {
        s_uninitialized = false;
        (void) directory_type();        /* sets it to dir_type::none        */
        current_domain("messages");     /* manual section 11.2.1            */
        (void) bindings();              /* initializes internal bindings    */
        (void) main_dictionary();       /* initializes internal dictionary  */
        (void) old_domain();
        (void) base_locale_directory();
    }
    return true;
}

#endif

#if defined POTEXT_ENABLE_I18N

/**
 *  See dictionarymgr::textdomain().
 */

std::string
textdomain (const std::string & domainname)
{
    return dictionary_manager().textdomain(domainname);
}

/**
 *  See dictionarymgr::bindtextdomain().
 */

std::string
bindtextdomain
(
    const std::string & domainname,
    const std::string & dirname
)
{
    return dictionary_manager().bindtextdomain(domainname, dirname);
}

std::string
bind_textdomain_codeset
(
    const std::string & domainname,
    const std::string & codeset
)
{
    return dictionary_manager().bind_textdomain_codeset(domainname, codeset);
}

#if 0
/**
 *  See dictionarymgr::wbindtextdomain().
 */

std::wstring
wbindtextdomain
(
    const std::string & domainname,
    const std::wstring & wdirname
)
{
    return dictionary_manager().wbindtextdomain(domainname, wdirname);
}
#endif

/**
 *  bindtextdomain() sets the base directory of the hierarchy containing
 *  message catalogs for a given message domain.
 *
 *  The message catalog used by the functions is expected to be at the pathname
 *  "dirname/locale/category/domainname.mo". Here 'dirname' is the directory
 *  specified through bindtextdomain(). Its default is system and configuration
 *  dependent; typically "prefix/share/locale", where 'prefix' is the
 *  installation prefix of the package.
 *
 *  The gettext manual say the location of a message catalog is
 *
 *      dirname/local/LC_categroy/domain_name.mo
 *
 *  and the LC_category for gettext() and dgettext() is always LC_MESSAGES.
 *  This can be changed via setlocale(). Some systems (e.g. mingw) don’t
 *  have LC_MESSAGES. We use a more or less arbitrary value for it, 1729,
 *  the smallest positive integer which can be represented in two different
 *  ways as the sum of two cubes.  When the system does not support
 *  setlocale() its behavior in setting locale is simulated by using the
 *  environment variables.
 *
 *  The default value of dirname for the GNU library is
 *
 *      /usr/local/share/locale
 *
 *  On Arch Linux, LC_MESSAGES is found (for example) in /usr/share/local/da
 *  and it contains *.mo files for installed application and *.qm files for
 *  Qt-related applications.
 *
 *  For the Vim editor, the location (on Arch) is
 *
 *      /usr/share/vim/vim91/lang/da
 *
 *  For CUPS, the location (on Arch) is
 *
 *      /usr/share/locale/da
 *
 *  and it contains only *.po files.
 *
 *  The library code doesn’t call setlocale(LC_ALL, ""); the main program sets
 *  the locale. The library code doesn’t call textdomain(PACKAGE); it would
 *  interfere with the text domain set by the main program.
 *
 * \param domainname
 *      Provides the base name of a message catalog, such as "en_US".
 *      It cannot be empty. It must consist of characters legal in filenames.
 *
 * \param dirname
 *      Provides the name of the LOCALDIR to which the domain is to be bound.
 *      For consistency, this directory should be an absolute path, not a
 *      relative one. If empty, then the next parameter should be supplied.
 *
 * \param wdirname
 *      The same as \a dirname, but for use with wide strings.
 *
 *  Also consider bind_textdomain_codeset(domainname, codeset = "UTF-8").
 *
 * \return
 *      If this function fails, an empty string is returned.
 */

std::string
init_lib_locale
(
    const std::string & domainname,     /* the desired domain name          */
    const std::string & dirname,        /* i.e. the locale directory        */
    const std::wstring & wdirname       /* optional wide-string format      */
)
{
    std::string result;
    bool ok = ! domainname.empty();
    if (ok)
    {
        bool use_wstring = ! wdirname.empty();
        std::string domdirname = dirname;
        if (use_wstring)
            domdirname = wstring_to_utf8(wdirname);

        if (! domdirname.empty())
        {
            std::string bd;
            bool ok = dictionary_manager().add_dictionaries
            (
                domdirname, domainname
            );
            if (ok)
                bd = bindtextdomain(domainname, domdirname);

            if (! bd.empty())
            {
                result = std::string(bd);
                logstream::info()
                    << "bindtextdomain() --> " << result << std::endl;
            }
            else
                logstream::error()
                    << "bindtextdomain() " << "failed" << std::endl;
        }
    }
    return result;
}

/**
 *  setlocale() installs a system locale or its portion as the new C locale.
 *  The modifications influences the execution of all locale-sensitive C
 *  functions until the next call to setlocale(). If locale is null,
 *  setlocale() queries the current C locale without modifying it.
 *
 *  'category' is locale category identifier, one of the LC_xxx macros or 0.
 *  Also called a 'facet'.
 *
 *  'locale' is a system-specific locale identifier; "" for the user-preferred
 *  locale, "C" for the minimal locale, "POSIX" for a more full C locale, or
 *  something like "da_DK" (language[_territory][.codeset][@modifier]).  If
 *  null, the current locale is queried.
 *
 *  setlocale() returns a pointer to a narrow null-terminated string ID'ing
 *  the C locale after applying the changes, or null pointer on failure.  A
 *  copy of the returned string along with the category used in the call may
 *  be used later in the program to restore the locale back to the state at
 *  the end of this call.
 *
 *  During program startup, the equivalent of std::setlocale(LC_ALL, "C") is
 *  executed before any user code is run.
 *
 *  See setlocale(3) for still more details. Note that switching a locale is
 *  not multi-thread safe.
 *
 *  Also compare this code to main(), for the ngettext program, in
 *
 *      gettext/gettext-runtimes/src/ngettext.c
 *
 *  Try running "ngettext --help" on your system.
 *
 * \param arg0
 *      Provides the name by which the program was called. This information
 *      is needed to determine where installed locale information might be
 *      stored.
 *
 * \param pkgname
 *      Provides the name of the PACKAGE. For example, "hello" in the
 *      test helloworld program.
 *
 * \param domainname
 *      Provides the base name of a message catalog, such as "en_US".
 *      It cannot be empty here. It must consist of characters legal in
 *      filenames.  We might support a value of "0" to reset the domain to
 *      "messages". Also note that an application might want to use its own
 *      package name, such as "hello", for the domain name. If empty, then
 *      the environment variable TEXTDOMAIN is used.
 *
 * \param dirname
 *      Provides the name of the LOCALEDIR. The standard search directory is
 *      /user/share/locale. If empty, then the environment variable
 *      TEXTDOMAINDIR is used. These choices are made in set_locale_info().
 *      See that function's description.
 *
 * \param wdirname
 *      The same as \a dirname, but for use with wide strings.
 *
 * \param category
 *      The area that is covered, such as LC_ALL, LC_MONETARY, and LC_NUMERIC.
 *      The default, 0, selects LC_ALL.
 *
 * \return
 *      Returns the result of init_lib_locale() [bindtextdomain() wrapper].
 *      An empty string is returned on failure.
 */

std::string
init_app_locale
(
    const std::string & arg0,
    const std::string & pkgname,
    const std::string & domainname,
    const std::string & dirname,
    const std::wstring & wdirname,      /* optional wide-string format      */
    int category
)
{
    std::string result;
#if defined USE_INIT_GETTEXT
    (void) init_gettext();              /* guarantee the static defaults    */
#endif
    if (category == (-1))
        category = LC_ALL;              /* LC_CTYPE is 0, LC_ALL is 6       */

    char * lc = std::setlocale(category, "");
    bool ok = not_nullptr(lc);
    if (ok)
    {
        /*
         * TODO: handle wide strings.
         */

        std::string domname = domainname;
        std::string domdirname = dirname;
        ok = set_locale_info(arg0, pkgname, domname, domdirname);
        if (ok)
        {
            logstream::info() << "setlocale() --> " << lc << std::endl;
            result = init_lib_locale(domname, domdirname, wdirname);
            if (! result.empty())
            {
                std::string td = textdomain(domname);       /* not 'pkg'    */
                ok = ! td.empty();
                if (ok)
                    logstream::info() << "textdomain() --> " << td << std::endl;
            }
        }
    }
    return result;
}

#endif  // defined POTEXT_ENABLE_I18N

}               // namespace po

/*
 * gettext.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
