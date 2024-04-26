# README for Potext 0.2.1 2024-04-26

__Potext__ is a partial replacement for the __GNU Gettext__ library and a
refactoring of the __Tinygettext__ library, with some additional features. It
supports reimplementations of basic __GNU gettext__ functions for __C++__
libraries and applications. In basic usage, no change in phrase marking
should needed.

# Major Features

    Note for this new version: clean the build directory (e.g. by
    running "./work.sh clean" before building, as meson.build files
    have changed.

##  Library Features

    *   Can be built using GNU C++ or Clang C++.
    *   The important features of the old tinygettext library follow.
        *   Rather than requiring a .mo file, the library directly parses
            .po files, which are human readable.
        *   A .mo file can be used instead of a .po if desired.
        *   Domain support.
        *   Locale directory support.
        *   Character-set support.
        *   Plural-forms support.
    *   Provides reimplementations for textdomain(), bindtextdomain(),
        provides gettext() and related functions, wrapped in the "po"
        namespace.
    *   Basic dependencies: Meson 1.1 and above; C++17 and above.
    *   The build system is Meson, and sample wrap files are provided
        for using Potext as a C++ subproject.
    *   PDF documentation built from LaTeX.

##  Usage

    *   See the test application hellopotext for common use-cases.
    *   See the "Potext Usage in Applications" section of the developer
        guide noted below.
    *   Simple Meson setup for an application using Potext as a
        subproject is included in a "tar" file in the extras/code directory.
        Unpack it somewhere and build it.

##  Code

    *   The code has been substantially rewritten for readability
        (by the Potext author). This includes getting rid of camel-case
        names and using the "auto" keyword.
    *   As with Tinygettext, C++17 is required for some of its features,
        such as usage of std::filesystem.
    *   The GNU and Clang C++ compilers are supported.

##  Fixes

    *   An empty message context tag (msgctxt "") is treated differently
        from non-present message contexts, as per the GNU documentation.
        This avoid unnecessary message translation collisions in the
        dictionary code.
    *   Improved the work.sh, added an --uninstall option.

##  Documentation

    *   A PDF developers guide with UML diagrams is included to make
        it much easier to understand how the library works. It also walks the
        reader through all of the test applications that come with Potext.

## To Do

    *   Try to work around spaces and punctuation in strings that do
        not contribute to the meaning and translation.  That is, ignore
        them, but restore them on translation.
    *   Figure out how to deal with the "category" parameter. LC\_MESSAGES
        is by far the most common, but....
    *   Flesh out and test the Windows support; it still has problems.
    *   Tighten up the nlsbinding class and the dictionarymgr class.
    *   Add using a soft link for the *.so library file.
    *   Make sure uninstall works.
    *   See Section 1.4 of the developer guide for more.

## Recent Changes

    None yet.

// vim: sw=4 ts=4 wm=2 et ft=markdown
