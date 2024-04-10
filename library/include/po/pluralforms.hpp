#if ! defined POTEXT_PO_PLURALFORMS_HPP
#define POTEXT_PO_PLURALFORMS_HPP

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
 * \file          pluralforms.hpp
 *
 *      Provides a class for working with plural-forms in translations.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-04-10
 * \license       See above.
 *
 */

#include <string>
#include <unordered_map>                /* std::unordered_map<> template    */

#include "po_build_macros.h"            /* POTEXT_BRUTE_FORCE_INITIALIZER   */

namespace po
{

/**
 *  Provides handling of plurals in potext. This class provides one "plurals"
 *  function, representing the "Plural-Forms" specified in the .po
 *  header.
 *
 *  Nest the equivalent "using" directive in the class.
 *
 *      typedef unsigned int (*pluralfunc)(int n);
 */

class pluralforms
{

public:

    /**
     *  Provides the function signature of a plural-forms selection function.
     */

    using function = unsigned (*) (int n);

    /**
     *  A list of pluralforms objects keyed by plural-forms descriptors.
     */

    using map = std::unordered_map<std::string, pluralforms>;

private:

    /**
     *  The number of plural forms this object can choose.
     */

    unsigned int m_nplural;

    /**
     *  The function that chooses the desired plural form.
     */

    function mf_plural;

public:

    static pluralforms from_string (const std::string & str);

    /*
     * Take note of the "default" specifiers here.
     */

    pluralforms ();
    pluralforms (unsigned nplural, function plural);
    pluralforms (const pluralforms &) = default;
    pluralforms (pluralforms &&) = default;
    pluralforms & operator = (const pluralforms &) = default;
    ~pluralforms () = default;

    unsigned get_nplural () const
    {
        return m_nplural;
    }

    unsigned get_plural (int n) const
    {
        return mf_plural ? mf_plural(n) : 0 ;
    }

    bool operator == (const pluralforms & other) const
    {
        return m_nplural == other.m_nplural && mf_plural == other.mf_plural;
    }

    bool operator != (const pluralforms & other) const
    {
        return ! (*this == other);
    }

    explicit operator bool () const
    {
        return mf_plural != nullptr;
    }

};              // class pluralforms

}               // namespace po

#endif          // POTEXT_PO_PLURALFORMS_HPP

/*
 * pluralforms.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */


