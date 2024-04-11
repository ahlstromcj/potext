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
 * \file          pluralforms.cpp
 *
 *      Support for looking up .po/.mo Plural-Forms and applying them.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-04-11
 * \license       See above.
 *
 *  https://www.gnu.org/software/gettext/manual/
 *
 *  char * ngettext (const char * id1, const char * id2, unsigned long int N)
 *
 *  The first message ID  contains the singular form of the string to
 *  convert, and is the key for the search in the catalog. The second ID
 *  is the plural form. N determines the plural form. With no message catalog
 *  id1 is returned if n == 1, otherwise id2.
 *
 *      printf( ngettext("%d file removed", "%d files removed", n), n );
 *
 *  Notes:
 *
 *      -   operator ?: The expression in the middle of the conditional
 *          operator (between ? and :) is parsed as if parenthesized: its
 *          precedence relative to ?: is ignored. Only the assignment and
 *          comma operators have a lower precedence.
 *      -   Normally, 0 = singular, 1 = first plural form, etc.
 *          But see the function banner comments below.
 *
 */

#include <cctype>                       /* std::isspace() function          */

#include "po/pluralforms.hpp"           /* po::pluralforms class            */

namespace po
{

/**
 *  Plural functions are used to select a string that matches a given
 *  count. \a n is the count and the return value is the string index
 *  used in the .po file, for example:
 *
 *   msgstr[0] = "You got %d error";
 *   msgstr[1] = "You got %d errors";
 *          ^-- return value of plural function
 */

/*
 *  No plural forms: Plural-Forms: nplurals=1; plural=0;
 *
 *      Japanese, Vietnamese, Korean, Thai, etc.
 *
 *  There is no distinction between the singular and plural form.
 *  The "plural rules" is empty.
 */

static unsigned
plural1 (int)
{
    return 0;
}

/**
 *  One plural form, one singular, singular is used for 1 only.
 *  See the code in glibc-2.39/intl/dcigettext.c, If the category is
 *  out-of-range or equal to LC_ALL, then if n == 1, msgid1 is returned,
 *  else msgid2 is returned. Otherwise plural_eval() [intl/eval-plural.h]
 *  is called to get the index.
 *
 *      Plural-Forms: nplurals=2; plural=n != 1;
 *
 *      English, German, Spanish, Dutch, Swedish, Danish, Norwegian,
 *      and many more "non-Germanic" lanuages like Greek.
 *
 *  This is referred to as the "Germanic" form. In this form, the singular
 *  form (0) is used for n equal to 1 only.
 *
 *  Consider Hungarian and Turkish, though:
 *
 *      For an explicit number:
 *
 *          "1 apple" = "1 alma"
 *          "2 apples" = "2 alma"
 *
 *     No explicit number:
 *
 *          "the apple" = "az alma"
 *          "the apples" = "az almak"
 */

static unsigned
plural2_1 (int n)
{
    return n != 1 ? 1 : 0 ;
}

/**
 *  Two forms, singular used for zero and one:
 *
 *      Plural-Forms: nplurals=2; plural=n>1;
 *
 *      Brazilian, Portuguese, French etc.
 */

static unsigned
plural2_2 (int n)
{
    return n > 1 ? 1 : 0 ;
}

/**
 *  See this URL for the rest of these special cases.
 *
 *  https://www.gnu.org/software/gettext/manual/html_node/Plural-forms.html
 */

static unsigned
plural2_mk (int n)
{
    return n == 1 || n % 10 == 1 ? 0 : 1 ;
}

static unsigned
plural2_mk_2 (int n)
{
    return (n % 10 == 1 && n % 100 != 11) ? 0 : 1 ;
}

/**
 */

static unsigned
plural3_es (int n)
{
    n == 1 ? 0 : n != 0 && n % 1000000 == 0 ? 1 : 2 ;
}

/**
 *   Three forms, special case for 0.
 *
 *      Plural-Forms: nplurals=3; plural= as written below.
 *
 *      Baltic family, Latvian
 */

static unsigned
plural3_lv (int n)
{
    n % 10 == 1 && n % 100 != 11 ? 0 : n != 0 ? 1 : 2 ;
}

/**
 *  Three forms, special cases for one and two.
 *
 *      Plural-Forms: nplurals=3; plural=n==1 ? 0 : n==2 ? 1 : 2;
 *
 *      Celtic, Gaeilge (Irish)
 */

static unsigned
plural3_ga (int n)
{
    return n == 1 ? 0 : n == 2 ? 1 : 2 ;
}

static unsigned
plural3_lt (int n)
{
    n % 10 == 1 && n % 100 != 11 ?
        0 : n % 10 >= 2 && (n % 100 < 10 || n % 100 >= 20) ? 1 : 2 ;
}

static unsigned
plural3_1 (int n)
{
    n % 10 == 1 && n % 100 != 11 ?
        0 : n % 10 >= 2 && n % 10 <= 4 && (n % 100 < 10 ||
            n % 100 >= 20) ? 1 : 2 ;
}

static unsigned
plural3_sk (int n)
{
    return n == 1 ? 0 : (n >= 2 && n <= 4) ? 1 : 2 ;
}

static unsigned
plural3_pl (int n)
{
    n == 1 ? 0 : n % 10 >= 2 && n % 10 <= 4 &&
        (n % 100 < 10 || n % 100 >= 20) ? 1 : 2 ;
}

static unsigned
plural3_ro (int n)
{
    n == 1 ? 0 : ((n % 100 > 19) || ((n % 100 == 0) && (n != 0))) ? 2 : 1 ;
}

/*
 * What a screwy implementation!
 */

static unsigned
plural3_sl (int n)
{
    n % 100 == 1 ? 0 : n % 100 == 2 ?
        1 : n % 100 == 3 || n % 100 == 4 ? 2 : 3 ;
}

static unsigned
plural4_be (int n)
{
    n % 10 == 1 && n % 100 != 11 ?
        0 : n % 10 >= 2 && n % 10 <= 4 &&
            (n % 100 < 12 || n % 100 > 14) ?
                1 : n % 10 == 0 || (n % 10 >= 5 && n % 10 <= 9) ||
                    (n % 100 >= 11 && n % 100 <= 14) ? 2 : 3 ;
}

static unsigned
plural4_cs (int n)
{
    (n == 1 && n % 1 == 0) ?
        0 : (n >= 2 && n <= 4 && n % 1 == 0) ?
            1 : (n % 1 != 0 ) ? 2 : 3 ;
}

static unsigned
plural4_cy (int n)
{
    n == 1 ? 0 : (n == 2) ? 1 : (n != 8 && n != 11) ? 2 : 3 ;
}

static unsigned
plural4_gd (int n)
{
    (n == 1 || n == 11) ?
        0 : (n == 2 || n == 12) ? 1 : (n > 2 && n < 20) ? 2 : 3 ;
}

static unsigned
plural4_he (int n)
{
    (n == 1 && n % 1 == 0) ?
        0 : (n == 2 && n % 1 == 0) ?
            1 : (n % 10 == 0 && n % 1 == 0 && n > 10) ? 2 : 3 ;
}

static unsigned
plural4_lt (int n)
{
    n % 10 == 1 && (n % 100 > 19 || n % 100 < 11) ?
        0 : (n % 10 >= 2 && n % 10 <= 9) &&
            (n % 100 > 19 || n % 100 < 11) ? 1 : n % 1 != 0 ? 2 : 3 ;
}

static unsigned
plural4_pl (int n)
{
    n == 1 ? 0 : (n % 10 >= 2 && n % 10 <= 4) &&
        (n % 100 < 12 || n % 100 > 14) ?
            1 : (n != 1 && (n % 10 >= 0 && n % 10 <= 1)) ||
                (n % 10 >= 5 && n % 10 <= 9) ||
                    (n % 100 >= 12 && n % 100 <= 14) ? 2 : 3 ;
}

static unsigned
plural4_sk (int n)
{
    n % 1 == 0 && n == 1 ? 0 : n % 1 == 0 && n >= 2 && n <= 4 ?
        1 : n % 1 != 0 ? 2 : 3 ;
}

static unsigned
plural4_uk (int n)
{
    n % 1 == 0 && n % 10 == 1 && n % 100 != 11 ?
        0 : n % 1 == 0 && n % 10 >= 2 && n % 10 <= 4 &&
            (n % 100 < 12 || n % 100 > 14) ?
                1 : n % 1 == 0 &&
                    (n % 10 == 0 || (n % 10 >= 5 && n % 10 <= 9) ||
                        (n % 100 >= 11 && n % 100 <= 14)) ? 2 : 3 ;
}

static unsigned
plural5_ga (int n)
{
    n == 1 ? 0 : n == 2 ? 1 : n < 7 ? 2 : n < 11 ? 3 : 4 ;
}

static unsigned
plural6_ar (int n)
{
    n == 0 ? 0 : n == 1 ?  1 : n == 2 ? 2 : n % 100 >= 3 && n % 100 <= 10 ?
        3 : n % 100 >= 11 ? 4 : 5 ;
}

/**
 *  The constructors.
 */

pluralforms::pluralforms () :
    m_nplural   (),
    mf_plural   ()
{
    // no code
}

pluralforms::pluralforms (unsigned nplural, function plural) :
    m_nplural   (nplural),
    mf_plural   (plural)
{
    // no code
}

/**
 *  Macros to save space and improve readability.
 */

#define PF  "nplurals="                 /* "Plural-Forms:nplurals="     */
#define PE  ";plural="

/**
 *  Picks a plural-forms function based an string representing it.
 */

pluralforms
pluralforms::from_string (const std::string & str)
{
    /*
     * Note that the plural forms here shouldn't contain any spaces.
     * Also note we use the C/C++ feature of string concatenation.
     * Lastly, note that we could also replace all of the explicit static
     * functions with lambda functions. A job for when we feel ambitious.
     */

#if defined POTEXT_BRUTE_FORCE_INITIALIZER
#include "po/bfplurals.hpp"             /* brute-force initialization       */
#else

    static map s_plural_forms
    {
        {
            PF "1" PE "0;",
            pluralforms(1, plural1)
        },
        {
            PF "2" PE "(n!=1);",        /* this one seems "normal"          */
            pluralforms(2, plural2_1)
        },
        {
            PF "2" PE "n!=1;",          /* this one seems not "normal"      */
            pluralforms(2, plural2_1)
        },
        {
            PF "2" PE "(n>1);",
            pluralforms(2, plural2_2)
        },
        {
            PF "2" PE "n==1||n%10==1?0:1;",
            pluralforms(2, plural2_mk)
        },
        {
            PF "2" PE "(n%10==1&&n%100!=11)?0:1;",
            pluralforms(2, plural2_mk_2)
        },

        /*
         * New for version 0.2. Found in the files
         * library/tests/mo/es/colord.mo and garcon.mo.
         */

        {
            PF "3" PE "n==1?0:n!=0&&n%1000000==0?1:2;",
            pluralforms(2, plural3_es)
        },
        {
            PF "3" PE "n%10==1&&n%100!=11?0:n!=0?1:2);",
            pluralforms(2, plural3_lv)
        },
        {
            PF "3" PE "n==1?0:n==2?1:2;",
            pluralforms(3, plural3_ga)
        },
        {
            PF "3" PE
            "(n%10==1&&n%100!=11?0:n%10>=2&&(n%100<10||n%100>=20)?1:2);",
            pluralforms(3, plural3_lt)
        },
        {
            PF "3" PE
            "(n%10==1&&n%100!=11?0:n%10>=2&&n%10<=4&&(n%100<10||n%100>=20)?1:2);",
            pluralforms(3, plural3_1)
        },
        {
            PF "3" PE "(n==1)?0:(n>=2&&n<=4)?1:2;",
            pluralforms(3, plural3_sk)
        },
        {
            PF "3" PE "(n==1?0:n%10>=2&&n%10<=4&&(n%100<10||n%100>=20)?1:2);",
            pluralforms(3, plural3_pl)
        },
        {
            PF "3" PE "(n%100==1?0:n%100==2?1:n%100==3||n%100==4?2:3);",
            pluralforms(3, plural3_sl)
        },
        {
            PF "3" PE "(n==1?0:(((n%100>19)||((n%100==0)&&(n!=0)))?2:1));",
            pluralforms(3, plural3_ro)
        },
        {
            PF "4" PE "(n%1==0&&n==1?0:n%1==0&&n>=2&&n<=4?1:n%1!=0?2:3);",
            pluralforms(4, plural4_sk)
        },
        {
            PF "4" PE "(n==1&&n%1==0)?0:(n>=2&&n<=4&&n%1==0)?1:(n%1!=0)?2:3;",
            pluralforms(4, plural4_cs)
        },
        {
            PF "4" PE
            "(n%10==1&&n%100!=11?0:n%10>=2&&n%10<=4&&(n%100<12||n%100>14)"
            "?1:n%10==0||(n%10>=5&&n%10<=9)||(n%100>=11&&n%100<=14)?2:3);",
            pluralforms(4, plural4_be)
        },
        {
            PF "4" PE "(n==1||n==11)?0:(n==2||n==12)?1:(n>2&&n<20)?2:3;",
            pluralforms(4, plural4_gd)
        },
        {
            PF "4" PE "(n==1)?0:(n==2)?1:(n!=8&&n!=11)?2:3;",
            pluralforms(4, plural4_cy)
        },
        {
            PF "4" PE
            "(n%10==1&&(n%100>19||n%100<11)?0:(n%10>=2&&n%10<=9)&&"
            "(n%100>19||n%100<11)?1:n%1!=0?2:3);",
            pluralforms(4, plural4_lt)
        },
        {
            PF "4" PE
            "(n%1==0&&n%10==1&&n%100!=11?0:n%1==0&&n%10>=2&&n%10<=4&&"
            "(n%100<12||n%100>14)?1:n%1==0&&(n%10==0||(n%10>=5&&"
            "n%10<=9)||(n%100>=11&&n%100<=14))?2:3);",
            pluralforms(4, plural4_uk)
        },
        {
            PF "4" PE
            "(n==1?0:(n%10>=2&&n%10<=4)&&(n%100<12||n%100>14)?1:n!=1&&"
            "(n%10>=0&&n%10<=1)||(n%10>=5&&n%10<=9)||(n%100>=12&&:"
            "n%100<=14)?2:3);",
            pluralforms(4, plural4_pl)
        },
        {
            PF "4" PE
            "(n==1&&n%1==0)?0:(n==2&&n%1==0)?1:(n%10==0&&n%1==0&&n>10)?2:3;",
            pluralforms(4, plural4_he)
        },
        {
            PF "5" PE "(n==1?0:n==2?1:n<7?2:n<11?3:4)",
            pluralforms(5, plural5_ga)
        },
        {
            PF "6" PE
            "n==0?0:n==1?1:n==2?2:n%100>=3&&n%100<=10?3:n%100>=11?4:5",
            pluralforms(6, plural6_ar)
        },
    };

#endif

    /**
     * Remove spaces from string before lookup. Also, .mo files do not
     * terminate the plural-form with a semi-colon, so we restore it here.
     */

    std::string spaceless_str;
    for (auto c : str)
    {
        if (! std::isspace(c))
            spaceless_str += c;
    }
    if (spaceless_str.back() != ';')
        spaceless_str += ';';

    map::const_iterator it = s_plural_forms.find(spaceless_str);
    return it != s_plural_forms.end() ? it->second : pluralforms() ;
}

}           // namespace po

/*
 * pluralforms.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
