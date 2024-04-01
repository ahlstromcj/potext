#if ! defined POTEXT_PO_BUILD_MACROS_H
#define POTEXT_PO_BUILD_MACROS_H

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
 *  If you do not like the changes or the GPL licensing, use the original
 *  tinygettext project, available at GitHub:
 *
 *      https://github.com/tinygettext/tinygettext
 */

/**
 * \file          po_build_macros.h
 *
 *      Macros that depend upon the decisions select to build the
 *      library.
 *
 * \library       potext
 * \author        Gary P. Scavone; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-04-01
 * \license       See above.
 *
 * Introduction:
 *
 *      We have two ways of configuring the build for the potext library.
 *      This file presents features that depend upon the build platform, and
 *      build options that might be considered permanent.
 *
 */

#include "platform_macros.h"            /* generic detecting of OS platform */

/**
 *  Tentative. We need to have this set via a Meson option. These definitions
 *  are currently mutually exclusive.
 *
 *      POTEXT_ENABLE_I18N:
 *
 *          Defines the _() and N_() macros to call gettext() and
 *          gettext_noop() when running an internationalized application.
 *
 *      POTEXT_ENABLE_LIBINTL:
 *
 *          Causes libintl.h to be included and the macros for xgettext
 *          processing to be defined. Without this flag, the _() and N_()
 *          calls simply return their argument, and textdomain() and
 *          bindtextdomain() are do-nothing macros. See gettext manual
 *          section 1.5.
 */

#define POTEXT_ENABLE_I18N
#undef  POTEXT_ENABLE_LIBINTL

/**
 *  For the "po" text source code, enable/disable the addition of all
 *  languages that can be support. Undefine for testing,  to
 *  support the most prevalent languages using the ABC alphabet.
 */

#define POTEXT_BUILD_ALL_LANGUAGES

/**
 *  If true, we can use the enum class LC to handle categories. This
 *  looks to be a bit difficult at the moment, so it is disabled.
 */

#undef POTEXT_BUILD_CATEGORY_SUPPORT

/**
 *  If true, support handling wide-string directory names, etc.  If false, all
 *  functions with std::wstring parameters are commented out. If true, in most
 *  cases we try to use std::codecvt etc. We wonder how much extra code this
 *  brings in.
 */

#define POTEXT_WIDE_STRING_SUPPORT

/**
 *  We do not need to use operator = () to set up some static
 *  arrays. We'll keep this around just in case.
 */

#undef POTEXT_USE_BRUTE_FORCE_INITIALIZATION

/**
 *  Use the SDL library?  What is dat? We'll leave it undefined.
 */

#undef POTEXT_PO_WITH_SDL

/**
 *  To do: improve the naming.
 */

#if defined POTEXT_EXPORT

#if defined PLATFORM_WINDOWS || defined PLATFORM_CYGWIN

#define POTEXT_DLL_PUBLIC   __declspec(dllexport)
#define POTEXT_API          __declspec(dllexport)

#else

#if PLATFORM_GNUC >= 4
#define POTEXT_DLL_PUBLIC   __attribute__((visibility("default")))
#define POTEXT_API          __attribute__((visibility("default")))
#else
#define POTEXT_API
#define POTEXT_DLL_PUBLIC
#endif

#endif

#else                       /* not exporting    */

#define POTEXT_API
#define POTEXT_DLL_PUBLIC

#endif

#endif          // POTEXT_PO_BUILD_MACROS_H

/*
 * po_build_macros.h
 *
 * vim: sw=4 ts=4 wm=4 et ft=c
 */

