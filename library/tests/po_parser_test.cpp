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
 * \file          po_parser_test.cpp
 *
 *      Simple test of parsing .po files.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-04-09
 * \license       See above.
 *
 *  The feaures of this test:
 *
 *      -   po::init_app_locale() is not called.
 *      -   po::poparser::parse_po_file() parses each file, and the result
 *          is evaluated.
 *
 */

#include <cerrno>                       /* errno                            */
#include <cstdlib>                      /* EXIT_SUCCESS, EXIT_FAILURE       */
#include <cstring>                      /* std::strerror()                  */
#include <string>                       /* std::string                      */
#include <fstream>                      /* std::ifstream                    */
#include <iostream>                     /* std:cout and cerr                */
#include <stdexcept>                    /* std::runtime_error               */

#include "po/poparser.hpp"              /* po::poparser class               */
#include "po/potext.hpp"                /* Potext header files              */
#include "po/logstream.hpp"             /* po::logstream class              */

/*
 *  The usage of an anonymous namespace is essentially the same as using
 *  the "static" keyword here, except it can also hide type declarations.
 */

namespace                               /* anonymous namespace for statics  */
{

/**
 *  Our callback just adds some bling at the beginning of a message.
 */

void
my_log_callback (const std::string & err)
{
    std::cerr << "[poparser test] " << err;
}

/**
 *  Use with the "--all" option to run through all files. With no such
 *  option, the command-line arguments replace this list of files for the
 *  run.
 *
 *  Note that "library/tests/po/de.po" yields invalid multibyte characters
 *  on umlauts, while "po/de.po" does not. The "file" command shows.
 *
 *  library/tests/po/de.po: GNU gettext message catalogue, ISO-8859 text
 *  po/de.po: GNU gettext message catalogue, Unicode text, UTF-8 text
 *
 *  But the former has "charset=UTF-8". Changing it to "ISO-8859-1"
 *  removed the multibyte error, but the test still fails. INVESTIGATE.
 *
 */

po::phraselist s_all_files
{
    /*
     * GNU gettext message catalogue, ASCII text. It is deliberately
     * erroneous: no closing quote at line 19.
     */

    "library/tests/broken.po",              /* erroneous, no closing quote  */

    /*
     * GNU gettext message catalogue, ISO-8859 text.
     */

    "library/tests/de.po",

    /*
     * GNU gettext message catalog, ISO-8859.
     */

    "library/tests/game/de.po",

    /*
     * GNU gettext message catalogue, ASCII text. It has one stanza with
     * an empty msgctxt string. It has one with a msgid of "gui" and
     * a msgid_plural of "Hello World" (singular!).
     */

    "library/tests/helloworld/de.po",

    /*
     * GNU gettext message catalogue, ISO-8859 text, but specifies
     * "charset=CHARSET".
     */

    "library/tests/level/de.po",            /* erroneous, bad CHARSET       */

    /*
     * GNU gettext message catalogue, Unicode text, UTF-8 text.
     */

    "library/tests/po/de_AT.po",

    /*
     * GNU gettext message catalogue, ISO-8859 text, but specifies
     * "charset=UTF-8". Since the default to-character-set is "UTF-8",
     * no conversions are made, they are disabled. At present, the Potext
     * library does not try to analyze the file to find out what the character
     * set really is.
     */

    "library/tests/po/de.po",               /* erroneous, multibyte bad     */

    /*
     * GNU gettext message catalogue, Unicode text, UTF-8 text, but
     * specifies "charset=utf-8".
     */

    "library/tests/po/fr.po",

    /*
     * file: GNU gettext message catalogue, Unicode text, UTF-8 text for
     * all of the following.
     */

    "po/de.po",
    "po/es.po",
    "po/fr.po",
    "po/pl.po"
};

/**
 *  Some tests are designed to fail. So failure is success.`
 */

bool
deliberate_error (const std::string & filename)
{
    return
    (
        filename == "library/tests/broken.po" ||
        filename == "library/tests/level/de.po" ||
        filename == "library/tests/po/de.po"
    );
}

}       // anonymous namespace

/*
 * Main routine.
 */

int
main (int argc, char * argv [])
{
    int result = EXIT_FAILURE;
    bool runtest = true;
    bool runall = false;
    po::logstream::set_enable_testing();    /* useful for detecting issues  */
    std::string arg1;
    if (argc > 1)
        arg1 = std::string(argv[1]);

    if (argc == 1 || arg1 == "--help")
    {
        runtest = false;
        std::cout
            << "Usage:\n\n"
            << "  " << argv[0] << " filename.po [filename.po ...]\n\n"
            << "Options:\n\n"
               "  --all    Run all of the following files.\n"
               "  --help   Show this help text.\n\n"
               "Useful test files are:\n\n"
            ;
            for (const auto & fname : s_all_files)
                std::cout << "  " << fname << "\n";

            std::cout << std::endl;
    }
    if (runtest)
    {
        if (arg1 == "--all")
        {
            runall = true;
        }
        else
        {
            s_all_files.clear();
            for (int i = 1; i < argc; ++i)
            {
                std::string fname = argv[i];
                s_all_files.push_back(fname);
            }
        }

        po::logstream::callbacks_set_all(my_log_callback);
        result = EXIT_SUCCESS;
        for (const auto & fname : s_all_files)
        {
            std::ifstream in(fname);
            if (! in)
            {
                result = EXIT_FAILURE;
                std::cerr
                    << argv[0] << ": cannot access " << fname << ": "
                    << std::strerror(errno)
                    << std::endl
                    ;
                }
            else
            {
                po::logstream::clear_test_error();          /* start fresh! */
                try
                {
                    po::dictionary dict1;
                    bool ok = po::poparser::parse_po_file(fname, in, dict1);
                    if (ok)
                        ok = ! po::logstream::get_test_error();

                    if (ok)
                    {
                        std::cout
                            << _("File") << " " << fname << " "
                            << _("parsed successfully")
                            << std::endl
                            ;
                    }
                    else
                    {
                        std::cout
                            << _("File") << " " << fname
                            << _("parsed unsuccessfully") << "..."
                            << std::endl
                            ;

                        ok = deliberate_error(fname) && runall;
                        if (ok)
                        {
                            std::cout
                                << "..." << _("An expected failure")
                                << std::endl
                                ;
                        }
                        else
                        {
                            result = EXIT_FAILURE;
                        }
                    }
                }
                catch (const std::runtime_error & err)
                {
                    std::cerr
                        << _("File") << " " << fname << ": "
                        << _("Exception") << ": " << err.what()
                        << std::endl
                        ;
                    if (deliberate_error(fname) && runall)
                    {
                        std::cout
                            << "..." << _("An expected failure")
                            << std::endl
                            ;
                    }
                    else
                    {
                        result = EXIT_FAILURE;
                    }
                }
            }
        }
    }
    if (po::logstream::get_test_error())
        result = EXIT_FAILURE;

    return result;
}

/*
 * po_parser_test.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
