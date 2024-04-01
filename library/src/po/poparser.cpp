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
 * \file          poparser.cpp
 *
 *      The poparser class is a refactoring of tinygettext::POParser.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-31
 * \license       See above.
 *
 */

#include <cstring>                      /* std::strlen() etc.               */
#include <iostream>                     /* std::istream, std::ostream       */

#include "po/dictionary.hpp"            /* po::dictionary class             */
#include "po/logstream.hpp"             /* po::logstream log-access funcs   */
#include "po/pluralforms.hpp"           /* po::pluralforms class            */
#include "po/poparser.hpp"              /* po::poparser class               */

/**
 *  We cannot enable translation in this module, because it leads to recursion
 *  and a stack overflow. Oh well, it was worth a try.  Can still grab
 *  messages for a dictionary, if desired.
 */

#if ! defined PO_HAVE_GETTEXT_RECURSIVE
#define _(str)      str
#endif

namespace po
{

/**
 *  An internal flag to handle present-but-empty "msgctxt" tags.
 */

#define MSGCTXT_EMPTY_FLAG      "-"

/**
 *  Constructor.
 */

poparser::poparser
(
    const std::string & filename,
    std::istream & in,
    dictionary & dic,
    bool usefuzzy
) :
    pomoparserbase  (filename, in, dic, usefuzzy),
    m_eof           (false),
    m_big5          (false),
    m_line_number   (0),
    m_current_line  ()                /* accessor line() */
{
    // no code
}

/**
 *  Static function to parse a given po file.
 */

bool
poparser::parse_po_file
(
    const std::string & filename,
    std::istream & in,
    dictionary & dic
)
{
    poparser parser(filename, in, dic);
    return parser.parse();
}

void
poparser::next_line ()
{
    ++m_line_number;
    if (! std::getline(in_stream(), m_current_line))
        m_eof = true;
}

static unsigned char
uchar_cast (char c)
{
    return static_cast<unsigned char>(c);
}

/**
 *  Examines the current line, skipping the specified number of characters.
 *  If that number is too much, or the character is not the expected
 *  double-quote character, an error is thrown.
 */

void
poparser::get_string_line (std::ostringstream & out, std::size_t skip)
{
    std::size_t linesize = line().size();
    if ((skip + 1) >= linesize)
        error(_("1. Unexpected end of line"), m_line_number);

    if (line(skip) != '"')
        error(_("Expected start of string"), m_line_number);

    std::string::size_type i;
    for (i = skip + 1; line(i) != '"'; ++i)
    {
        char c = line(i);
        unsigned char uc = uchar_cast(c);
        if (m_big5 && uc >= 0x81 && uc <= 0xfe)
        {
            out << c;
            ++i;
            if (i >= line().size())
                error(_("Invalid Big5 encoding"), m_line_number);

            out << line(i);
        }
        else if (i >= line().size())
        {
            error(_("2. Unexpected end of line"));
        }
        else if (c == '\\')
        {
            ++i;
            if (i >= line().size())
                error(_("Unexpected end of line in handling"), m_line_number);

            switch (c)
            {
            case 'a':  out << '\a'; break;
            case 'b':  out << '\b'; break;
            case 'v':  out << '\v'; break;
            case 'n':  out << '\n'; break;
            case 't':  out << '\t'; break;
            case 'r':  out << '\r'; break;
            case '"':  out << '"';  break;
            case '\\': out << '\\'; break;

            default:

                std::ostringstream err;
                err << _("Unhandled escape") << " '\\" << line(i) << "'";
                warning(err.str());
                out << line(i-1) << line(i);
                break;
            }
        }
        else
            out << c;
    }

    /*
     * Process trailing garbage in line and warn if there is any.
     */

    for (i = i + 1; i < line().size(); ++i)
    {
        if (! std::isspace(line(i)))
        {
            warning(_("Unexpected garbage after string ignored"));
            break;
        }
    }
}

/**
 *  Skips into the current line to get the string after the first
 *  double-quote.
 *
 *  Then it goes to the next line and gets another string.
 */

std::string
poparser::get_string (std::size_t skip)
{
    if ((skip + 1) >= line().size())
        error(_("3. Unexpected end of line"), m_line_number);

    std::ostringstream ssout;
    if (line(skip) == ' ' && line(skip + 1) == '"')
    {
        get_string_line(ssout, skip + 1);
    }
    else
    {
        if (pedantic())
            warning(_("A single space must separate keyword and string"));

        for ( ; ; ++skip)
        {
            if (skip >= line().size())
            {
                error(_("4. Unexpected end of line"), m_line_number);
            }
            else if (line(skip) == '"')                 /* same as '\"'! */
            {
                get_string_line(ssout, skip);
                break;
            }
            else if (! std::isspace(line(skip)))
            {
                error(_("Tagged string must start with quote"), m_line_number);
            }
            else
            {
                // skip space
            }
        }
    }

next:

    next_line();
    int i = 0;
    for (auto ch : line())
    {
        if (ch == '"')
        {
            if (i == 1)
            {
                if (pedantic())
                    warning(_("leading whitespace before string"));
            }
            get_string_line(ssout,  i);
            goto next;
        }
        else if (std::isspace(ch))
        {
            // skip
        }
        else
            break;

        ++i;
    }
    return ssout.str();
}

/**
 * \param header
 *      This string consist of all of the header lines crammed together,
 *      separated by backslashes:
 *
\verbatim
    "Project-Id-Version: PACKAGE VERSION\\Report-Msgid-Bugs-To: \\POT-Creation-
    Date: 2009-01-30 08:01+0100\\PO-Revision-Date: 2009-01-30 08:39 +0100\\
    Last-Translator: FULL NAME <EMAIL@ADDRESS>\\Language-Team: LANGUAGE <LL@li.
    org>\\MIME-Version: 1.0\\Content-Type: text/plain; charset=UTF-8\\
    Content-Transfer-Encoding: 8bit\\"
\endverbatim
 */

bool
poparser::parse_header (const std::string & header)
{
    std::string from_charset;
    std::string::size_type contentpos = header.find
    (
        "Content-Type: text/plain; charset="
    );
    if (contentpos != std::string::npos)
    {
        contentpos = header.find_first_of("=", contentpos + 1);
        if (contentpos != std::string::npos)
        {
            std::string::size_type slashpos = header.find_first_of
            (
                "\\", contentpos + 1
            );
            if (slashpos != std::string::npos)
            {
                std::string::size_type count = slashpos - contentpos;
                from_charset = header.substr(contentpos, count);
            }
        }
    }
    else
        warning(_("No Content-Type header detected"));

    std::string::size_type pluralpos = header.find("Plural-Forms");
    if (pluralpos != std::string::npos)
    {
        std::string::size_type slashpos = header.find_first_of
        (
            "\\", pluralpos + 1                 /* ends with "\n" string    */
        );
        if (slashpos != std::string::npos)
        {
            std::string::size_type count = slashpos - pluralpos;
            std::string plurals = header.substr(pluralpos, count);
            pluralforms plural_forms = pluralforms::from_string(plurals);
            if (! plural_forms)
            {
                warning(_("Unknown Plural-Forms"));
                /*
                 * TODO: m_plural_forms = "nplurals=1; plural=0";
                 */
            }
            else
            {
                if (! dict().get_plural_forms())
                {
                    dict().set_plural_forms(plural_forms);
                }
                else
                {
                    if (dict().get_plural_forms() != plural_forms)
                    {
                        warning
                        (
                            _("Plural-Forms mismatch between .po "
                            "file and dictionary")
                        );
                    }
                }
            }
        }
    }
    if (from_charset.empty() || from_charset == "CHARSET")
    {
        if (pedantic())
            warning(_("Charset not found for .po; fallback to UTF-8"));

        from_charset = "UTF-8";
    }
    else if (from_charset == "BIG5")
    {
        m_big5 = true;
    }
    return converter().set_charsets(from_charset, dict().get_charset());
}

bool
poparser::is_empty_line ()
{
    if (line().empty())
    {
        return true;
    }
    else if (line(0) == '#')
    {
        return                          /* handle comments as empty lines   */
        (
            line().size() == 1 ||
            (line().size() >= 2 && std::isspace(line(1)))
        );
    }
    else
    {
        for (auto ch : line())
        {
            if (! std::isspace(ch))
                return false;
        }
    }
    return true;
}

/**
 *  Compares the start of the current line to the prefixstr parameter.
 */

bool
poparser::prefix_match (const char * prefixstr)
{
    std::size_t sz = std::strlen(prefixstr);
    return line().compare(0, sz, prefixstr) == 0;
}

/**
 * UTF-8:
 *
 *      Skip UTF-8 intro that some text editors produce. See the
 *      http://en.wikipedia.org/wiki/Byte-order_mark page.
 *
 *          "The BOM is a particular usage of the special Unicode character
 *          code, U+FEFF ZERO WIDTH NO-BREAK SPACE...  The UTF-8
 *          representation of the BOM is the (hexadecimal) byte sequence
 *          EF BB BF."
 *
 * Fuzzy:
 *
 *      Encountering "#, fuzzy" turns on fuzzy processing permanently.
 */

static char
hex_cast (int ch)
{
    return static_cast<char>(ch);
}

static bool
check_BOM (const std::string & str)
{
    return
    (
        str.size() >= 3 &&                      // check for BOM
        str[0] == hex_cast(0xef) &&             // &iuml;   ï
        str[1] == hex_cast(0xbb) &&             // &raquo;  »
        str[2] == hex_cast(0xbf)                // &iquest; ¿
    );
}

/**
 *  Problem: How to handle 'msgctxt ""' in a .po file. From this site:
 *
 *  https://www.gnu.org/software/gettext/manual/html_node/PO-Files.html
 *
 *      "The context serves to disambiguate messages with the same
 *      untranslated-string. It is possible to have several entries with the
 *      same untranslated-string in a PO file, provided that they each have a
 *      different context. Note that an empty context string and an absent
 *      msgctxt line do not mean the same thing."
 *
 *  Also:
 *
 *      "For a PO file to be valid, no two entries without msgctxt may have
 *      the same untranslated-string or untranslated-string-singular.
 *      Similarly, no two entries may have the same msgctxt and the same
 *      untranslated-string or untranslated-string-singular."
 *
 *  To handle the addition of a present-but-empty "msgctxt" tag, we use
 *  a special tag, "-" (MSGCTXT_EMPTY_FLAG) to flag this; it gets converted
 *  to an empty context.
 */

bool
poparser::parse ()
{
    bool result = true;
    next_line();
    if (check_BOM(line()))
        m_current_line = line().substr(3);  // tricky?

    while (! m_eof)
    {
        try
        {
            bool fuzzy = false;
            bool has_msgctxt = false;
            std::string msgctxt;
            std::string msgid;
            while (prefix_match("#"))           /* skip top part, set fuzzy */
            {
                if (line().size() >= 2 && line(1) == ',')
                {
                    if (line().find("fuzzy", 2) != std::string::npos)
                        fuzzy = true;
                }
                next_line();
            }
            if (! is_empty_line())
            {
                bool got_a_tag = false;
                if (prefix_match("msgctxt"))    /* gets next line (msgstr)  */
                {
                    msgctxt = get_string(7);
                    has_msgctxt = true;
                    got_a_tag = true;
                }
                if (prefix_match("msgid"))
                {
                    msgid = get_string(5);      /* gets next line plural    */
                    got_a_tag = true;
                }
                if (prefix_match("msgid_plural"))
                {
                    if (has_msgctxt && msgctxt.empty())
                        msgctxt = MSGCTXT_EMPTY_FLAG;

                    (void) get_msgid_plural(fuzzy, msgctxt, msgid);
                    got_a_tag = true;
                }
                if (prefix_match("msgstr"))
                {
                    if (has_msgctxt && msgctxt.empty())
                        msgctxt = MSGCTXT_EMPTY_FLAG;

                    (void) get_msgstr(fuzzy, msgctxt, msgid);
                    got_a_tag = true;
                }
                if (! got_a_tag)
                    error(_("Expected a msg tag"), m_line_number);
            }
            next_line();
        }
        catch (const internal_parser_error &)
        {
            result = false;
        }
    }
    return result;
}

/**
 *  Message context and message ID plus plural-form processing.
 */

bool
poparser::get_msgid_plural
(
    bool fuzzy,
    const std::string & msgctxt,
    const std::string & msgid
)
{
    bool result = true;
    std::string msgid_plural = get_string(12);
    phraselist msglist;
    bool saw_nonempty_msgstr = false;

next:

    if (is_empty_line())                    /* blank line ends the entry    */
    {
        if (msglist.empty())
          error(_("Expected 'msgstr[0 to 9]'"), m_line_number);
    }
    else if
    (
        prefix_match("msgstr[") && line().size() > 8 &&
        std::isdigit(line(7)) && line(8) == ']'
    )
    {
        unsigned number = unsigned(line(7) - '0');
        std::string msgstr = get_string(9);
        if (! msgstr.empty())
            saw_nonempty_msgstr = true;

        /*
         *  Can leave some empty strings in this vector.
         */

        if (number >= msglist.size())
            msglist.resize(number + 1);

        msglist[number] = converter().convert(msgstr);
#if defined PLATFORM_DEBUG_TMI
        if (msglist[number].empty())
            std::cout << "ERROR in " << m_filename << std::endl;
#endif
        goto next;
    }
    else
        error(_("Expected 'msgstr[n]' entry"), m_line_number);

    if (! is_empty_line())
        error(_("Expected 'msgstr[n]' entry or empty line"), m_line_number);

    if (saw_nonempty_msgstr)
    {
        if (use_fuzzy() || ! fuzzy)
        {
            if (! dict().get_plural_forms())
            {
                warning("msgstr[n] found, but no Plural-Form");
            }
            else
            {
                if (msglist.size() != dict().get_plural_forms().get_nplural())
                    warning(_("msgstr[n] count != Plural-Forms.nplural"));
            }
            if (msgctxt.empty())
            {
                (void) dict().add(msgid, msgid_plural, msglist);
            }
            else
            {
                std::string ctxt;
                if (msgctxt != MSGCTXT_EMPTY_FLAG)
                    ctxt = msgctxt;

                (void) dict().add(ctxt, msgid, msgid_plural, msglist);
            }
        }
#if defined PLATFORM_DEBUG_TMI
        std::cout
            << (fuzzy ? _("fuzzy") : _("not fuzzy")) << "\n" << "msgid \""
            << msgid << "\"\n" << "msgid_plural \"" << msgid_plural << "\""
            << std::endl
            ;

        for
        (
            phraselist::size_type i = 0;
            i < msglist.size(); ++i
        )
        {
            std::string msgconversion = converter().convert(msglist[i]);
            std::cout
                << "msgstr[" << i << "] \"" << msgconvertsion
                << "\""
                << std::endl
                ;
#if defined PLATFORM_DEBUG_TMI
            if (msgconversion.empty())
            {
                std::cout << "ERROR in " << m_filename << std::endl;
            }
#endif
        }
        std::cout << std::endl;
#endif
    }
    return result;
}

/**
 *  Message context and message ID.
 */

bool
poparser::get_msgstr
(
    bool fuzzy,
    const std::string & msgctxt,
    const std::string & msgid
)
{
    bool result = true;
    std::string msgstr = get_string(6);
    if (msgid.empty())
    {
        result = parse_header(msgstr);
    }
    else if (msgstr.empty())
    {
        result = false;
    }
    else
    {
        if (use_fuzzy() || ! fuzzy)
        {
            if (msgctxt.empty())
            {
                (void) dict().add(msgid, converter().convert(msgstr));
            }
            else
            {
                std::string ctxt;
                if (msgctxt != MSGCTXT_EMPTY_FLAG)
                    ctxt = msgctxt;

                (void) dict().add(ctxt, msgid, converter().convert(msgstr));
            }
        }
#if defined PLATFORM_DEBUG_TMI
        std::cout
            << (fuzzy ? _("fuzzy") : _("not fuzzy")) << "\n"
            << "msgid \"" << msgid << "\"\n"
            << "msgstr \"" << converter().convert(msgstr)
            << "\"" << std::endl
            ;
#endif
    }
    return result;
}

}               // namespace po

/*
 * poparser.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
