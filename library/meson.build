#*****************************************************************************
# meson.build (potext)
#-----------------------------------------------------------------------------
##
# \file        meson.build
# \library     potext
# \author      Chris Ahlstrom
# \date        2024-02-07
# \updates     2024-04-25
# \license     $XPC_SUITE_GPL_LICENSE$
#
#  This file is part of the potext library and tests. See the top-level
#  meson.build file.
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# The *.pc file goes into $libdir/potext-0.2/pkgconfig instead of
# $libdir/pkgconfig as the meson documentation asserts.  Not sure why, but
# wiring the expected directory here works.
#-----------------------------------------------------------------------------

alt_pkgconfig_libdir = join_paths(
   po_prefix, get_option('libdir'), 'pkgconfig'
   )

#-----------------------------------------------------------------------------
# Dependencies and a small app to test them.  Symbols from/for the subprojects.
# Note that Meson subprojects are meant to be self-contained projects, but we
# don't use subprojects here. Also note that the order of the subdirs reflects
# a hierarchy of dependencies.
#-----------------------------------------------------------------------------

libpotext_base_headers = []
libpotext_headers = []
libpotext_sources = []
build_args = []

#-----------------------------------------------------------------------------
# Dependencies on Linux
#-----------------------------------------------------------------------------
#
# None, yet.
#
#-----------------------------------------------------------------------------

empty_depends = [ ]

#-----------------------------------------------------------------------------
# Used by the potext_version() function.
#-----------------------------------------------------------------------------

build_args += [
   '-DPOTEXT_NAME=' + '"' + meson.project_name() + '"',
   '-DPOTEXT_VERSION=' + '"' + meson.project_version() + '"'
   ]

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
# Build
#-----------------------------------------------------------------------------
#
#  The #include statements alway have the relative path in them, so
#  we need add in only each sub-project's "include" directory. No need
#  for "include/po"... force all modules to use "#include po/xyz.hpp".
#
#-----------------------------------------------------------------------------

po_includedirs = include_directories('include')
po_install = not po_subproj

#-----------------------------------------------------------------------------
# If set above, libpotext_dep is not found. Linkage options:
#
#     link_with : po_library_build
#     link_with : po_library_shared
#     link_with:  po_library_static
#
# These library builds are specified in library/src/meson.build.
#
# Can also add dependency variables:
#
#     variables : { 'var' : 'value', 'number' : '3' }
#
# To access from another project:
#
#     var = libpotext_dep.get_variable(internal : 'var', cmake : 'CMAKE_VAR)
#
#-----------------------------------------------------------------------------

subdir('include')
subdir('src')

#-----------------------------------------------------------------------------
# Selectable static-or-shared library
#-----------------------------------------------------------------------------

po_library_build = library(
   po_project_base,
   libpotext_sources,
   install : po_install,
   install_dir : get_option('libdir') / po_project_base,
   cpp_args : build_args,
   dependencies : empty_depends,
   include_directories : po_includedirs
   )

#-----------------------------------------------------------------------------
# Libtool versioning for shared library
#-----------------------------------------------------------------------------
#
# Use libtool-version numbers because they're easier to understand. Before
# making a release, the libxxx_so_* numbers should be modified. Components
# are of the form C:R:A.
#
# a. If binary compatibility has been broken (removed/changed interfaces)
#    change to C+1:0:0.
# b. If interfaces have been changed or added, with binary compatibility
#    preserved, change to C+1:0:A+1
# c. If the interface is the same as the previous version, change to
#    C:R+1:A
#
#-----------------------------------------------------------------------------

libpotext_lt_c = 0
libpotext_lt_r = 2
libpotext_lt_a = 0
libpotext_so_version = '@0@.@1@.@2@'.format(
   (libpotext_lt_c - libpotext_lt_a), libpotext_lt_a, libpotext_lt_r)

#-----------------------------------------------------------------------------
# Always build the tests unless potext is being used as a subproject.
# Note the external dependencies meant for superprojects of potex.
#-----------------------------------------------------------------------------

libpotext_dep = declare_dependency(
   include_directories : [ po_includedirs ],
   link_with : [ po_library_build ]
   )

#-----------------------------------------------------------------------------
# Future:
#
#  1. Might need library_dep = dependency('threads') and add it to the
#     library dependencies setting.
#  2. Usage of library() versus static_library() and shared_library().
#  3. Pkgconfig: Add extra_cflags()?
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# The *.pc file goes into $libdir/potext-0.1/pkgconfig instead of
# $libdir/pkgconfig as the meson documentation asserts.  Not sure why, but
# wiring the expected directory here works.
#
# This will cause libpotext.pc to be created and put in the install
# directory.
#
#     libraries : po_library_shared,
#
#-----------------------------------------------------------------------------

if not po_subproj

   #----------------------------------------------------------
   # install_subdir(), install_data(): top-level meson.build
   # subdir('tests'): library/meson.build
   #----------------------------------------------------------

   if get_option('enable-tests')
      subdir('tests')
   endif

endif

# po_exe_sources = files('potext_linkage.cpp')
# 
# po_exe = executable('potext_linkage',
#    po_exe_sources,
#    dependencies: libpotext_dep,
#    install: true
# )
# 
# test('Basic Linkage', po_exe)

#****************************************************************************
# meson.build (potext)
#----------------------------------------------------------------------------
# vim: ts=3 sw=3 ft=meson
#----------------------------------------------------------------------------
