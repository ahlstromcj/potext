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
 * \file          aliases.hpp
 *
 *      Defines language aliases.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-06
 * \updates       2024-02-13
 * \license       See above.
 *
 */

#include <string>
#include <unordered_map>

namespace po
{

/**
 *  Many of those are not useful for us, since we leave encoding to the app,
 *  not to the language. We could/should also match against all language
 *  names, not just aliases from locale.alias. Aliases taken from the
 *  /etc/locale.alias file.
 */

static std::string
resolve_language_alias (const std::string & name)
{
    using aliases = std::unordered_map<std::string, std::string>;

#if defined POTEXT_USE_BRUTE_FORCE_INITIALIZATION

    static aliases s_languages_aliases;
    if (s_languages_aliases.empty())
    {
        s_languages_aliases["bokmal"]           = "nb_NO.ISO-8859-1";
        s_languages_aliases["bokmål"]           = "nb_NO.ISO-8859-1";
        s_languages_aliases["catalan"]          = "ca_ES.ISO-8859-1";
        s_languages_aliases["croatian"]         = "hr_HR.ISO-8859-2";
        s_languages_aliases["czech"]            = "cs_CZ.ISO-8859-2";
        s_languages_aliases["danish"]           = "da_DK.ISO-8859-1";
        s_languages_aliases["dansk"]            = "da_DK.ISO-8859-1";
        s_languages_aliases["deutsch"]          = "de_DE.ISO-8859-1";
        s_languages_aliases["dutch"]            = "nl_NL.ISO-8859-1";
        s_languages_aliases["eesti"]            = "et_EE.ISO-8859-1";
        s_languages_aliases["estonian"]         = "et_EE.ISO-8859-1";
        s_languages_aliases["finnish"]          = "fi_FI.ISO-8859-1";
        s_languages_aliases["français"]         = "fr_FR.ISO-8859-1";
        s_languages_aliases["french"]           = "fr_FR.ISO-8859-1";
        s_languages_aliases["galego"]           = "gl_ES.ISO-8859-1";
        s_languages_aliases["galician"]         = "gl_ES.ISO-8859-1";
        s_languages_aliases["german"]           = "de_DE.ISO-8859-1";
        s_languages_aliases["greek"]            = "el_GR.ISO-8859-7";
        s_languages_aliases["hebrew"]           = "he_IL.ISO-8859-8";
        s_languages_aliases["hrvatski"]         = "hr_HR.ISO-8859-2";
        s_languages_aliases["hungarian"]        = "hu_HU.ISO-8859-2";
        s_languages_aliases["icelandic"]        = "is_IS.ISO-8859-1";
        s_languages_aliases["italian"]          = "it_IT.ISO-8859-1";
        s_languages_aliases["ja_JP"]            = "ja_JP.eucJP";
        s_languages_aliases["ja_JP.ujis"]       = "ja_JP.eucJP";
        s_languages_aliases["japanese"]         = "ja_JP.eucJP";
        s_languages_aliases["japanese.euc"]     = "ja_JP.eucJP";
        s_languages_aliases["japanese.sjis"]    = "ja_JP.SJIS";
        s_languages_aliases["ko_KR"]            = "ko_KR.eucKR";
        s_languages_aliases["korean"]           = "ko_KR.eucKR";
        s_languages_aliases["korean.euc"]       = "ko_KR.eucKR";
        s_languages_aliases["lithuanian"]       = "lt_LT.ISO-8859-13";
        s_languages_aliases["no_NO"]            = "nb_NO.ISO-8859-1";
        s_languages_aliases["no_NO.ISO-8859-1"] = "nb_NO.ISO-8859-1";
        s_languages_aliases["norwegian"]        = "nb_NO.ISO-8859-1";
        s_languages_aliases["nynorsk"]          = "nn_NO.ISO-8859-1";
        s_languages_aliases["polish"]           = "pl_PL.ISO-8859-2";
        s_languages_aliases["portuguese"]       = "pt_PT.ISO-8859-1";
        s_languages_aliases["romanian"]         = "ro_RO.ISO-8859-2";
        s_languages_aliases["russian"]          = "ru_RU.ISO-8859-5";
        s_languages_aliases["slovak"]           = "sk_SK.ISO-8859-2";
        s_languages_aliases["slovene"]          = "sl_SI.ISO-8859-2";
        s_languages_aliases["slovenian"]        = "sl_SI.ISO-8859-2";
        s_languages_aliases["spanish"]          = "es_ES.ISO-8859-1";
        s_languages_aliases["swedish"]          = "sv_SE.ISO-8859-1";
        s_languages_aliases["thai"]             = "th_TH.TIS-620";
        s_languages_aliases["turkish"]          = "tr_TR.ISO-8859-9";
    }

#else

    static aliases s_languages_aliases
    {
        { "bokmal",             "nb_NO.ISO-8859-1"  },
        { "bokmål",             "nb_NO.ISO-8859-1"  },
        { "catalan",            "ca_ES.ISO-8859-1"  },
        { "croatian",           "hr_HR.ISO-8859-2"  },
        { "czech",              "cs_CZ.ISO-8859-2"  },
        { "danish",             "da_DK.ISO-8859-1"  },
        { "dansk",              "da_DK.ISO-8859-1"  },
        { "deutsch",            "de_DE.ISO-8859-1"  },
        { "dutch",              "nl_NL.ISO-8859-1"  },
        { "eesti",              "et_EE.ISO-8859-1"  },
        { "estonian",           "et_EE.ISO-8859-1"  },
        { "finnish",            "fi_FI.ISO-8859-1"  },
        { "français",           "fr_FR.ISO-8859-1"  },
        { "french",             "fr_FR.ISO-8859-1"  },
        { "galego",             "gl_ES.ISO-8859-1"  },
        { "galician",           "gl_ES.ISO-8859-1"  },
        { "german",             "de_DE.ISO-8859-1"  },
        { "greek",              "el_GR.ISO-8859-7"  },
        { "hebrew",             "he_IL.ISO-8859-8"  },
        { "hrvatski",           "hr_HR.ISO-8859-2"  },
        { "hungarian",          "hu_HU.ISO-8859-2"  },
        { "icelandic",          "is_IS.ISO-8859-1"  },
        { "italian",            "it_IT.ISO-8859-1"  },
        { "ja_JP",              "ja_JP.eucJP"       },
        { "ja_JP.ujis",         "ja_JP.eucJP"       },
        { "japanese",           "ja_JP.eucJP"       },
        { "japanese.euc",       "ja_JP.eucJP"       },
        { "japanese.sjis",      "ja_JP.SJIS"        },
        { "ko_KR",              "ko_KR.eucKR"       },
        { "korean",             "ko_KR.eucKR"       },
        { "korean.euc",         "ko_KR.eucKR"       },
        { "lithuanian",         "lt_LT.ISO-8859-13" },
        { "no_NO",              "nb_NO.ISO-8859-1"  },
        { "no_NO.ISO-8859-1",   "nb_NO.ISO-8859-1"  },
        { "norwegian",          "nb_NO.ISO-8859-1"  },
        { "nynorsk",            "nn_NO.ISO-8859-1"  },
        { "polish",             "pl_PL.ISO-8859-2"  },
        { "portuguese",         "pt_PT.ISO-8859-1"  },
        { "romanian",           "ro_RO.ISO-8859-2"  },
        { "russian",            "ru_RU.ISO-8859-5"  },
        { "slovak",             "sk_SK.ISO-8859-2"  },
        { "slovene",            "sl_SI.ISO-8859-2"  },
        { "slovenian",          "sl_SI.ISO-8859-2"  },
        { "spanish",            "es_ES.ISO-8859-1"  },
        { "swedish",            "sv_SE.ISO-8859-1"  },
        { "thai",               "th_TH.TIS-620"     },
        { "turkish",            "tr_TR.ISO-8859-9"  }
    };

#endif

    std::string key;
    std::size_t index = 0;
    key.resize(name.size());
    for (const auto c : name)
        key[index++] = std::tolower(c);

    auto i = s_languages_aliases.find(key);
    if (i != s_languages_aliases.end())
        return i->second;
    else
        return name;
}

}               // namespace po

/*
 * aliases.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
