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
 * \file          helloworld.cpp
 *
 *      This test program uses GNU gettext itself, along with associated
 *      macroes found in the copy of gettext.h in the potext/library/include
 *      directory.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-18
 * \license       See above.
 *
 * gettext.h:
 *
 *      https://www.gnu.org/software/gettext/manual/html_node/Contexts.html
 *
 *      Contexts can be added to strings to be translated. A context dependent
 *      translation lookup is when a translation for a given string is
 *      searched, that is limited to a given context. The translation for the
 *      same string in a different context can be different. The different
 *      translations of the same string in different contexts can be stored in
 *      the in the same MO file, and can be edited by the translator in the
 *      same PO file.
 *
 *      The gettext.h include file contains the lookup macros for strings with
 *      contexts. They are implemented as thin macros and inline functions
 *      over the functions from <libintl.h> (which gettext.h includes).
 *
 *   We store a copy from ardour in the library/include directory. Some of the
 *   macros defined:
 *
 *      const char * pgettext (const char * msgctxt, const char * msgid);
 *      const char * npgettext
 *      (
 *          const char * msgctxt, const char * msgid,
 *          const char * msgidplural, unsigned long int N
 *      );
 *
 * Additional functions in <libintl.h> (see "man 3 npgettext"):
 *
 *      char * ngettext
 *      (
 *          const char * msgid, const char * msgidplural,
 *          unsigned long int N
 *      );
 *
 *      The parameter n selects the appropriate plural form.
 *      We use N = 1 unless otherwise provided below.
 *
 * Usage:
 *
 *      https://www.labri.fr/perso/fleury/posts/programming/
 *              a-quick-gettext-tutorial.html
 *
 *      LANG=xx_XX build/library/tests/helloworld
 *
 *  This test essentially ignores GNU gettext.
 */

#include <clocale>                      /* std::setlocale()                 */
#include <cstdlib>                      /* EXIT_SUCCESS, EXIT_FAILURE       */
#include <cstring>                      /* std::strcmp()                    */
#include <iostream>                     /* std::cout                        */

/*
 *  Note that gettext.h depends on GNU gettext. It defines some macros
 *  used below. We keep a copy in the extras/code directory.
 */

#include "platform_macros.h"            /* PLATFORM_UNIX macro              */
#include "../../extras/code/gettext.h"  /* pgettext(), gettext(3), etc.     */

#define _(x)    gettext(x)              /* GNU version of gettext()         */

/**
 *  Process:
 *
 *      -   Set the i18n environment.
 *          -   Set the locale.
 *          -   Set the directory containing the message catalogs for the
 *              given message domain.
 *          -   Set the current message domain.
 */

int
main (int argc, char * argv [])
{
    int result = EXIT_SUCCESS;
    if (argc > 1)
    {
        if (std::strcmp(argv[1], "--help") == 0)
        {
            std::cout
                << "Currently no options are available; to do."
                << std::endl
            ;
        }
        else
            std::cout << "No options required at this time" << std::endl;
    }
    else
    {
        /*
         * In a program using potext instead of gettext, the next three lines
         * of code could be replaced by a call to
         *
         *      po::init_app_locale
         *      (
         *          "helloworld", "hello", "library/tests/helloworld"
         *      );
         *
         * We will make an application to test that Real Soon Now.
         */

        std::setlocale(LC_ALL, "");

#if defined PLATFORM_UNIX
        bindtextdomain("hello", "/usr/share/locale");
        (void) textdomain("hello");
#endif

        std::cout
            << _("Short Hello World") << "\n"
            << _("Hello World") << "\n"
            << _("Hello Worlds") << "\n"
            << pgettext("", "Hello World") << "\n"
            << pgettext("console", "Hello World") << "\n"
            << ngettext("gui", "Hello World", 1) << "\n"
            << ngettext("Hello World", "Hello Worlds", 5) << "\n"
            << npgettext("", "Hello World", "Hello Worlds", 5) << "\n"
            << npgettext("console", "Hello World", "Hello Worlds", 5) << "\n"
            << npgettext("gui", "Hello World", "Hello Worlds", 5)
            << std::endl
            ;
        std::cout
            << "This test simply prints, and it always succeeds. Needs work!"
            << std::endl
            ;
    }
    return result;
}

/*
 * helloworld.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
