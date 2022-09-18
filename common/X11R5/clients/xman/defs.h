/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)r5xman:defs.h	1.1"
/*
 * xman - X window system manual page display program.
 *
 * $XConsortium: defs.h,v 1.24 91/02/13 16:08:07 converse Exp $
 *
 * Copyright 1987, 1988 Massachusetts Institute of Technology
 *
 *
 * Author:    Chris D. Peterson, MIT Project Athena
 * Created:   October 22, 1987
 */

#ifndef HELPFILE
#define HELPFILE "/usr/lib/X11/xman.help" /* name of the default helpfile. */ 
#endif

/* The default cursors */

#define XMAN_CURSOR "left_ptr"		/* Top level cursor. */
#define HELP_CURSOR "left_ptr"	        /* The help cursor. */
#define MANPAGE_CURSOR "left_ptr"	/* The manpage cursor. */
#define SEARCH_ENTRY_CURSOR "question_arrow"	/* The search text widget
						   cursor. */
#define DIRECTORY_NORMAL "fixed" /* The default dir font */

#define OPTION_MENU "optionMenu" /* Name of the Option Menu. */
#define SECTION_MENU "sectionMenu" /* Name of the Section Menu. */

#define HELP_BUTTON "helpButton" /* Name of top help button */
#define QUIT_BUTTON "quitButton" /* Name of top quit button */
#define MANPAGE_BUTTON "manpageButton" /* Name of top manpage button */

#define TOPBOXNAME  "topBox"	/* Name of the Top Box. */
#define MANNAME "manualBrowser"	/* name for each manual page widget. */
#define SEARCHNAME "search" /* The name for the search widget. */
#define HELPNAME  "help"	/* The name of the help widget. */
#define DIRECTORY_NAME "directory" /* name of the directory widget. */
#define MANUALPAGE "manualPage"	/* name of the Scrollbyline widget that
				 contains the man page. */
#define DIALOG         "dialog"

/* Names of the menu buttons */

#define NUM_OPTIONS 9		/* Number of menu options. */

#define DIRECTORY      "displayDirectory"
#define MANPAGE        "displayManualPage"
#define HELP           "help"
#define SEARCH         "search"
#define BOTH_SCREENS   "showBothScreens"
#define REMOVE_MANPAGE "removeThisManpage"
#define OPEN_MANPAGE   "openNewManpage"
#define SHOW_VERSION   "showVersion"
#define QUIT           "quit"

/* definitions of string to use for show both and show one. */

#define SHOW_BOTH "Show Both Screens"
#define SHOW_ONE "Show One Screen"

/* 
 * Things will not look right if you change these names to make 
 * MANUALSEARCH longer APROPOSSEARCH, see search.c for details.
 */

#define MANUALSEARCH "manualPage"
#define APROPOSSEARCH "apropos"
#define CANCEL "cancel"

#define MANUAL 0
#define APROPOS 1

#define NO_SECTION_DEFAULTS ("no default sections")

#define TBL "tbl"

#define DEFAULT_WIDTH 500	/* The default width of xman. */
#define SECTALLOC  8		/* The number of entries allocated
				   at a time for the manual structures. */
#define ENTRYALLOC 100		/* The number of entries allocated
				   at a time for a section. */

#define INITIAL_DIR 0		/* The Initial Directory displayed. */

#define COPY "cp"		/* copy command */
#define CHMOD_MODE 00666	/* permissions set on saved formatted files */
#define MANDESC "mandesc"	/* name of the mandesc files */

#define INDENT 15
#define TYP20STR "MMMMMMMMMMMMMMMMMMMM"

#define FILE_SAVE "yes"
#define CANCEL_FILE_SAVE "no"
#define MANTEMP "/tmp/xmanXXXXXX"

/*
 * Macro Definitions.
 */

#define streq(a, b)        ( strcmp((a), (b)) == 0 )

/* 
 * Function definitions moved to man.h
 */
