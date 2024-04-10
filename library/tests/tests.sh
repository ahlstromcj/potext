#!/bin/sh
#
#******************************************************************************
# tests.sh (potext)
#------------------------------------------------------------------------------
##
# \file           tests.sh
# \library        potext
# \author         Chris Ahlstrom
# \date           2024-02-14
# \update         2024-04-10
# \version        $Revision$
# \license        $XPC_SUITE_GPL_LICENSE$
#
#     The above is modified by the following to remove even the mild GPL
#     restrictions:
#
#     Use this script in any manner whatsoever.  You don't even need to give
#     me any credit.  However, keep in mind the value of the GPL in keeping
#     software and its descendant modifications available to the community for
#     all time.
#
#     Runs a number of tests listed in library/tests/testlines.list.
#     Currently meant to be run from the top directory of potext:
#
#     potext $ ./library/tests/tests.sh &> tests.log
#
# Issue:
#
#     On an older Ubuntu system, a lot of errors like this appear in the
#     output:
#
#     error: po::iconvert: invalid multibyte sequence in: "-Übersetzung" @1
#
#     On an Arch system, they do not appear. Here are the two versions of
#     iconv, as shown by "iconv --version":
#
#        iconv (Ubuntu GLIBC 2.31-0ubuntu9.14) 2.31 (copyright 2020)
#
#        iconv (GNU libc) 2.39 (Arch, copyright 2024)
#
#------------------------------------------------------------------------------

LANG=C
export LANG
POTEXT_SCRIPT_EDIT_DATE="2024-04-10"
POTEXT_LIBRARY_API_VERSION="0.2"
POTEXT_LIBRARY_VERSION="$POTEXT_LIBRARY_API_VERSION.0"
POTEXT="potext"
POTEXT_TEST_BINARY_DIR="./build/library/tests"
POTEXT_TEST="$POTEXT_TEST_BINARY_DIR/potext_test"
HELLO_POTEXT="$POTEXT_TEST_BINARY_DIR/hellopotext"
MO_PARSER_TEST="$POTEXT_TEST_BINARY_DIR/mo_parser_test"
PO_PARSER_TEST="$POTEXT_TEST_BINARY_DIR/po_parser_test"
POTEXT_TEST_DIR="./library/tests"
POTEXT_TEST_LINES="$POTEXT_TEST_DIR/testlines.list"
COUNTER=0
RESULT="PASSED"

echo "test.sh run started at"
date

#----------------------------------------------------------------------------

echo
echo "$HELLO_POTEXT:"
echo
LASTRESULT="PASSED"
$HELLO_POTEXT
if test $? != 0 ; then
   LASTRESULT="FAILED"
   RESULT="FAILED"
fi
echo "[$LASTRESULT] hellopotext"

#----------------------------------------------------------------------------

echo
echo "$PO_PARSER_TEST --all:"
echo
LASTRESULT="PASSED"
$PO_PARSER_TEST --all
if test $? != 0 ; then
   LASTRESULT="FAILED"
   RESULT="FAILED"
fi
echo "[$LASTRESULT] po_parser_text"

#----------------------------------------------------------------------------

echo
echo "$MO_PARSER_TEST --all:"
echo
LASTRESULT="PASSED"
$MO_PARSER_TEST --all
if test $? != 0 ; then
   LASTRESULT="FAILED"
   RESULT="FAILED"
fi
echo "[$LASTRESULT] mo_parser_text"

#----------------------------------------------------------------------------

echo
echo "$POTEXT_TEST tests from $POTEXT_TEST_LINES:"
echo
LASTRESULT="PASSED"
while IFS= read -r INLINE
do
    FIRST=$(printf %.1s "$INLINE")
    if test "$FIRST" != "#" -a "$FIRST" != "" ; then
      echo "$POTEXT_TEST $INLINE"
      $POTEXT_TEST $INLINE
      if test $? != 0 ; then
         LASTRESULT="FAILED"
         RESULT="FAILED"
         echo "TEST FAILED: $INLINE"
      fi
      echo "  $COUNTER [$LASTRESULT] potext_test $INLINE"
      LASTRESULT="PASSED"
      COUNTER=$((COUNTER+1))
    fi
done < $POTEXT_TEST_LINES

#----------------------------------------------------------------------------

echo
echo "OVERALL RESULT: $RESULT"

#******************************************************************************
# tests.sh (potext)
#------------------------------------------------------------------------------
# vim: ts=3 sw=3 wm=4 et ft=sh
#------------------------------------------------------------------------------
