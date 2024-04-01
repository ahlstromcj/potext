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
 * \file          language.hpp
 *
 *  Macros that depend upon the build platform.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-04
 * \license       See above.
 *
 */

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "c_macros.h"                   /* not_nullptr()                    */
#include "po/po_types.hpp"
#include "po/language.hpp"

/**
 *  Defines po::resolve_language_alias().
 */

#include "po/aliases.hpp"

/**
 *  Defines the languagespec in the po namespace.
 */

#include "po/languagespecs.hpp"         /* struct languagespec and array    */

namespace po
{

/**
 *  Static member functions.
 */

/**
 *  Create a language from language and country code:
 *
 *  Example: languge("de", "DE");
 *
 *  Note that s_languages[] is defined in the languagespe.hpp module.
 *  Also note that array is currently only partly populated to simplify
 *  testing.  Do NOT forget to activate ALL of them!!!
 */

language
language::from_spec
(
    const std::string & lang,
    const std::string & country,
    const std::string & modifier
)
{
    using languagespeclist = std::vector<const languagespec *>;
    using languagespecmap = std::unordered_map<std::string, languagespeclist>;

    static languagespecmap s_language_map;
    if (s_language_map.empty())               /* initialize s_language_map  */
    {
        for (int i = 0; ! s_languages[i].language.empty(); ++i)
            s_language_map[s_languages[i].language].push_back(&s_languages[i]);
    }

    languagespecmap::iterator i = s_language_map.find(lang);
    if (i != s_language_map.end())
    {
        languagespeclist & lst = i->second;
        languagespec tmpspec;
        tmpspec.language = lang;
        tmpspec.country  = country;
        tmpspec.modifier = modifier;
        language tmplang(&tmpspec);

        const languagespec * best_match = nullptr;
        int best_match_score = 0;

        /*
         * Search for the language that best matches the given spec, valuing
         * country more then modifier.
         */

        for (auto j = lst.begin(); j != lst.end(); ++j)
        {
            int score = language::match(language(*j), tmplang);
            if (score > best_match_score)
            {
                best_match = *j;
                best_match_score = score;
            }
        }
        return language(best_match);
    }
    else
    {
        return language();
    }
}

/**
 *  Create a language from language and country code:
 *
 *  Example: language("deutsch");
 *  Example: language("de_DE");
 */

language
language::from_name (const std::string & spec_str)
{
    return from_env(resolve_language_alias(spec_str));
}

/**
 *  Create a language from an environment variable style string
 *  (e.g de_DE.UTF-8@modifier)
 *
 *  Split LANGUAGE_COUNTRY.CODESET@MODIFIER into parts.
 */

language
language::from_env (const std::string & env)
{
    std::string::size_type ln = env.find('_');
    std::string::size_type dt = env.find('.');
    std::string::size_type at = env.find('@');
    std::string country;
    std::string codeset;
    std::string modifier;
    std::string lang = env.substr(0, std::min(std::min(ln, dt), at));
    if (ln != std::string::npos && ln+1 < env.size())           // _
    {
        country = env.substr
        (
            ln + 1,
            std::min(dt, at) == std::string::npos ?
                std::string::npos : std::min(dt, at) - (ln+1)
        );
    }
    if (dt != std::string::npos && dt+1 < env.size())           // .
    {
        codeset = env.substr
        (
            dt + 1,
            at == std::string::npos ?
                std::string::npos : (at - (dt + 1))
        );
    }
    if (at != std::string::npos && ((at + 1) < env.size()))     // @
    {
        modifier = env.substr(at + 1);
    }
    return from_spec(lang, country, modifier);
}

/**
 *  Compares two languages, returns 0 on missmatch and a score
 *  between 1 and 9 on match, the higher the score the better the
 *  match.
 */

int
language::match (const language & lhs, const language & rhs)
{
    if (lhs.get_language() != rhs.get_language())
    {
        return 0;
    }
    else
    {
        static int match_tbl[3][3] =    /* modifier match, wildchard, miss  */
        {
            { 9, 8, 5 },                // country match
            { 7, 6, 3 },                // country wildcard
            { 4, 2, 1 },                // country miss
        };
        int c;
        if (lhs.get_country() == rhs.get_country())
            c = 0;
        else if (lhs.get_country().empty() || rhs.get_country().empty())
            c = 1;
        else
            c = 2;

        int m;
        if (lhs.get_modifier() == rhs.get_modifier())
            m = 0;
        else if (lhs.get_modifier().empty() || rhs.get_modifier().empty())
            m = 1;
        else
            m = 2;

        return match_tbl[c][m];
    }
}

/**
 *  Non-static member functions.
 */

language::language (const languagespec * langspec) : m_language_spec (langspec)
{
    // no other code
}

language::language () : m_language_spec ()
{
    // no other code
}

/**
 *  Returns either an existing languagespec or an empty dummy.
 */

const languagespec &
language::spec () const
{
    static languagespec s_dummy;
    return not_nullptr(m_language_spec) ? *m_language_spec : s_dummy ;
}

/**
 *  These are pointer comparisons.
 */

bool
language::operator == (const language & rhs) const
{
    return m_language_spec == rhs.m_language_spec;
}

bool
language::operator != (const language & rhs) const
{
    return m_language_spec != rhs.m_language_spec;
}

/**
 *  Returns the language code (i.e. de, en, fr). If the dummy spec is
 *  returned, the string is empty.
 */

std::string
language::get_language () const
{
    return spec().language;
}

/**
 *  Returns the country code (i.e. DE, AT, US). If the dummy spec is
 *  returned, the string is empty.
 */

std::string
language::get_country () const
{
    return spec().country;
}

/**
 *  Returns the modifier of the language (i.e. latn or Latn for Serbian with
 *  non-cyrilic characters). If the dummy spec is returned, the string is
 *  empty.
 */

std::string
language::get_modifier () const
{
    return spec().modifier;
}

/**
 *  Returns the human readable name of the language, such as "French". If the
 *  dummy spec is returned, the string is empty.
 */

std::string
language::get_name () const
{
    return spec().name;
}

/**
 *  Returns the human readable name of the language in the language itself.
 *  If unavailable, then get_name() is called.
 */

std::string
language::get_localized_name () const
{
    if (m_language_spec && ! m_language_spec->name_localized.empty())
        return m_language_spec->name_localized;
    else
        return get_name();
}

/**
 *  Returns the language as string in the form of an environment variable:
 *  {language}_{country}@{modifier}.
 */

std::string
language::to_string () const
{
    if (m_language_spec)
    {
        std::string var = m_language_spec->language;
        if (! m_language_spec->country.empty())
        {
            var += "_";
            var += m_language_spec->country;
        }
        if (! m_language_spec->modifier.empty())
        {
            var += "@";
            var += m_language_spec->modifier;
        }
        return var;
    }
    else
        return std::string("");
}

}           // namespace po

/*
 * language.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
