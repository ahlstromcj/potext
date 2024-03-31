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
 * \file          po_parser_test.hpp
 *
 *  Macros that depend upon the build platform.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-16
 * \license       See above.
 *
 */

#include <cerrno>                       /* errno                            */
#include <cstdlib>                      /* EXIT_SUCCESS, EXIT_FAILURE       */
#include <cstring>                      /* std::strerror()                  */
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "po/poparser.hpp"
#include "po/potext.hpp"
#include "po/logstream.hpp"

/*
 *  The usage of an anonymous namespace is essentially the same as using
 *  the "static" keyword here, except it can also hide type declarations.
 */

namespace
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
    "library/tests/broken.po",              /* erroneous, no closing quote  */
    "library/tests/de.po",
    "library/tests/helloworld/de.po",
    "library/tests/level/de.po",            /* erroneous, bad CHARSET       */
    "library/tests/po/de_AT.po",
    "library/tests/po/de.po",               /* erroneous, multibyte bad     */
    "library/tests/po/fr.po",
    "po/de.po",
    "po/es.po",
    "po/fr.po",
    "po/pl.po"
};

/**
 *  Some tests are designed to fail.
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

} // namespace

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
        if (arg1 != "--all")
        {
            runall = true;
            s_all_files.clear();
            for (int i = 1; i < argc; ++i)
            {
                std::string fname = argv[i];
                s_all_files.push_back(fname);
            }
        }

        po::logstream::callbacks_set_all(my_log_callback);
        result = EXIT_SUCCESS;
        for (int i = 1; i < argc; ++i)
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
                            po::logstream::clear_test_error();
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
                        po::logstream::clear_test_error();
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
