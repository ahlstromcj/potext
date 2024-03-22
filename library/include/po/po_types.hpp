#if ! defined  POTEXT_PO_PO_TYPES_HPP
#define POTEXT_PO_PO_TYPES_HPP

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
 * \file          po_types.hpp
 *
 *      Universal types in the po namespace for the potext library.
 *
 * \library       potext
 * \author        Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-13
 * \license       See above.
 *
 */

#include <string>
#include <vector>

#include "c_macros.h"

namespace po
{

/**
 * This type is identical to the "::tokenization" type in libs66.
 */

using phraselist = std::vector<std::string>;

/**
 *  https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1408r0.pdf
 *
 *  "Abandon observer_ptr" by Bjarne Stroustrop.
 *
 *  We will still provide observer_ptr<> for our own usage:
 *
 *      "Until we replace that typename with the appropriate concept this will
 *      be the best definition:
 *
 *      -   It allows people to mark pointers as non-owning.
 *      -   It doesn't create interface problems.
 *      -   It interoperates perfectly with T*s.
 *      -   The benefits from an observer_ptr class could be had in the form
 *          of compiler warnings and static analysis tools.
 *
 *      However, I don't propose that because that is still "the tail wagging
 *      the dog" and doesn't keep the simplest and most common case simple.
 *      What we should do instead is to mark pointers that are owners
 *      as owners. We do that using unique_ptr<> and shared_ptr<> except when
 *      we need to pass a pointer through a C-style interface."
 */

template<typename TYPE> using observer_ptr = TYPE *;
template<typename TYPE> using owner_ptr = TYPE *;


}               // namespace po

#endif          // POTEXT_PO_PO_TYPES_HPP

/*
 * po_types.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

