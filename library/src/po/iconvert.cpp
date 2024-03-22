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
 * \file          iconvert.hpp
 *
 *      A refactoring of the tinygettext::IConv class.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-16
 * \license       See above.
 *
 *  To do: Handle the following:
 *
 *      If the string //IGNORE is appended to to-encoding, characters that
 *      cannot be converted are discarded and an error is printed after
 *      conversion.
 *
 *      If the string //TRANSLIT is appended to to-encoding, characters that
 *      cannot be represented in the target character set, it can be
 *      approximated through one or several similar looking characters.
 */

#include <cctype>                       /* std::toupper()                   */
#include <cstring>                      /* std::strerror(), std::memset()   */
#include <cerrno>                       /* errno, like errno.h              */
#include <sstream>                      /* std::stringstream                */
#include <stdexcept>                    /* std::runtime_error()             */

#include "c_macros.h"                   /* is_nullptr() macro               */
#include "po/iconvert.hpp"              /* po::iconvert (iconv) class       */
#include "po/logstream.hpp"             /* po::logstream::error() etc.      */

#if defined USE_FLUXBOX_RECODE_FUNCTION
#include <vector>                       /* std::vector for output array     */
#endif

/**
 *  We cannot enable translation in this module, because it leads to
 *  recursion and a stack overflow. Oh well, it was worth a try.
 *  Can still grab messages for a dictionary, if desired.
 */

#if ! defined PO_HAVE_GETTEXT_RECURSIVE
#define _(str)      str
#endif

namespace po
{

/**
 *  Free static functions in the po namespace.
 */

static iconv_t iconv_open
(
    const std::string & tocode, const std::string & fromcode
);
static int iconv_close (iconv_t cdescriptor);

#if ! defined USE_FLUXBOX_RECODE_FUNCTION

static size_t iconv
(
    iconv_t cdescriptor,
    const char ** inbuf,
    size_t * inbytesleft,
    char ** outbuf,
    size_t * outbytesleft
);

#endif

/**
 *  Internal helper function.
 */

static std::size_t
iconv_error ()
{
    return static_cast<size_t>(-1);
}

/**
 *  Constructors and destructor.
 */

iconvert::iconvert (const std::string & filename) :
    m_filename              (filename),
    m_to_charset            (),
    m_from_charset          (),
    m_conversion_descriptor (nullptr)
{
    // no code
}

iconvert::iconvert
(
    const std::string & from_charset,
    const std::string & to_charset
) :
    m_filename              (),
    m_to_charset            (),
    m_from_charset          (),
    m_conversion_descriptor (nullptr)
{
    set_charsets(from_charset, to_charset); /* opens conversion descriptor  */
}

iconvert::~iconvert ()
{
    if (m_conversion_descriptor)
        po::iconv_close(m_conversion_descriptor);
}

/**
 *  Closes the conversion descriptor if it is open; makes sure the
 *  charset strings are upper-case (and characters); and then it
 *  opens the conversion descriptor.
 *
 *  We have changed this function to return a bool and use logstream
 *  instead of throwing an error, for better control.
 */

bool
iconvert::set_charsets
(
    const std::string & from_charset,
    const std::string & to_charset
)
{
    bool result = true;
    if (m_conversion_descriptor)
        po::iconv_close(m_conversion_descriptor);

    m_from_charset = from_charset;
    m_to_charset = to_charset;
    for (auto & ch : m_to_charset)
        ch = static_cast<char>(std::toupper(ch));

    for (auto & ch : m_from_charset)
        ch = static_cast<char>(std::toupper(ch));

    if (m_to_charset == m_from_charset)
    {
        m_conversion_descriptor = nullptr;
    }
    else
    {
        m_conversion_descriptor = po::iconv_open
        (
            m_to_charset.c_str(), m_from_charset.c_str()
        );
        if (m_conversion_descriptor == reinterpret_cast<iconv_t>(-1))
        {
            result = false;
            if (errno == EINVAL)
            {
                po::logstream::error()
                    << "set_charsets() " << _("failed") << ": "
                    << _("conversion from") << " '"
                    << from_charset
                    << "' " << _("to") << " '" << to_charset
                    << "' " << _("unavailable")
                ;
            }
            else
            {
                po::logstream::error()
                    << "set_charsets() " << _("failed") << ": "
                    << std::strerror(errno)
                    ;
            }
        }
    }
    return result;
}

/**
 *  Convert a string from encoding to another.
 *
 *  Worst case scenario: ASCII -> UTF-32, so we allow up to four times
 *  the bytes.
 *
 *  We try to avoid to much copying around, so we write directly
 *  into a std::string. No more! One, this is crazy. Two, this is done
 *  only at parsing, so it does not affect translation speed.
 *
 *      std::vector<char> outbuf{outbytesleft, 'X'};
 */

#if defined USE_FLUXBOX_RECODE_FUNCTION

std::string
iconvert::convert (const std::string & text)
{
    if (! m_conversion_descriptor)
    {
        return text;
    }
    else
    {
        return recode(m_conversion_descriptor, text);
    }
}

#else           // ! defined USE_FLUXBOX_RECODE_FUNCTION

std::string
iconvert::convert (const std::string & text)
{
    if (! m_conversion_descriptor)
    {
        return text;
    }
    else
    {
        std::string result;
        size_t inbytesleft = text.size();
        const size_t outbytes = 4 * inbytesleft;
        size_t outbytesleft = outbytes;
        const char * inbuf = text.data();               /* &text[0]         */
        char * outbuf = new (std::nothrow) char [outbytes];
        if (is_nullptr(outbuf))
            return text;

        const char * outbuf_original = outbuf;
        (void) std::memset(outbuf, 'x', outbytes);
        size_t ret = po::iconv                          /* wraps ::iconv(3) */
        (
            m_conversion_descriptor,
            &inbuf, &inbytesleft,
            &outbuf, &outbytesleft
        );
        bool success = ret != iconv_error();
        if (success)
        {
            size_t sz = outbytes - outbytesleft;
            (void) result.assign(outbuf_original, sz - 1);
        }
        else
        {
            result.clear();
            if (errno == EILSEQ || errno == EINVAL)
            {
                /*
                 * Invalid multibyte sequence.  FIXME: Could try to skip the
                 * invalid byte and continue. For now we reset the state.
                 */

                std::string location = m_filename.empty() ?
                    "convert()" : m_filename ;

                po::iconv
                (
                    m_conversion_descriptor, nullptr, nullptr,
                    nullptr, nullptr
                );
                po::logstream::error()
                    << _("error") << ": " << location << ":\n"
                    << _("invalid multibyte sequence in") << ": \""
                    << text << "\"" << std::endl
                    ;
            }
            else if (errno == E2BIG)
            {
                po::logstream::error()
                    << "E2BIG: " << _("Output buffer too small")
                    << "; " << _("programmer error")
                    << std::endl
                    ;
            }
            else if (errno == EBADF)
            {
                po::logstream::error()
                    << "EBADF: " << _("Output buffer too small")
                    << "; " << _("programmer error")
                    << std::endl
                    ;
            }
            else
            {
                po::logstream::error()
                    << _("Unspecified error") << "; "
                    << _("programmer error")
                    << std::endl
                    ;
            }
        }
        return result;
    }
}

#endif          // defined USE_FLUXBOX_RECODE_FUNCTION

/**
 *  Free functions in the po namespace.
 */

/**
 *  The values permitted for fromcode and tocode and the supported
 *  combinations are system-dependent. For the GNU C library, the
 *  permitted values are  listed by the "iconv --list" command, and
 *  all combinations of the listed values are supported.
 *  Furthermore the GNU C library and the  GNU libiconv library
 *  support the two additional suffixes.
 */

static iconv_t
iconv_open (const std::string & stdtocode, const std::string & stdfromcode)
{
#if defined POTEXT_PO_WITH_SDL
    return SDL_iconv_open(tocode.c_str(), fromcode.c_str());
#else
    return ::iconv_open(stdtocode.c_str(), stdfromcode.c_str());
#endif
}

/**
 *  A straightforward wrapper for closing the conversion descriptor.
 */

static int
iconv_close (iconv_t cd)
{
#if defined POTEXT_PO_WITH_SDL
    return SDL_iconv_close(cd);
#else
    return ::iconv_close(cd);
#endif
}

/**
 *  Converts a sequence of characters in one character encoding to another
 *  character encoding.
 *
 *  The processing of the C function ::iconv() is complex and we must take
 *  care to avoid stack-smashing.
 *
 * \param cd
 *      Provides the SDL or normal iconv_t conversion descriptor.
 *      It is created by a call to ::iconv_open().
 *
 * \param [inout] inbuf
 *      The address of a variable that points to the first character of the
 *      input buffer. It is incremented as each (multi-byte) character is
 *      converted to output.
 *
 * \param [inout] inbytesleft
 *      Input and output value for the input buffer, decremented as each input
 *      character sequence is processed.
 *
 * \param [inout] outbuf
 *      The address of a variable that points to the first character of the
 *      output buffer. It is incremented as each character is converted
 *      to output.
 *
 * \param [inout] outbytesleft
 *      Input and output value for the input buffer. It is decrements as
 *      each character eats up its space in the output buffer.
 *
 * \return
 *      Returns the number of characters converted in a non-reversible way.
 *      If 0, the conversion was reversable. If -1, the conversion failed
 *      in some way an ::errno is set. See man page iconv(3) for details.
 */

#if defined USE_FLUXBOX_RECODE_FUNCTION

/**
 *  Recodes the text from one encoding to another assuming cd is correct. This
 *  code is adapted from Fluxbox, with some changes to improve it.
 *
 *  In the "C" locale, this will strip any high-bit characters
 *  because C means the 7-bit ASCII charset. If you don't want this
 *  then you need to set your locale to something UTF-8 or something
 *  ISO8859-1.
 *
 * \param cd
 *      Provides the iconv type code descriptor.
 *
 * \param in
 *      The text to be converted, *not* necessarily NULL-terminated.
 *
 * \return
 *      Returns the recoded string, or an empty string upon failure.
*/

std::string
iconvert::recode (iconv_t cd, const std::string & in)
{
    static const iconv_t ICONV_NULL = reinterpret_cast<iconv_t>(-1);
    std::string result;
    if (! in.empty())
    {
        if (cd == ICONV_NULL)               /* we cannot do conversions     */
        {
            result = in;
        }
        else
        {
            size_t insize = in.size();
            size_t outsize = 4 * insize;    /* allow for 1-byte to 4-byte   */
            size_t inbytesleft = insize;
            size_t outbytesleft = outsize;
            std::vector<char> out(outsize);
            const char * in_ptr = in.data();
            char * out_ptr = &out[0];
            size_t rc = iconv_error();
            bool again = true;
            while (again)
            {
                again = false;
#if defined POTEXT_PO_WITH_SDL
                rc = SDL_iconv
                (
                    cd, in.data(), &inbytesleft, &out_ptr, &outbytesleft
                );
#else
                rc = iconv
                (
                    cd, detail::constptrhack(&in_ptr),
                    &inbytesleft, &out_ptr, &outbytesleft
                );
#endif
                if (rc == iconv_error())
                {
                    std::string location = m_filename.empty() ?
                        "convert" : m_filename ;

                    size_t errindex = insize - inbytesleft;
                    switch (errno)
                    {

                    /*
                     * Invalid multibyte sequence.  Skip the invalid byte
                     * and continue.
                     */

                    case EILSEQ:                /* try skipping a byte      */

                        ++in_ptr;
                        --inbytesleft;
                        again = true;
                        po::logstream::error()
                            << _("error") << ": " << location << ":\n"
                            << _("invalid multibyte sequence in") << ": \""
                            << in << "\" @" << errindex << std::endl
                            ;
                        break;

                    case EINVAL:

                        ++in_ptr;
                        --inbytesleft;
                        again = true;
                        po::logstream::error()
                            << _("error") << ": " << location << ":\n"
                            << _("incomplete multibyte sequence in") << ": \""
                            << in << "\" @" << errindex << std::endl
                            ;
                        break;

                    case E2BIG:                 /* need more space!         */

                        outsize += insize;
                        out.resize(outsize);
                        if (out.capacity() != outsize)
                            again = true;

                        outbytesleft += insize;
                        out_ptr = (&out[0] + outsize) - outbytesleft;
                        po::logstream::error()
                            << "E2BIG: " << _("Output buffer too small")
                            << std::endl
                            ;
                        break;

                    default:                    /* something else broke     */

                        perror("iconv");
                        po::logstream::error()
                            << _("iconv() error") << "; "
                            << std::strerror(errno)
                            << std::endl
                            ;
                        break;
                    }
                }
            }
            result.append(&out[0], outsize - outbytesleft);
            iconv(cd, NULL, NULL, NULL, NULL);  /* reset conversion desc.   */
        }
    }
    return result;
}

#else           // ! defined USE_FLUXBOX_RECODE_FUNCTION

static size_t
iconv
(
    iconv_t cd,
    const char ** inbuf,
    size_t * inbytesleft,               /* return value for input handling  */
    char ** outbuf,
    size_t * outbytesleft               /* return value for output handling */
)
{
#if defined POTEXT_PO_WITH_SDL
    return SDL_iconv(cd, inbuf, inbytesleft, outbuf, outbytesleft);
#else
    return ::iconv
    (
        cd, detail::constptrhack(inbuf),
        inbytesleft, outbuf, outbytesleft
    );
#endif
}

#endif          // defined USE_FLUXBOX_RECODE_FUNCTION

}               // namespace po

/*
 * iconvert.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
