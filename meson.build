#*****************************************************************************
# meson.build (potext)
#-----------------------------------------------------------------------------
##
# \file        meson.build
# \library     potext
# \author      Chris Ahlstrom
# \date        2024-02-06
# \updates     2024-04-25
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
   version: '0.2.1',
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

po_pkg_description = 'A small gettext replacement extending tinygettext.'
po_pkg_version = meson.project_version()
po_api_version = '0.2'
po_info_date = '2024-04-25'
po_info_build_type = get_option('buildtype')
po_info_build_root = meson.project_build_root()
po_info_lib_type = get_option('default_library')
po_info_project_root = meson.project_source_root()
po_debug = get_option('buildtype').contains('debug')
po_project_base = 'potext-@0@'.format(po_api_version)
po_library_base = 'libpotext-@0@'.format(po_api_version)
po_pkgconfig_name = 'libpotext'
po_subproj = meson.is_subproject()

if po_debug
   if not po_subproj
      add_global_arguments('-DDEBUG', language : 'cpp')
   endif
endif

#-----------------------------------------------------------------------------
# Easy access to directory options.  Interim until meson version 0.64.0
# We want to be able to install the header files to subdirectories of,
# for example, /usr/local/include/potext-0.2, just as they appear in this
# project, where the include subdirectory is 'po'.
#-----------------------------------------------------------------------------
# po_data_dir = get_option('datadir') / 'potext-@0@'.format(po_api_version)
# libraries_root = 'library'

po_dir = 'potext-@0@'.format(po_api_version)
po_po_dir = join_paths(po_dir, 'po')
po_prefix = get_option('prefix')
po_bindir = join_paths(po_prefix, get_option('bindir'))
po_includedir = join_paths(po_prefix, get_option('includedir'), po_dir)
po_libdir = join_paths(po_prefix, get_option('libdir'), po_dir)
po_datadir = join_paths(po_prefix, get_option('datadir'), po_dir)
po_docdir = join_paths(po_prefix, po_datadir, 'doc', po_dir)
po_localedir = join_paths(po_prefix, get_option('localedir'))
po_metainfodir = join_paths(po_datadir, 'metainfo')
libraries_top = meson.project_source_root()
libpotext_include_top = join_paths(libraries_top, 'include')

#-----------------------------------------------------------------------------
# pkgconfig
#-----------------------------------------------------------------------------
#
#     pkg.generate():    resides in library/meson.build
#     install_headers(): see library/include/meson.build
#
#-----------------------------------------------------------------------------

if not po_subproj

# TODO: figure out where to put and install the logo
#
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
# Only make public interfaces visible
#-----------------------------------------------------------------------------

'''
if target_machine.system() == 'windows' or target_machine.system() == 'cygwin'
   build_args += '-DLIBPUBLIC="__declspec(dllexport)"'
else
   build_args += '-DLIBPUBLIC=__attribute__((visibility("default")))'
endif
'''

#-----------------------------------------------------------------------------
# Package config
#-----------------------------------------------------------------------------
#  On Arch Linux, the .pc file is installed to /usr/local/lib/pkgconfig.
#  On Ubuntu, the .pc file is installed to
#     /usr/local/lib/x86_64-linux-gnu/pkgconfig.
#-----------------------------------------------------------------------------

if not po_subproj

   pkg = import('pkgconfig')
   pkg.generate(
      po_library_build,
      filebase : po_pkgconfig_name,
      version : po_pkg_version,
      name : po_project_base,
      description : po_pkg_description,
      install_dir : alt_pkgconfig_libdir,
      subdirs : po_dir,
      libraries : po_library_build,
      url : 'https://github.com/ahlstromcj/potext',
      )

# TODO: figure out where to put and install the logo

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

#-----------------------------------------------------------------------------
# Info
#-----------------------------------------------------------------------------

summary(
   {
      'Date' : po_info_date,
      'Filebase' : po_project_base,
      'Version' : po_pkg_version,
      'Base' : po_project_base,
      'Description' : po_pkg_description,
      'Source Root' : po_info_project_root,
      'Headers Root' : libpotext_include_top,
      'Sub Dir' : po_project_base,
      'Build Type' : po_info_build_type,
      'Build Root' : po_info_project_root,
      'Library Type' : po_info_lib_type,
      'Subproject' : po_subproj
   },
   section : 'Potext Project'
   )

summary(
   {
      'prefix' : po_prefix,
      'includedir' : po_includedir,
      'bindir' : po_bindir,
      'libdir' : po_libdir,
      'datadir' : po_datadir,
      'docdir': po_docdir
   },
   section : 'Potext Install Directories'
   )

#****************************************************************************
# meson.build (potext)
#----------------------------------------------------------------------------
# vim: ts=3 sw=3 ft=meson
#----------------------------------------------------------------------------
