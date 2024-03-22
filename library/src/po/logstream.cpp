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
 * \file          logstream.cpp
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

#include <iostream>

#include "po/logstream.hpp"

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
 *  Constructors.
 */

logstream::logstream () :
    m_callback  (&logstream::def_error_callback),
    m_out       ()
{
    // no other code
}

logstream::logstream (callback cb) :
    m_callback  (cb),
    m_out       ()
{
    if (! cb)
        m_callback = &logstream::def_error_callback;
}

logstream::~logstream()
{
    if (! m_out.str().empty())
        m_callback(m_out.str());
}

/**
 *  The default static callbacks ofr errors, warnings, and information.
 */

void
logstream::def_error_callback (const std::string & str)
{
    if (str.empty())
        std::cerr << "[potext] " << _("empty error message") << std::endl;
    else
        std::cerr << "[potext] " << str << std::endl;
}

void
logstream::def_warn_callback (const std::string & str)
{
    if (get_enable_testing())
    {
        if (str.empty())
            std::cerr << "[potext] " << _("empty warning message") << std::endl;
        else
            std::cerr << "[potext] " << str << std::endl;
    }
}

void
logstream::def_info_callback (const std::string & str)
{
    if (get_enable_testing())
    {
        if (str.empty())
            std::cerr << "[potext] " << _("empty info message") << std::endl;
        else
            std::cerr << "[potext] " << str << std::endl;
    }
}

/**
 *  Static values and functions for testing and error exits.
 */

bool logstream::sm_enable_testing   = false;
bool logstream::sm_test_error       = false;
bool logstream::sm_use_std_cerr     = false;

void
logstream::set_enable_testing ()
{
    sm_enable_testing = sm_use_std_cerr = true;
}

bool
logstream::get_enable_testing ()
{
    return sm_enable_testing;
}

void
logstream::set_test_error ()
{
    sm_test_error = true;
}

void
logstream::clear_test_error ()
{
    sm_test_error = false;
}

bool
logstream::get_test_error ()
{
    return sm_test_error;
}

bool
logstream::get_use_std_cerr ()
{
    return sm_use_std_cerr;
}

/**
 *  Static default callback function values
 */

logstream::callback logstream::sm_info_callback =
    &logstream::def_info_callback;

logstream::callback logstream::sm_warning_callback =
    &logstream::def_warn_callback;

logstream::callback logstream::sm_error_callback =
    &logstream::def_error_callback;

/**
 *  Alternate static callback assignment functions.
 */

void
logstream::set_info_callback (callback cb)
{
    if (cb)
        sm_info_callback = cb;
}

void
logstream::set_warning_callback (callback cb)
{
    if (cb)
        sm_warning_callback = cb;
}

void
logstream::set_error_callback (callback cb)
{
    if (cb)
        sm_error_callback = cb;
}

void
logstream::callbacks_set_all (callback cb)
{
    if (cb)
    {
        sm_info_callback    = cb;
        sm_warning_callback = cb;
        sm_error_callback   = cb;
    }
}

void
logstream::callbacks_reset ()
{
    sm_info_callback    = &logstream::def_info_callback;
    sm_warning_callback = &logstream::def_warn_callback;
    sm_error_callback   = &logstream::def_error_callback;
}

/**
 *  Static access functions. An improvement over the log_error, etc. macros.
 *  Like the macros, though, these functions create a new logstream object
 *  using the given callback, and then call that object's get() function to
 *  return the output stream.
 *
 *  Interestingly, all the std::cout and std::cerr output is shown to the
 *  console before all of the logstream output is shown.
 */

std::ostream &
logstream::info ()
{
    static logstream s_stream(logstream::sm_info_callback);
    return s_stream.get();
}

std::ostream &
logstream::warning ()
{
    static logstream s_stream(logstream::sm_warning_callback);
    return s_stream.get();
}

/**
 *  We want to be able to redirect errors to a log file when testing.
 *  So now we have a flag for that.
 */

std::ostream &
logstream::error ()
{
    if (get_enable_testing())
        set_test_error();

    if (get_use_std_cerr())
    {
        return std::cerr;
    }
    else
    {
        static logstream s_stream(logstream::sm_error_callback);
        return s_stream.get();
    }
}

std::ostream &
logstream::get ()
{
    return m_out;
}

}           // namespace tinygettext

/*
 * logstream.cpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */
