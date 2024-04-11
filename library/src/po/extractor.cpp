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
 *      https://github.com/laurent22/simple-gettext
 *
 *  Also see the credits for the brute-force string-search algorithm.
 */

/**
 * \file          extractor.cpp
 *
 *      The extractor class is a refactoring of simple-gettext::MoParser.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-03-24
 * \updates       2024-04-11
 * \license       See above.
 *
 *      This class is actually more generally useful than just its usage
 *      in the Potext library.
 */

#include <cstring>                      /* std::strlen() etc.               */
#include <limits>                       /* std::numeric_limits<>::max       */

#include "c_macros.h"                   /* not_nullptr() text macro         */
#include "po/extractor.hpp"             /* po::extractor class              */

namespace po
{

/**
 *  Static functions defined later in this module.
 */

static int brute_force
(
    const std::string & text,
    const std::string & pattern
);

#if defined TEST_BRUTE_FORCE
static bool brute_force_test ();
#endif

/**
 *  Principal constructor
 */

extractor::extractor (const std::string & source) :
    m_data              (source),
    m_data_pos          (0),
    m_swapped_bytes     (false)
{
    // no other code
}

/**
 *  This function looks for a specific string in the binary data.
 *  Currently brute force. See the static function brute_force().
 *  We don't want a recursive solution, because the binary file might be
 *  large enough to induce a stack overflow. Too paranoid?
 *
 * \param target
 *      Provides the string to look for. The whole string length must be
 *      matched.
 *
 * \param start
 *      Provides the offset at which to start looking. Defaults to 0.
 *      Not yet used, but still checked.
 *
 * \return
 *      Returns a null pointer or a pointer to the data.
 */

const char *
extractor::find (const std::string & target, std::size_t start) const
{
    const char * result = nullptr;
    if (valid_offset(start + target.length()))
    {
        int index = brute_force(m_data, target);
        if (index >= 0)
            result = m_data.data() + index;
    }
    return result;
}

/**
 *  Similar to find(), but for offsets.
 */

std::size_t
extractor::find_offset (const std::string & target, std::size_t start) const
{
    std::size_t result = std::numeric_limits<std::size_t>::max();
    const char * bptr = find(target, start);    /* checks the parameters    */
    if (not_nullptr(bptr))
        result = bptr - m_data.data();

    return result;
}

/**
 *  Look for a character in the given range.
 *
 *  The use case for moparser is looking for the EOT character (for context
 *  strings) or additionl NUL characters to get all the plural forms.
 *
 * \param target
 *      The target character (e.g. EOT or NUL).
 *
 * \param start
 *      The offset of the start of the search range.
 *
 * \param len
 *      The length of the search range.
 *
 * \return
 *      Returns a valid offset to the first instance of the character,
 *      if found.  Use the valid_offset() function to test this value.
 */

std::size_t
extractor::find_character
(
    char target, std::size_t start, std::size_t len
) const
{
    std::size_t result = std::numeric_limits<std::size_t>::max();
    std::size_t rangelength = start + len;
    if (valid_offset(rangelength))
    {
        std::string::size_type pos = m_data.find(target, start);
        if (pos != std::string::npos)
        {
            if (pos <= rangelength)
                result = std::size_t(pos);
        }
    }
    return result;
}


/**
 *  This function is akin to find(), but checks for the string at the
 *  current position in the binary data.
 */

bool
extractor::match (const std::string & target, std::size_t sz) const
{
    bool result = true;
    std::size_t i_index = sz == 0 ? m_data_pos : sz ;
    for (auto ch : target)
    {
        if (m_data[i_index] != ch)
        {
            result = false;
            break;
        }
        ++i_index;
    }
    return result;
}

/**
 *  Starting at the given position, this function extracts some data
 *  bytes into a string.
 */

std::string
extractor::get (std::size_t start, std::size_t len) const
{
    return m_data.substr(start, len);
}

/**
 *  Gets character from the start to the first delimiter encountered.
 *
 * \param start
 *      The position at which to start looking.
 *
 * \param delimiters
 *      The delimiters to search for. By default, it is the single character
 *      new-line ("\n").
 */

std::string
extractor::get_delimited
(
    std::size_t start,
    const std::string & delimiters
) const
{
    std::string result;
    std::string::size_type pos = m_data.find_first_of(delimiters, start);
    if (pos != std::string::npos)
    {
        std::size_t count = pos - start;            /* stop at delimiter    */
        result = get(start, count);
    }
    return result;
}

/**
 *  Finds a pattern in a string by comparing the pattern to every substring.
 *  Adapted from:
 *
 *      https://github.com/TheAlgorithms/C-Plus-Plus/tree/master/strings
 *
 * \param text
 *      Any string that might contain the pattern.
 *
 * \param pattern
 *      String that we are searching for.
 *
 * \return
 *      Index where the pattern starts in the text. Or -1 if the
 *      pattern was not found.
 */

static int
brute_force (const std::string & text, const std::string & pattern)
{
    std::size_t pat_length = pattern.length();
    std::size_t txt_length = text.length();
    int index = -1;
    if (pat_length <= txt_length)
    {
        for (std::size_t i = 0; i < txt_length - pat_length + 1; ++i)
        {
            /*
             * Why make a string just to compare it?
             *
             *      std::string s = text.substr(i, pat_length);
             *      if (s == pattern)
             */

            bool match = true;
            std::size_t i_index = i;
            for (auto ch : pattern)
            {
                if (text[i_index] != ch)
                {
                    match = false;
                    break;
                }
                ++i_index;
            }
            if (match)
            {
                index = i;
                break;
            }
        }
    }
    return index;
}

#if defined TEST_BRUTE_FORCE

/**
 *  Set of brute_force() test cases.
 */

static bool
brute_force_test ()
{
    static const std::vector<std::vector<std::string>> s_test_set
    {
        /* text, pattern, expected output   */

        { "a",    "aa", "-1"    },
        { "a",    "a",  "0"     },
        { "ba",   "b",  "0"     },
        { "bba",  "bb", "0"     },
        { "bbca", "c",  "2"     },
        { "ab",   "b",  "1"     }
    };
    bool result = true;
    for (const auto & i : s_test_set)
    {
        int output = brute_force(i[0], i[1]);
        if (std::to_string(output) == i[2])
        {
            printf("success\n");
        }
        else
        {
            printf("failure\n");
            result = false;
        }
    }
    return result;
}

#endif      // TEST_BRUTE_FORCE

/**
 *  Conditionally reverses the bytes in a word.
 */

extractor::word
extractor::swap (word ui)
{
	return m_swapped_bytes ?
        (ui << 24) | ((ui & 0xff00) << 8) | ((ui >> 8) & 0xff00) | (ui >> 24) :
            ui ;
}

}               // namespace po

/*
 * extractor.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
