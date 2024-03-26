#if ! defined  POTEXT_PO_ICONVERT_HPP
#define POTEXT_PO_ICONVERT_HPP

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
 *      Macros that depend upon the build platform.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-26
 * \license       See above.
 *
 */

#include <string>

#include "po_build_macros.h"            /* the POTEST_PO_WITH_SDL macro     */

#if defined POTEXT_PO_WITH_SDL
#include "SDL.h"
#else
#include <iconv.h>
#endif

#define USE_FLUXBOX_RECODE_FUNCTION

namespace po
{

#if ! defined POTEXT_PO_WITH_SDL

namespace detail
{

struct constptrhack
{
    const char ** ptr;

    constptrhack (char ** ptr_) :
        ptr (const_cast<const char **>(ptr_))
    {
        // no code
    }

    constptrhack (const char ** ptr_) :
        ptr(ptr_)
    {
        // no code
    }

    operator const char ** () const
    {
        return ptr;
    }

    operator char ** () const
    {
        return const_cast<char**>(ptr);
    }

};              // struct constptrhack

}               // namespace detail

#endif          // ! defined POTEXT_PO_WITH_SDL

/**
 *  The iconv_t data type.
 */

#if defined POTEXT_PO_WITH_SDL

using iconv_t = ::SDL_iconv_t;

#else

using iconv_t = ::iconv_t;

#endif

class iconvert
{
    friend class poparser;

private:

    /**
     *  Useful in reporting errors, especially in test scripts.
     */

    std::string m_filename;

    /**
     *  The destination character encoding.
     */

    std::string m_to_charset;

    /**
     *  The source character encoding.
     */

    std::string m_from_charset;

    /**
     *  Conversion descriptor.
     */

    iconv_t m_conversion_descriptor;

public:

    iconvert (const std::string & filename = "");
    iconvert (const std::string & fromcode, const std::string & tocode);
    iconvert (const iconvert &);
    iconvert (iconvert &&) = default;
    iconvert & operator = (const iconvert &);
    ~iconvert ();

    std::string convert (const std::string & text);

    bool set_charsets
    (
        const std::string & fromcode,
        const std::string & tocode
    );

private:

#if defined USE_FLUXBOX_RECODE_FUNCTION
    std::string recode (iconv_t cd, const std::string & in);
#endif

};

}               // namespace po

#endif          // POTEXT_PO_ICONVERT_HPP

/*
 * iconvert.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
