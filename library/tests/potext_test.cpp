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
 * \file          potext_test.cpp
 *
 *      A large set of tests for various aspects of the Potext library.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-04-10
 * \license       See above.
 *
 */

#include <cstdlib>                      /* EXIT_SUCCESS, EXIT_FAILURE       */
#include <cstring>                      /* std::strcmp()                    */
#include <fstream>                      /* std::ifstream                    */
#include <iostream>                     /* std::cout and std::cerr          */
#include <stdexcept>                    /* std::runtime_error               */

#include "po/logstream.hpp"             /* po::logstream::get_test_error()  */
#include "po/moparser.hpp"              /* po::moparser class               */
#include "po/poparser.hpp"              /* po::poparser class               */
#include "po/potext.hpp"                /* #includes three header files     */
#include "po/unixfilesystem.hpp"        /* po::unixfilesystem               */
#include "po/wstrfunctions.hpp"         /* po::is_po_file(), is_mo_file()   */

namespace
{

static void
print_msg
(
    const std::string & msgid,
    const std::string & msgid_plural,
    const po::phraselist & msgstrs
)
{
    std::cout << "msgid \"" << msgid << "\""<< std::endl;
    if (msgstrs.size() > 1)
    {
        std::size_t count = 0;
        if (! msgid_plural.empty())
            std::cout << "msgid_plural \"" << msgid_plural << "\"\n";

        for (const auto & msg : msgstrs)
        {
            std::cout << "msgstr[" << count++ << "] \""
                << msg << "\""
                << std::endl
                ;
        }
    }
    else
    {
        for (const auto & msg : msgstrs)
            std::cout << "msgstr \"" << msg << "\"" << std::endl;
    }
}

static void
print_msg_ctxt
(
    const std::string & ctxt,
    const std::string & msgid,
    const std::string & msgid_plural,
    const po::phraselist & msgstrs
)
{
    std::cout << "msgctxt \"" << ctxt << "\"\n";
    print_msg(msgid, msgid_plural, msgstrs);
}

static void
print_usage (const std::string & arg0)
{
    std::cout
<< "Usage:\n\n"
<< "  [a] " << arg0 << " translate <file.po> <msg>\n"
<< "  [b] " << arg0 << " translate <file.po> <context> <msg>\n"
<< "  [c] " << arg0 << " translate <file.po> <singular> <plural> <N>\n"
<< "  [d] " << arg0 << " translate <file.po> <context> <singular> <plural> <N>\n"
<< "  [e] " << arg0 << " directory <dir> <msg> [<lang>]\n"
<< "  [f] " << arg0 << " language <lang>\n"
<< "  [g] " << arg0 << " language-dir <dir>\n"
<< "  [h] " << arg0 << " list-msgstrs <file>\n\n"
<<
   "[a] Create a dictionary from 'file'; translate the 'msg'.\n"
   "[b] Ditto; translate the 'msg' using the 'context'.\n"
   "[c] Ditto; translate 'singular' & Nth form of the 'plural' message.\n"
   "[d] Ditto; translate 'singular' & Nth form of 'plural', using 'context'.\n"
   "[e] Create a language object from 'lang' and show stuff a bit like [g].\n"
   "[f] Get the language by its name (e.g. fr_FR) and show its attributes.\n"
   "[g] Set a dictionary manager using 'dir', get the languages, and list them.\n"
   "[h] Create a dictionary from 'file' and print the messages and contexts.\n\n"
   "Shortcuts: 'tr', 'dir', 'lang', 'ld', and 'lm'\n\n"
<< "See the developer guide (PDF) for more details, especially on the format\n"
   "of the <lang> parameter."
<< std::endl
    ;
}

static void
read_dictionary (const std::string & filename, po::dictionary & dict)
{
    std::ifstream in(filename.c_str());
    if (! in)
    {
        throw std::runtime_error("Could not open " + filename);
    }
    else
    {
        bool ok = false;
        if (po::is_po_file(filename))
        {
            ok = po::poparser::parse_po_file(filename, in, dict);
        }
        else if (po::is_mo_file(filename))
        {
            ok = po::moparser::parse_mo_file(filename, in, dict);
        }

        /*
         * Done in destructor.
         *
         * in.close();
         */

        if (! ok)
            throw std::runtime_error("Could not parse " + filename);
    }
}

}               // namespace

int
main (int argc, char * argv [])
{
    int result = EXIT_SUCCESS;
    po::logstream::set_enable_testing();    /* useful for detecting issues  */
    std::string appname{argv[0]};
    if (appname.length() >= 12)
        appname = "potext_test";

    if (argc == 1)
    {
        print_usage(appname);
        return result;
    }
    try
    {
        std::string option{argv[1]};
        if (option == "language-dir" || option == "ld")
        {
            /*
             * Test [g]
             */

            if (argc == 3)
            {
                /*
                 * The default constructor does this, with the default character
                 * set of UTF-8.
                 *
                 *  po::dictionarymgr dictmgr
                 *  (
                 *      std::unique_ptr<po::filesystem>(new po::unixfilesystem)
                 *  );
                 */

                po::dictionarymgr dictmgr;
                dictmgr.add_directory(argv[2]);

                const std::set<po::language> & langs = dictmgr.get_languages();
                std::cout << "No. of languages: " << langs.size() << std::endl;
                for (auto i = langs.begin(); i != langs.end(); ++i)
                {
                    const po::language & lang = *i;
                    std::cout
                        << "Env:       " << lang.to_string()     << "\n"
                        << "Name:      " << lang.get_name()      << "\n"
                        << "Language:  " << lang.get_language()  << "\n"
                        << "Country:   " << lang.get_country()   << "\n"
                        << "Modifier:  " << lang.get_modifier()  << "\n"
                        << std::endl
                        ;
                }
            }
            else
            {
                result = EXIT_FAILURE;
                std::cerr
                    << "Use format: '"
                    << appname << " language-dir <dir>'"
                    << std::endl
                    ;
            }
        }
        else if (option == "language" || option == "lang")
        {
            /*
             * Test [f]
             */

            if (argc == 3)
            {
                const char * language_cstr = argv[2];
                po::language lang = po::language::from_name(language_cstr);
                if (lang)
                {
                    std::cout
                        << "Env:       " << lang.to_string()     << std::endl
                        << "Name:      " << lang.get_name()      << std::endl
                        << "Language:  " << lang.get_language()  << std::endl
                        << "Country:   " << lang.get_country()   << std::endl
                        << "Modifier:  " << lang.get_modifier()  << std::endl
                        ;
                }
                else
                {
                    result = EXIT_FAILURE;
                    std::cerr << language_cstr << " not found" << std::endl;
                }
            }
            else
            {
                result = EXIT_FAILURE;
                std::cerr
                    << "Use format: '"
                    << appname << " languager <lang>'"
                    << std::endl
                    ;
            }
        }
        else if (option == "translate" || option == "tr")
        {
            /*
             * Tests [a], [b], [c], and [d]
             */

            if (argc == 4)
            {
                const char * filename = argv[2];
                const char * msg  = argv[3];
                po::dictionary dict;
                read_dictionary(filename, dict);

                std::string tr = dict.translate(msg);
                std::cout << "Translation: \"" << tr << "\"" << std::endl;
            }
            else if (argc == 5)
            {
                const char * filename = argv[2];
                const char * context  = argv[3];
                const char * msg  = argv[4];
                po::dictionary dict;
                read_dictionary(filename, dict);

                std::string tr = dict.translate_ctxt(context, msg);
                std::cout
                    << "Context '" << context << "' translation: \""
                    << tr << "\""
                    << std::endl
                    ;
            }
            else if (argc == 6)
            {
                const char * filename = argv[2];
                const char * msg_singular = argv[3];
                const char * msg_plural = argv[4];
                int num = std::stoi(argv[5]);
                po::dictionary dict;
                read_dictionary(filename, dict);

                std::string tr = dict.translate_plural
                (
                    msg_singular, msg_plural, num
                );
                std::cout
                    << "Translation of '" << msg_singular
                    << "' & '" << msg_plural << "': \"" << tr << "\""
                    << std::endl
                    ;
            }
            else if (argc == 7)
            {
                const char * filename = argv[2];
                const char * context  = argv[3];
                const char * msg_singular = argv[4];
                const char * msg_plural   = argv[5];
                int num = std::stoi(argv[6]);
                po::dictionary dict;
                read_dictionary(filename, dict);

                std::string tr = dict.translate_ctxt_plural
                (
                    context, msg_singular, msg_plural, num
                );
                std::cout
                    << "Context '" << context << "' "
                    << "translation of '" << msg_singular
                    << "' & '" << msg_plural << "': \"" << tr << "\""
                    << std::endl
                    ;
            }
            else
            {
                result = EXIT_FAILURE;
                std::cerr
                    << "Use one of the 'translate' options shown here:"
                    << std::endl
                    ;
                print_usage(appname);
            }
        }
        else if (option == "directory" || option == "dir")
        {
            /*
             * Test [e]
             */

            if (argc == 4 || argc == 5)
            {
                const char * dir = argv[2];
                const char * msg   = argv[3];
                const char * lang = (argc == 5) ? argv[4] : nullptr;

                /*
                 * The default constructor does this, with the default character
                 * set of UTF-8.
                 *
                 *  po::dictionarymgr dictmgr
                 *  (
                 *      std::unique_ptr<po::filesystem>(new po::unixfilesystem)
                 *  );
                 */

                po::dictionarymgr mgr;
                mgr.add_directory(dir);
                if (lang)
                {
                    po::language lang2 = po::language::from_name(lang);
                    if (lang2)
                    {
                        mgr.set_language(lang2);
                    }
                    else
                    {
                        std::cout << "Unknown language: " << lang << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
                std::cout
                    << "Directory:   '" << dir << "'\n"
                    << "Message:     '" << msg << "'\n"
                    << "Language:    '" << mgr.get_language().to_string()
                    << "' (name: '"
                    << mgr.get_language().get_name() << "', language: '"
                    << mgr.get_language().get_language() << "', country: '"
                    << mgr.get_language().get_country() << "', modifier: '"
                    << mgr.get_language().get_modifier() << "')\n"
                    << "Translation: '"
                    << mgr.get_dictionary().translate(msg) << "'"
                    << std::endl
                    ;
            }
            else
            {
                result = EXIT_FAILURE;
                std::cerr
                    << "Use format: '"
                    << appname << " directory <dir> <msg> [<lang>]'"
                    << std::endl
                    ;
            }
        }
        else if (option == "list-msgstrs" || option == "lm")
        {
            /*
             * Test [h]
             */

            if (argc == 3)
            {
                const char * filename = argv[2];
                po::dictionary dict;
                read_dictionary(filename, dict);
                dict.foreach(print_msg);
                dict.foreach_ctxt(print_msg_ctxt);
            }
            else
            {
                result = EXIT_FAILURE;
                std::cerr
                    << "Use format: '"
                    << appname << " list-msgstrs <file>'"
                    << std::endl
                    ;
            }
        }
        else
            print_usage(appname);
    }
    catch (const std::exception & err)
    {
        std::cout << "Exception: " << err.what() << std::endl;
        result = EXIT_FAILURE;
    }
    if (result == EXIT_SUCCESS)
    {
        if (po::logstream::get_test_error())
            result = EXIT_FAILURE;
    }
    return result;
}

/*
 * potext_test.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

