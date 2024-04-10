#if defined POTEXT_BRUTE_FORCE_INITIALIZER

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
 * \file          bfplurals.hpp
 *
 *      Defines the brute force initialization of the plural forms.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-16
 * \updates       2024-04-10
 * \license       See above.
 *
 *  This file is needed only if one needs to support compilers to old to
 *  support static initializers.
 *
 *  This module is best edited in wide-screen.
 */

/**
 *  A set of plural forms.
 */

static pluralforms::map s_plural_forms;

/**
 *  Set up the forms by brute-force assignements. Note that PF and PE
 *  are defined in the pluralforms module.
 */

if (s_plural_forms.empty())
{
    s_plural_forms[PF "1" PE "0;"] = pluralforms(1, plural1);
    s_plural_forms[PF "2" PE "(n!=1);"] = pluralforms(2, plural2_1);
    s_plural_forms[PF "2" PE "n!=1;"] = pluralforms(2, plural2_1);
    s_plural_forms[PF "2" PE "(n>1);"] = pluralforms(2, plural2_2);
    s_plural_forms[PF "2" PE "n==1||n%10==1?0:1;"] = pluralforms(2, plural2_mk);
    s_plural_forms[PF "2" PE "(n%10==1&&n%100!=11)?0:1;"] = pluralforms(2, plural2_mk_2);
    s_plural_forms[PF "3" PE "n%10==1&&n%100!=11?0:n!=0?1:2);"] = pluralforms(2, plural3_lv);
    s_plural_forms[PF "3" PE "n==1?0:n==2?1:2;"] = pluralforms(3, plural3_ga);
    s_plural_forms[PF "3" PE "(n%10==1&&n%100!=11?0:n%10>=2&&(n%100<10||n%100>=20)?1:2);"] = pluralforms(3, plural3_lt);
    s_plural_forms[PF "3" PE "(n%10==1&&n%100!=11?0:n%10>=2&&n%10<=4&&(n%100<10||n%100>=20)?1:2);"] = pluralforms(3, plural3_1);
    s_plural_forms[PF "3" PE "(n==1)?0:(n>=2&&n<=4)?1:2;"] = pluralforms(3, plural3_sk);
    s_plural_forms[PF "3" PE "(n==1?0:n%10>=2&&n%10<=4&&(n%100<10||n%100>=20)?1:2);"] = pluralforms(3, plural3_pl);
    s_plural_forms[PF "3" PE "(n%100==1?0:n%100==2?1:n%100==3||n%100==4?2:3);"] = pluralforms(3, plural3_sl);
    s_plural_forms[PF "3" PE "(n==1?0:(((n%100>19)||((n%100==0)&&(n!=0)))?2:1));"] = pluralforms(3, plural3_ro);
    s_plural_forms[PF "4" PE "(n%1==0&&n==1?0:n%1==0&&n>=2&&n<=4?1:n%1!=0?2:3);"] = pluralforms(4, plural4_sk);
    s_plural_forms[PF "4" PE "(n==1&&n%1==0)?0:(n>=2&&n<=4&&n%1==0)?1:(n%1!=0)?2:3;"] = pluralforms(4, plural4_cs);
    s_plural_forms[PF "4" PE "(n%10==1&&n%100!=11?0:n%10>=2&&n%10<=4&&(n%100<12||n%100>14)?1:n%10==0||(n%10>=5&&n%10<=9)||(n%100>=11&&n%100<=14)?2:3);"] = pluralforms(4, plural4_be);
    s_plural_forms[PF "4" PE "(n==1||n==11)?0:(n==2||n==12)?1:(n>2&&n<20)?2:3;"] = pluralforms(4, plural4_gd);
    s_plural_forms[PF "4" PE "(n==1)?0:(n==2)?1:(n!=8&&n!=11)?2:3;"] = pluralforms(4, plural4_cy);
    s_plural_forms[PF "4" PE "(n%10==1&&(n%100>19||n%100<11)?0:(n%10>=2&&n%10<=9)&&(n%100>19||n%100<11)?1:n%1!=0?2:3);"] = pluralforms(4, plural4_lt);
    s_plural_forms[PF "4" PE "(n%1==0&&n%10==1&&n%100!=11?0:n%1==0&&n%10>=2&&n%10<=4&&(n%100<12||n%100>14)?1:n%1==0&&(n%10==0||(n%10>=5&&n%10<=9)||(n%100>=11&&n%100<=14))?2:3);"] = pluralforms(4, plural4_uk);
    s_plural_forms[PF "4" PE "(n==1?0:(n%10>=2&&n%10<=4)&&(n%100<12||n%100>14)?1:n!=1&&(n%10>=0&&n%10<=1)||(n%10>=5&&n%10<=9)||(n%100>=12&&n%100<=14)?2:3);"] = pluralforms(4, plural4_pl);
    s_plural_forms[PF "4" PE "(n==1&&n%1==0)?0:(n==2&&n%1==0)?1:(n%10==0&&n%1==0&&n>10)?2:3;"] = pluralforms(4, plural4_he);
    s_plural_forms[PF "5" PE "(n==1?0:n==2?1:n<7?2:n<11?3:4)"] = pluralforms(5, plural5_ga);
    s_plural_forms[PF "6" PE "n==0?0:n==1?1:n==2?2:n%100>=3&&n%100<=10?3:n%100>=11?4:5"] = pluralforms(6, plural6_ar);
}

#endif          // defined POTEXT_BRUTE_FORCE_INITIALIZER

/*
 * bfplurals.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
