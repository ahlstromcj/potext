#if !defined POTEXT_PO_LOGSTREAM_HPP
#define POTEXT_PO_LOGSTREAM_HPP

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
 * \file          logstream.hpp
 *
 *      A replacement for tinygettext/Log and logstream.
 *
 * \library       potext
 * \author        tinygettext; refactoring by Chris Ahlstrom
 * \date          2024-02-05
 * \updates       2024-03-16
 * \license       See above.
 *
 */

#include <sstream>                      /* std::ostringstream               */

#include "platform_macros.h"            /* OS & debug macroes etc.          */

namespace po
{

class logstream
{

public:

    using callback = void (*) (const std::string &);

private:

    static bool sm_enable_testing;
    static bool sm_test_error;
    static bool sm_use_std_cerr;

    static callback sm_info_callback;
    static callback sm_warning_callback;
    static callback sm_error_callback;

    callback m_callback;
    std::ostringstream m_out;

public:

    logstream ();
    logstream (callback cb);
    logstream (const logstream &) = delete;
    logstream (logstream &&) = delete;
    logstream & operator = (const logstream &) = delete;
    ~logstream ();

    static void set_enable_testing ();
    static bool get_enable_testing ();
    static void set_test_error ();
    static void clear_test_error ();
    static bool get_test_error ();
    static bool get_use_std_cerr ();
    static void callbacks_set_all (callback cb);
    static void set_info_callback (callback cb);
    static void set_warning_callback (callback cb);
    static void set_error_callback (callback cb);

    static std::ostream & info ();
    static std::ostream & warning ();
    static std::ostream & error ();

private:

    static void callbacks_reset ();
    static void def_error_callback (const std::string & str);
    static void def_warn_callback (const std::string & str);
    static void def_info_callback (const std::string & str);

    std::ostream & get ();

};          // class logstream

}           // namespace po

#endif      // POTEXT_PO_LOGSTREAM_HPP

/*
 * logstream.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
