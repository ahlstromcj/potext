# README for Potext 0.2.0 2024-03-26

__Potext__ is a partial replacement for the __GNU Gettext__ library and a
refactoring of the __Tinygettext__ library, with some additional features. It
supports reimplementations of basic __GNU gettext__ functions for __C++__
libraries and applications. In basic usage, no change in phrase marking
should needed.

# Major Features

    *   Note for this new version: clean the build directory (e.g. by
        running "./work.sh clean" before building, as meson.build files
        have changed.

##  Library Features

    *   The build system is Meson, and sample wrap files are provided
        for using Potext as a C++ subproject.
    *   The important features of the old tinygettext library follow.
        *   Rather than requiring a .mo file, the library directly parses
            .po files, which are human readable.
        *   Domain support.
        *   Locale directory support.
        *   Character-set support.
    *   Provides reimplementations for textdomain(), bindtextdomain(),
        provides gettext() and related functions, wrapped in the "po"
        namespace.

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

##  Documentation

    *   A PDF developers guide with UML diagrams is included to make
        it much easier to understand how the library works. It also walks the
        reader through all of the test applications that come with Potext.

## To Do

    *   Try to work around spaces and punctuation in strings that do
        not contribute to the meaning and translation.  That is, ignore
        them, but restore them on translation.
    *   Figure out how to deal with the "category" parameter. LC_MESSAGES
        is by far the most common, but....
    *   Add the parsing of .mo files to create the internal dictionaries.
    *   Flesh out and test the Windows support; it still has problems.
    *   Tighten up the nlsbinding class and the dictionarymgr class.
    *   See Section 1.4 of the developer guide for more.

## Recent Changes

    None yet.

// vim: sw=4 ts=4 wm=2 et ft=markdown
