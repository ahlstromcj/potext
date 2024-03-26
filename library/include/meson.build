#*****************************************************************************
# meson.build (potext/library/include)
#-----------------------------------------------------------------------------
##
# \file        library/include/meson.build
# \library     potext
# \author      Chris Ahlstrom
# \date        2024-02-06
# \updates     2024-03-25
# \license     $XPC_SUITE_GPL_LICENSE$
#
#  This file is part of the "potext" library. See the top-level meson.build
#  file for license information.
#
#  Here we locate the headers. The base headers go directly under the potext
#  header location (/usr/locale/include/potext.../) and the rest go
#  under that in the 'po' directory.
#
#  The arrays are defined in library/meson.build.
#
#  Headers for generic macros, platform detection, and the feature
#  specification for the build.
#
#-----------------------------------------------------------------------------

libpotext_base_headers += files(
   'c_macros.h',
   'platform_macros.h',
   'po_build_macros.h'
   )

#-----------------------------------------------------------------------------
#  Headers for the potext classes and functions.
#-----------------------------------------------------------------------------

libpotext_headers += files(
   'po/aliases.hpp',
   'po/bfplurals.hpp',
   'po/dictionary.hpp',
   'po/dictionarymgr.hpp',
   'po/dirent.h',
   'po/extractor.hpp',
   'po/filesystem.hpp',
   'po/gettext.hpp',
   'po/gettextex.hpp',
   'po/iconvert.hpp',
   'po/language.hpp',
   'po/languagespecs.hpp',
   'po/logstream.hpp',
   'po/moparser.hpp',
   'po/nlsbindings.hpp',
   'po/pluralforms.hpp',
   'po/pomoparserbase.hpp',
   'po/poparser.hpp',
   'po/potext.hpp',
   'po/po_types.hpp',
   'po/tinygettext.hpp',
   'po/unixfilesystem.hpp'
   )

# Not yet needed.
#
# configure_file(
#    input: 'potext-config.h.in',
#    output: 'potext-config.h',
#    configuration: cdata
#    )

#-----------------------------------------------------------------------------
# Headers to install
#-----------------------------------------------------------------------------
#
# The headers are stored in ${prefix}/include/potext-0.1/po and the -I value
# is ${prefix}/include/potext-0.1.
#
#-----------------------------------------------------------------------------

install_headers(libpotext_base_headers, subdir : po_dir)
install_headers(libpotext_headers, subdir : po_po_dir)

#****************************************************************************
# meson.build (potext/library/include)
#----------------------------------------------------------------------------
# vim: ts=3 sw=3 ft=meson
#----------------------------------------------------------------------------
