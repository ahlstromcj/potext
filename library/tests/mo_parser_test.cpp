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
 * \file          mo_parser_test.hpp
 *
 *      Simple test of parsing .mo files.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-04-02
 * \updates       2024-04-08
 * \license       See above.
 *
 */

#include <cerrno>                       /* errno                            */
#include <cstdlib>                      /* EXIT_SUCCESS, EXIT_FAILURE       */
#include <cstring>                      /* std::strerror()                  */
#include <string>                       /* std::string                      */
#include <iostream>                     /* std::cout                        */
#include <fstream>                      /* std::ifstream                    */

#include "po/moparser.hpp"              /* po::moparser to test             */
#include "po/potext.hpp"                /* po::dictionarymgr, po::gettext   */
#include "po/logstream.hpp"             /* po::logstream                    */

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
    std::cerr << "[moparser test] " << err;
}

/**
 *  Use with the "--all" option to run through all files. With no such
 *  option, the command-line arguments replace this list of files for the
 *  run.
 */

po::phraselist s_all_files
{
    "library/tests/mo/es/colord.mo",
    "library/tests/mo/es/garcon.mo",
    "library/tests/mo/es/newt.mo",
    "library/tests/mo/de/helloworld.mo"
};

} // namespace

/*
 * Main routine.
 */

int
main (int argc, char * argv [])
{
    int result = EXIT_FAILURE;
    bool runtest = true;
    po::logstream::set_enable_testing();    /* useful for detecting issues  */
    std::string arg1;
    if (argc > 1)
        arg1 = std::string(argv[1]);

    if (argc == 1 || arg1 == "--help")
    {
        runtest = false;
        std::cout
            << "Usage:\n\n"
            << "  " << argv[0] << " filename.mo [filename.mo ...]\n\n"
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
                    bool ok = po::moparser::parse_mo_file(fname, in, dict1);
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
                        result = EXIT_FAILURE;
                    }
                }
                catch (const std::runtime_error & err)
                {
                    std::cerr
                        << _("File") << " " << fname << ": "
                        << _("Exception") << ": " << err.what()
                        << std::endl
                        ;
                    result = EXIT_FAILURE;
                }
            }
        }
    }
    if (po::logstream::get_test_error())
        result = EXIT_FAILURE;

    return result;
}

/*
 * mo_parser_test.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
