#*****************************************************************************
# meson.build (potext/tests)
#-----------------------------------------------------------------------------
##
# \file        tests/meson.build
# \library     potext
# \author      Chris Ahlstrom
# \date        2024-02-06
# \updates     2024-04-02
# \license     $XPC_SUITE_GPL_LICENSE$
#
#  This file is part of the "potext" library. See the top-level meson.build
#  file for license information.
#
#-----------------------------------------------------------------------------

helloworld_exe = executable(
   'helloworld',
   sources : [ 'helloworld.cpp' ],
   dependencies : [ libpotext_dep ]
   )

mo_parser_test_exe = executable(
   'mo_parser_test',
   sources : [ 'mo_parser_test.cpp' ],
   dependencies : [ libpotext_dep ]
   )

po_parser_test_exe = executable(
   'po_parser_test',
   sources : [ 'po_parser_test.cpp' ],
   dependencies : [ libpotext_dep ]
   )

potext_test_exe = executable(
   'potext_test',
   sources :  [ 'potext_test.cpp' ],
   dependencies : [ libpotext_dep ]
   )

hellopotext_exe = executable(
   'hellopotext',
   sources : [ 'hellopotext.cpp' ],
   dependencies : [ libpotext_dep ]
   )

test('Hello Potext', hellopotext_exe)
test('Potext Hello World', helloworld_exe)
test('Potext Parser Test', po_parser_test_exe)
test('Potext MO Parser Test', mo_parser_test_exe)
test('Potext Test', potext_test_exe)
   
#****************************************************************************
# meson.build (potext/tests)
#----------------------------------------------------------------------------
# vim: ts=3 sw=3 ft=meson
#----------------------------------------------------------------------------

