/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ifndef	NOIDENT
#ident	"@(#)olexamples:text/unittest11.c	1.4"
#endif

/*
 *  OLXTK-32.22 - test that the XtNsourceType resource works
 *	for the valid values: OL_STRING_SOURCE, OL_DISK_SOURCE, and
 *	OL_PROG_DEFINED_SOURCE.
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xol/OpenLook.h>
#include <Xol/ControlAre.h>
#include <Xol/Text.h>

static char string1[] = "This is a text widget. \
 This test is setting XtNsourceType to OL_STRING_SOURCE.";

static char string3[] = "This is a text widget. \
 This test is setting XtNsourceType to OL_PROG_DEFINED_SOURCE.";

void main (argc, argv)
int argc;
char **argv;
{
	Widget toplevel, box, editText;
	Arg arg[20];
	unsigned int n;

	toplevel = OlInitialize("quitButton",
		"QuitButton",
		NULL,
		0,
		&argc,
		argv);

	n = 0;
	XtSetArg(arg[n], XtNlayoutType, OL_FIXEDCOLS);		n++;
	box = XtCreateManagedWidget("ControlArea",
		controlAreaWidgetClass,
		toplevel,
		arg,
		n);

	n = 0;
	XtSetArg(arg[n], XtNstring, string1);			n++;
	XtSetArg(arg[n], XtNsourceType, OL_STRING_SOURCE);	n++;
	XtSetArg(arg[n], XtNwidth, 200);			n++;
	editText = XtCreateManagedWidget("Text",
		textWidgetClass,
		box,
		arg,
		n);

	n = 0;
	XtSetArg(arg[n], XtNfile, "disksrc");			n++;
	XtSetArg(arg[n], XtNsourceType, OL_DISK_SOURCE);	n++;
	XtSetArg(arg[n], XtNwidth, 200);			n++;
	XtSetArg(arg[n], XtNeditType, OL_TEXT_EDIT);		n++;
	editText = XtCreateManagedWidget("Text",
		textWidgetClass,
		box,
		arg,
		n);

/*
	n = 0;
	XtSetArg(arg[n], XtNstring, string3);			n++;
	XtSetArg(arg[n], XtNsourceType, OL_PROG_DEFINED_SOURCE);n++;
	XtSetArg(arg[n], XtNwidth, 200);			n++;
	editText = XtCreateManagedWidget("Text",
		textWidgetClass,
		box,
		arg,
		n);
*/

	XtRealizeWidget(toplevel);
	XtMainLoop();
}
