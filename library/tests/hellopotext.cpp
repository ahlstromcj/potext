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
 * \file          hellopotext.cpp
 *
 *      Provides a test of potext from start to finish, but only for
 *      .po files.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-02-24
 * \updates       2024-04-11
 * \license       See above.
 *
 *  This program is meant to be a real-life usage of the potext library.
 *
 *  Should we also make it buildable and runnable with GNU's gettext as
 *  well? Naaaaaaaah.
 *
 *  Note that many of the test translations supplied are probably wrong
 *  in some way, such as "File" being converted to a French verb instead
 *  of a French noun. But we're merely testing lookup here. If it bugs you,
 *  fix it and make a pull request.
 *
 *  On another note, since we know that strings like "FAILED" are not
 *  in any dictionary, we do not use the _() marker with them.
 */

#include <cstdlib>                      /* EXIT_SUCCESS, EXIT_FAILURE       */
#include <iostream>                     /* std::cout and std::cerr          */
#include <stdexcept>                    /* std::runtime_error               */

#include "po/potext.hpp"                /* po::textdomain() gettext() etc.  */
#include "po/logstream.hpp"             /* po::logstream functions          */

namespace
{

/**
 *  Provides the --help option.
 */

static void
print_usage (const std::string & arg0)
{
    std::cout
<< "Usage:\n\n"
<< "  [a] " << arg0 << "\n"
<< "  [b] " << arg0 << " <domain>\n"
<< "  [b] " << arg0 << " <domain> <localedir>\n"
<< "\n"
   "[a] Create a smoke test with a default domain(s) and locale directory.\n"
   "[b] Use the given domain and the default locale directory.\n"
   "[e] Use the given domain and locale directory.\n"
<< "See the developer guide (PDF) for more details.\n"
<< std::endl
    ;
}

}               // anonymous namespace

/*
 *  The first gettext() smoke test phrase is present in po/es.po. The second
 *  is not.
 *
 *  Exception throwing occurs in po::iconvert::set_charsets(),
 *  poparser::error() [called by pretty much any function in poparser].
 *  The dictionarymgr calls parse_po_file(), but it catches the exceptions
 *  and returns bogus return value.
 */

static bool
gettext_smoke_test (const std::string & dom_name)
{
    bool result = true;
    std::string unknown = "Unknown system error";
    std::string smoketest = _("Unknown system error");
    std::cout
        << "gettext('" << unknown << "') "
        << "[" << _("domain") << " '" << dom_name << "'] = '"
        << smoketest << "'"
        ;
    bool ok = smoketest == "Hay un error desconocido del sistema";
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;

    unknown = "No such sentence in dictionary";
    smoketest = _("No such sentence in dictionary");
    std::cout
        << "gettext('" << unknown << "') "
        << "[" << _("domain") << " '" << dom_name << "'] = '"
        << smoketest << "'"
        ;
    ok = smoketest == "No such sentence in dictionary";
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    return result;
}

/*
 *  Four cases are supported. We need to figure out how to detect
 *  a non-translation case.
 *
 *  "Unknown system error" is present in all the .po files in the
 *  project's po directory.
 */

static bool
dgettext_smoke_test
(
    const std::string & dom_name,
    const std::string & expected
)
{
    std::string unknown = "Unknown system error";
    std::string smoketest = po::dgettext(dom_name, unknown);
    std::cout
        << "dgettext('" << dom_name << "', '" << unknown << "') = '"
        << smoketest << "'"
        ;

    bool result = smoketest == expected;
    if (! result)
        std::cout << " " << "FAILED";

    std::cout << std::endl;
    return result;
}

/**
 *  We do not currently handle category parameters.
 */

static bool
dcgettext_smoke_test (const std::string & dom_name)
{
    std::cout
        << "dcgettext(" << dom_name << ") " << "not yet implemented"
        << std::endl
        ;

    return true;
}

/*
 *  A test of plural processing. Languages in the "po" directory;
 *
 *      -   de.  "Plural-Forms: nplurals=2; plural=(n != 1);\n"
 *      -   es.  "Plural-Forms: nplurals=2; plural=(n != 1);\n"
 *      -   fr.  "Plural-Forms: nplurals=2; plural=(n > 1);\n"
 *
 *  This smoke test handles only the "es" domain.
 */

static bool
ngettext_smoke_test (const std::string & dom_name)
{
    bool result = true;
    std::string smoketest = po::ngettext("File", "Files", 1);
    std::cout
        << "ngettext('File', 'Files', 1) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;

    bool ok = smoketest == "Archivos";
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    smoketest = po::ngettext("File", "Files", 2);
    std::cout
        << "ngettext('File', 'Files', 2) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;
    ok = smoketest == "Archivo";
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    smoketest = po::ngettext("Person", "People", 1);
    std::cout
        << "ngettext('Person', 'People', 1) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;
    ok = smoketest == "Persona";
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    smoketest = po::ngettext("Person", "People", 2);
    std::cout
        << "ngettext('Person', 'People', 2) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;
    ok = smoketest == "Gente";
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    return result;
}

static bool
dngettext_smoke_test
(
    const std::string & dom_name,
    const std::string & expected,
    const std::string & plural
)
{
    bool result = true;
    std::string smoketest = po::dngettext(dom_name, "File", "Files", 1);
    std::cout
        << "dngettext('File', 'Files', 1) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;

    bool ok = smoketest == expected;
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    smoketest = po::dngettext(dom_name, "File", "Files", 2);
    std::cout
        << "dngettext('File', 'Files', 2) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;
    ok = smoketest == plural;
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    return result;
}

static bool
dngettext_smoke_test_2
(
    const std::string & dom_name,
    const std::string & expected,
    const std::string & plural
)
{
    bool result = true;
    std::string smoketest = po::dngettext(dom_name, "Person", "People", 1);
    std::cout
        << "dngettext('Person', 'Persons', 1) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;

    bool ok = smoketest == expected;
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    smoketest = po::dngettext(dom_name, "Person", "People", 2);
    std::cout
        << "dngettext('Person', 'People', 2) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;
    ok = smoketest == plural;
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    return result;
}

/**
 *  This test of converting based on domain name and category is
 *  NOT READY.
 */

static bool
dcngettext_smoke_test (const std::string & dom_name)
{
    std::cout
        << "dcngettext(" << dom_name << ")" << " not yet implemented"
        << std::endl
        ;
    return true;    /* yeah, we know */
}

/**
 *  A test of getting a message translation by context (without plurals
 *  being involved). The contexts are "success" and "failure", with the
 *  latter attempting to be sarcastic.
 */

static bool
pgettext_smoke_test
(
    const std::string & dom_name,
    const std::string & expected1,
    const std::string & expected2
)
{
    bool result = true;
    std::string smoketest = po::pgettext("success", "Congratulations!");
    std::cout
        << "pgettext('success', 'Congratulations!') [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;

    bool ok = smoketest == expected1;
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    smoketest = po::pgettext("failure", "Congratulations!");
    std::cout
        << "pgettext('failure', 'Congratulations!') [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;
    ok = smoketest == expected2;
    if (! ok)
    {
        result = false;
        std::cout << " " << "FAILED";
    }
    std::cout << std::endl;
    return result;
}

#if defined USE_THIS_IN_NCGETTEXT

    /*
     * This is a plural with a message context.
     */

    smoketest = po::ngettext("Retry", "Retries", 1);
    std::cout
        << "ngettext('Retry', 'Retries', 1) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;
    smoketest = po::ngettext("Retry", "Retries", 2);
    std::cout
        << "ngettext('Retry', 'Retries', 2) [" << _("domain") << " '"
        << dom_name << "'] = '" << smoketest << "'"
        ;

#endif

/*
 *  Here we set the default domain for po::gettext() to "es".
 *  Then we pass alternate domains ("fr" or "de") to
 *  po::dgettext().
 */

static bool
directory_test
(
    const std::string & arg0,
    const std::string & domain,     // es
    const std::string & dirname     // po
)
{
    bool result = true;
    std::string dir_name = po::init_app_locale  /* returns "po" dir */
    (
        arg0, "hellopotext", domain, dirname
    );
    std::cout
        << arg0 << ": " << _("domain") << " " << domain
        << " " << _("directory") << " " << dirname
        << std::endl
        ;
    if (dir_name.empty())
    {
        result = false;
        std::cerr << _("Could not process domain") << " '"
            << domain << "' and directory '" << dirname << "'"
            << std::endl
            ;
    }
    else
    {
        bool ok = gettext_smoke_test(domain);
        if (! ok)
            result = false;

        /*
         * Smoke tests for dgettext().
         */

        std::string expected = "Hay un error desconocido del sistema";
        ok = dgettext_smoke_test(domain, expected);
        if (! ok)
            result = false;

        expected = "Erreur système non identifiée";
        ok = dgettext_smoke_test("fr", expected);
        if (! ok)
            result = false;

        expected = "Unbekannter Systemfehler";
        ok = dgettext_smoke_test("de", expected);
        if (! ok)
            result = false;

        expected = "Unknown system error";
        ok = dgettext_smoke_test("xx", expected);
        if (! ok)
            result = false;

        /*
         * dcgettext() is not ready, and currently just calls
         * dgettext(), already tested above. Handling categories
         * requires some extra hard work.
         */

        (void) dcgettext_smoke_test(domain);

        /*
         * ngettext_smoke_test() tests plurals in the default domain.
         * The responses are easy to check in the test function.
         */

        ok = ngettext_smoke_test(domain);
        if (! ok)
            result = false;

        /**
         * dngettext_smoke_test() tests plurals in the given domain.
         *
         * dcngettext_smoke_test() tests plurals in the given domain
         * and category. We have to provide the translations to the
         * test function (or we could check the domain and get its
         * expected results.
         */

        std::string plural = "Archivos";
        expected = "Archivo";
        ok = dngettext_smoke_test("es", expected, plural);
        if (! ok)
            result = false;

        plural = "Des dossiers";
        expected = "Déposer";
        ok = dngettext_smoke_test("fr", expected, plural);
        if (! ok)
            result = false;

        plural = "Dateien";
        expected = "Datei";
        ok = dngettext_smoke_test("de", expected, plural);
        if (! ok)
            result = false;

        plural = "File";
        expected = "File";
        ok = dngettext_smoke_test("xx", expected, plural);
        if (! ok)
            result = false;

        plural = "Gente";
        expected = "Persona";
        ok = dngettext_smoke_test_2(domain, expected, plural);
        if (! ok)
            result = false;

        plural = "Personnes";
        expected = "Personne";
        ok = dngettext_smoke_test_2("fr", expected, plural);
        if (! ok)
            result = false;

        plural = "Menschen";
        expected = "Person";
        ok = dngettext_smoke_test_2("de", expected, plural);
        if (! ok)
            result = false;

        plural = "Person";
        expected = "Person";
        ok = dngettext_smoke_test_2("xx", expected, plural);
        if (! ok)
            result = false;
        /*
         *  This function is not ready.
         */

        (void) dcngettext_smoke_test(domain);

        /**
         *  pgettext() test. This was a macro in GNU Gettext but
         *  is a function in the Potext library.
         */

        std::string expected1 = "¡Felicidades!";
        std::string expected2 = "¡Gran trabajo amigo!"; /* sarcasm  */
        ok = pgettext_smoke_test("es", expected1, expected2);
        if (! ok)
            result = false;
    }
    return result;
}

#if 0
        /*
         *  The pgettext() function does not have a domain parameter.
         */

        ok = pgettext_smoke_test("es", expected1, expected2);
        if (! ok)
            result = false;

        expected1 = "Toutes nos félicitations!";
        expected2 = "Super boulot mec!";                /* sarcasm  */
        ok = pgettext_smoke_test("fr", expected1, expected2);
        if (! ok)
            result = false;

        expected1 = "Glückwunsch!";
        expected2 = "Tolle Arbeit, Alter!";             /* sarcasm  */
        ok = pgettext_smoke_test("de", expected1, expected2);
        if (! ok)
            result = false;
#endif  // 0

/*
 *  The main routine.
 */

int
main (int argc, char * argv [])
{
    int result = EXIT_SUCCESS;
    bool success = true;
    po::logstream::set_enable_testing();    /* useful for detecting issues  */
    std::string appname{argv[0]};
    if (appname.length() >= 12)
        appname = "hellopotext";

    try
    {
        if (argc == 1)
        {
            std::string arg0 = std::string(argv[0]);
            std::string dom_name = "es";
            std::string dir_name = "po";
            result = directory_test(arg0, dom_name, dir_name);
        }
        else if (argc == 2)
        {
            std::string arg = std::string(argv[1]);
            if (arg == "--help")
            {
                print_usage(appname);
            }
            else
            {
                /*
                 * The first smoke test is present in po/es.po. The second
                 * is not. We will likely add the translations to the other
                 * files.
                 */

                std::string arg0 = std::string(argv[0]);
                std::string domain = std::string(argv[1]);
                result = directory_test(arg0, domain, "po");
#if defined USE_THIS_TEST
                std::string dom_name = po::init_app_locale
                (
                    arg0, "hellopotext", domain, "po"
                );
                if (! dom_name.empty())
                {
                    std::string smoketest = _("Unknown system error");
                    std::cout
                        << _("Language domain") << " " << dom_name << std::endl
                        << _("Test result") << " 1: '" << smoketest << "'"
                        << std::endl
                        ;
                    smoketest = _("No such sentence in dictionary");
                    std::cout
                        << _("Test result") << " 2: '" << smoketest << "'"
                        << std::endl
                        ;
                }
#endif
            }
        }
        else if (argc == 3)
        {
            std::string arg0 = std::string(argv[0]);
            std::string domain = std::string(argv[1]);
            std::string dirname = std::string(argv[2]);
            result = directory_test(arg0, domain, dirname);
        }
    }
    catch (const std::exception & err)
    {
        std::cout << "Exception: " << err.what() << std::endl;
        result = EXIT_FAILURE;
        success = false;
    }
    if (success && result == EXIT_SUCCESS)
    {
        /*
         * Some errors are deliberate.
         *
         *  if (po::logstream::get_test_error())
         *      result = EXIT_FAILURE;
         */

        std::cout << "SUCCESS" << std::endl;
    }
    else
        std::cout << "FAILURE" << std::endl;

    return result;
}

/*
 * hellopotext.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

