#if ! defined POTEXT_PO_GETTEXT_HPP
#define POTEXT_PO_GETTEXT_HPP

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
 * \file          gettext.hpp
 *
 *      Declares reimplementations of gettext functions in the po namespace.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-02-16
 * \updates       2024-04-01
 * \license       See above.
 *
 *  gettext_noop()  pseudo function call that serves as a marker for the
 *  automated extraction of messages, but does not call gettext().  The
 *  run-time translation is done at a different place in the code.
 *
 *  The argument, String, should be a literal string.  Concatenated strings
 *  and other string expressions won't work.
 *
 *  The macro's expansion is not parenthesized, so that it is suitable as
 *  initializer for static 'char[]' or 'const char[]' variables.
 *
 *  For now, just define this value and be sure to include po/gettext.hpp
 *  when extracti messages.
 *
 *  In main(), call the following functions [we will provide a wrapper
 *  function at some point]:
 *
 *      setlocale(LC_ALL, "");
 *
 *          - or -
 *
 *              setlocale(LC_CYTPE, "");
 *              setlocale(LC_MESSAGES, "");
 *              . . .
 *
 *      bindtextdomain(PACKAGE, LOCALEDIR);             // PACKAGE or LANG?
 *      textdomain(PACKAGE);
 *      bind_textdomain_codeset(PACKAGE, CODESET);      // OPTIONAL
 *
 *  CODESET could be something like "UTF-8".
 */

#include <string>                       /* std::string & std::wstring       */

#include "po_build_macros.h"            /* build (and platform) macros      */

#define PO_HAVE_GETTEXT                 /* a friendlier header marker       */

/**
 *  The usual declaration of the ‘_’ macro in each source file starts as
 *  "#define _(str) gettext(str)" for a program. For a library, with its own
 *  translation domain: "#define _(str) dgettext(PACKAGE, str)". That is,
 *  dgettext() is used instead of gettext(). Similarly, dngettext() should be
 *  used in place of ngettext().
 */

#if defined POTEXT_ENABLE_I18N          /* this is normally defined         */

#undef POTEXT_ENABLE_LIBINTL

#define _(str)                          po::gettext (str)
#define gettext_noop(str)               str
#define N_(str)                         gettext_noop (str)

#else           // ! defined POTEXT_ENABLE_I18N

#if defined POTEXT_ENABLE_LIBINTL       /* see po_build_macros.h            */

#define _(str)                          gettext (str)
#define gettext_noop(str)               str
#define N_(str)                         gettext_noop(str)

#else           // ! defined POTEXT_ENABLE_LIBINTL

#define _(str)                          (str)
#define N_(str)                         str
#define textdomain(domain)
#define bindtextdomain(pkg, dir)

#endif          // defined POTEXT_ENABLE_LIBINTL

#endif          // defined POTEXT_ENABLE_I18N

namespace po
{

/**
 *  Indicates how the user, package, or application wants to select the locales.
 *
 * \var none
 *      Not sure yet.
 *
 * \var system
 *      The default location to seach is based on the installation $prefix
 *      and the package/library/application name:
 *
 *          $prefix/<appname>/po (e.g. /usr/local/share/seq66-0.99/po)
 *
 * \var user
 *      The default location to seach is based on the user's .config directory
 *      and the package/library/application name:
 *
 *          $home/po (e.g. /home/user/.config/<appname>/po)
 *
 * \var freeform
 *      The directory is specified at run-time and can be any directory,
 *      including a relative directory.
 *
 */

enum class dir_type
{
    none,
    system,
    user,
    freeform
};

/**
 *  Free functions in the po namespace.
 */

extern std::string gettext (const std::string & msgid);
extern std::string dgettext
(
    const std::string & domainname,
    const std::string & msgid
);
extern std::string dcgettext
(
    const std::string & domain,
    const std::string & msgid,
    int category = (-1)
);
extern std::string ngettext
(
   const std::string & msgid1,
   const std::string & msgid2,
   unsigned long n
);
extern std::string dngettext
(
   const std::string & domain,
   const std::string & msgid1,
   const std::string & msgid2,
   unsigned long n
);
extern std::string dcngettext
(
   const std::string & domain,
   const std::string & msgid1,
   const std::string & msgid2,
   unsigned long n,
   int category = (-1)
);
extern std::string pgettext
(
   const std::string & msgctxt,
   const std::string & msgid1
);
extern std::string dpgettext
(
   const std::string & domain,
   const std::string & msgctxt,
   const std::string & msgid1
);
extern std::string dcpgettext
(
   const std::string & domain,
   const std::string & msgctxt,
   const std::string & msgid1,
   int category
);
extern std::string init_lib_locale
(
    const std::string & domainname,
    const std::string & dirname,
    const std::wstring & wdirname = L""
);
extern std::string init_app_locale
(
    const std::string & arg0,
    const std::string & pkg,
    const std::string & domainname,
    const std::string & dirname,
    const std::wstring & wdirname = L"",
    int category = (-1)
);

#if defined POTEXT_ENABLE_I18N

extern std::string textdomain (const std::string & domainname);
extern std::string bindtextdomain
(
    const std::string & domainname,
    const std::string & dirname
);
extern std::string bind_textdomain_codeset
(
    const std::string & domain,
    const std::string & codeset
);

#if defined POTEXT_WIDE_STRING_SUPPORT
extern std::wstring wbindtextdomain
(
    const std::string & domainname,
    const std::wstring & wdirname
);
#endif

#endif

}               // namespace po

#endif          // POTEXT_PO_GETTEXT_HPP

/*
 * gettext.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

