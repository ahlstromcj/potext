#if ! defined POTEXT_PO_EXTRACTOR_HPP
#define POTEXT_PO_EXTRACTOR_HPP

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
 * \file          extractor.hpp
 *
 *      A class to process binary data.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-03-24
 * \updates       2024-03-27
 * \license       See above.
 *
 */

#include <string>                       /* std::string<> template           */
#include <vector>                       /* std::vector<> template           */

#include "platform_macros.h"            /* PLATFORM_WINDOWS macro           */

/*
 *  A macro short for reinterpret_cast<type *>(x).
 */

#define RECAST_PTR(type, x)     reinterpret_cast<type *>(x)

/*
 *  Keep this class "local" to "po" for now.
 */

namespace po
{

#if defined _MSC_VER
using int32_t = __int32;                /* typedef __int32 int32_t;         */
#else
#include <stdint.h>
#endif

/**
 *  A class to work with binary data stored in a vector of bytes.
 */

class extractor
{

public:

    using byte = unsigned char;
    using word = int32_t;
    using offset = struct
    {
        word o_length;    // length of the string
        word o_offset;    // offset of the string
    };

private:

    /**
     *  A reference to the data vector. We do not want to copy it.
     *  We depend on the caller to be smart.
     */

    const std::string & m_data;

    /**
     *  The current position in m_data.
     */

    std::size_t m_data_pos;

    /**
     *  Indicates if bytes got swapped, based on endian.
     */

    bool m_swapped_bytes;

public:

    extractor () = delete;
    extractor (const std::string & source);
    extractor (const extractor &) = delete;
    extractor (extractor &&) = delete;
    extractor & operator = (const extractor &) = delete;
    ~extractor () = default;

    std::size_t reset_data_pos ()
    {
        m_data_pos = 0;
        return 0;
    }

    std::size_t data_pos () const
    {
        return m_data_pos;
    }

    word swap (word ui);

    void set_swapped_bytes ()
    {
        m_swapped_bytes = true;
    }

    std::size_t skip (std::size_t sz)
    {
        m_data_pos += sz;
        if (m_data_pos >= m_data.size())
            m_data_pos = m_data.size() - 1;

        return m_data_pos;
    }

    char * ptr (std::size_t sz = 0)
    {
        if (sz == 0)
            sz = m_data_pos;

        return const_cast<char *>(m_data.data()) + sz;
    }

    offset * offset_ptr (word offsetvalue)
    {
        return RECAST_PTR
        (
            offset,
            const_cast<char *>(m_data.data()) + std::size_t(offsetvalue)
        );
    }

    bool valid_offset (std::size_t sz) const
    {
        return sz < m_data.size();
    }

    const char * find
    (
        const std::string & target,
        std::size_t start = 0
    ) const;

    std::size_t find_offset
    (
        const std::string & target,
        std::size_t start = 0
    ) const;

    std::size_t find_character
    (
        char target,
        std::size_t start,
        std::size_t len
    ) const;

    bool match (const std::string & target, std::size_t start = 0) const;
    std::string get (std::size_t start, std::size_t len) const;
    std::string get_delimited
    (
        std::size_t start,
        const std::string & delimiters = "\n"
    ) const;

};              // class extractor

}               // namespace po

#endif          // POTEXT_PO_EXTRACTOR_HPP

/*
 * extractor.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
