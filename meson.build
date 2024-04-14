#*****************************************************************************
# meson.build (potext)
#-----------------------------------------------------------------------------
##
# \file        meson.build
# \library     potext
# \author      Chris Ahlstrom
# \date        2024-02-06
# \updates     2024-03-31
# \license     $XPC_SUITE_GPL_LICENSE$
#
#  This file is part of the potext library and tests.
#
#  The "potext" library is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  The "potext" library is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU General Public License along with
#  the "potext" library. If not, see <https://www.gnu.org/licenses/>.
#
#     See the "work.sh" bash script.
#
# Debug:
#
#     In order to see the library modules (and not just the main application
#     module), set 'default_library=static' instead of using 'both'.
#
# Warning level:
#
#     At some point, try setting it to "everything".
#
# Release:
#
#     See the description of the 'buildtype' option in contrib/meson.text.
#     Specify "--buildtype=release" on the meson command line.
#
# Reconfigure:
#
#     $ meson configure -Dprefix=/home/me/tmp -Dbuildtype=release build/
#
# Installation:
#
#     The library is build/libXXX-0.2.a. We will install the library
#     all in in /usr/lib/potext-0.2 or /usr/local/lib/potext-0.2, rather
#     than a subdirectory for each of these somewhat related libraries.
#
# C++17:
#
#     We were using C++14, but using std::filesystem requires C++17.
#
#-----------------------------------------------------------------------------

project('potext', 'cpp',
   version: '0.2.0',
   license: 'GPLv2',
   meson_version: '>=1.1.0',
   default_options: [
      'cpp_std=c++17', 'warning_level=3',
      'default_library=both'          # 'static' for debug, 'both' otherwise
      ]
   )

#-----------------------------------------------------------------------------
# Extra flags
#-----------------------------------------------------------------------------

'''
cxx = meson.get_compiler('cpp')
extra_flags = [
   '-fno-common',
   '-Wshadow',
   '-Wstrict-overflow=5',
   '-Wformat-security',
   '-Wformat-overflow=2',
   '-Wunused-result'
   ]
'''

#-----------------------------------------------------------------------------
# Global information for all the libraries.
#-----------------------------------------------------------------------------

po_description = 'Builds the potext library and tests projects.'
po_date = '2024-03-31'
po_version = meson.project_version()
po_api_version = '0.2'
po_build_type = get_option('buildtype')
po_debug = get_option('buildtype').contains('debug')
po_project_base = 'potext-@0@'.format(po_api_version)
po_library_base = 'libpotext-@0@'.format(po_api_version)
po_project_root = meson.project_source_root()
po_libtype = get_option('default_library')
po_data_dir = get_option('datadir') / 'potext-@0@'.format(po_api_version)

if po_debug
   if not meson.is_subproject()
      add_global_arguments('-DDEBUG', language : 'cpp')
   endif
endif

#-----------------------------------------------------------------------------
# Easy access to directory options.  Interim until meson version 0.64.0
# We want to be able to install the header files to subdirectories of,
# for example, /usr/local/include/potext-0.2, just as they appear in this
# project, where the include subdirectory is 'po'.
#-----------------------------------------------------------------------------

po_dir = 'potext-@0@'.format(po_api_version)
po_po_dir = join_paths(po_dir, 'po')
po_prefix = get_option('prefix')
po_bindir = join_paths(po_prefix, get_option('bindir'))
po_includedir = join_paths(po_prefix, get_option('includedir'), po_dir)
po_libdir = join_paths(po_prefix, get_option('libdir'), po_dir)
po_datadir = join_paths(po_prefix, get_option('datadir'), po_dir)
po_localedir = join_paths(po_prefix, get_option('localedir'))
po_metainfodir = join_paths(po_datadir, 'metainfo')
po_docdir = join_paths(po_prefix, po_datadir, 'doc', po_dir)
po_project_root = meson.project_source_root()
po_libtype = get_option('default_library')

libraries_root = 'library'
libraries_top = '.'
libpotext_includetop = join_paths(libraries_top, 'include')

#-----------------------------------------------------------------------------
# pkgconfig
#-----------------------------------------------------------------------------
#
#     pkg.generate(): see library/meson.build
#     install_headers(): see library/include/meson.build
#
#-----------------------------------------------------------------------------

if not meson.is_subproject()

# TODO: figure out where to put and install the potext logo
#  install_subdir(
#     join_paths('doc'),
#     install_dir : get_option('docdir'),
#     strip_directory : true
#     )

   install_data(
      files(
         'ChangeLog',
         'LICENSE.md',
         'README.md',
         'doc/potext-developer-guide.pdf'
         ),
      install_dir : get_option('docdir')
      )

endif

subdir('library')

#-----------------------------------------------------------------------------
# Dependencies on Linux
#-----------------------------------------------------------------------------

empty_depends = [ ]

#-----------------------------------------------------------------------------
# Only make public interfaces visible
#-----------------------------------------------------------------------------

# if target_machine.system() == 'windows' or target_machine.system() == 'cygwin'
#    build_args += '-DLIBPUBLIC="__declspec(dllexport)"'
# else
#    build_args += '-DLIBPUBLIC=__attribute__((visibility("default")))'
# endif

#-----------------------------------------------------------------------------
# Info
#-----------------------------------------------------------------------------

summary(
   {
      'Date' : po_date,
      'Filebase' : po_project_base,
      'Version' : po_version,
      'Base' : po_project_base,
      'Description' : po_description,
      'Sub Dir' : po_project_base,
      'Build Type' : po_build_type,
      'Library Type' : po_libtype,
      'Root' : po_project_root
   },
   section : 'Project'
   )

summary(
   {
      'prefix' : po_prefix,
      'includedir' : po_includedir,
      'bindir' : po_bindir,
      'libdir' : po_libdir,
      'datadir' : po_datadir,
      'appdatadir' : po_datadir
   },
   section : 'Install Directories'
   )

#****************************************************************************
# meson.build (potext)
#----------------------------------------------------------------------------
# vim: ts=3 sw=3 ft=meson
#----------------------------------------------------------------------------
