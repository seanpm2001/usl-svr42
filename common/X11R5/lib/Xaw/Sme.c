/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)r5Xaw:Sme.c	1.2"
/* $XConsortium: Sme.c,v 1.9 91/02/17 16:44:14 rws Exp $ */

/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 *
 */

/*
 * Sme.c - Source code for the generic menu entry
 *
 * Date:    September 26, 1989
 *
 * By:      Chris D. Peterson
 *          MIT X Consortium 
 *          kit@expo.lcs.mit.edu
 */

#include <stdio.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Xaw/XawInit.h>
#include <X11/Xaw/SmeP.h>
#include <X11/Xaw/Cardinals.h>

#define offset(field) XtOffsetOf(SmeRec, sme.field)
static XtResource resources[] = {
  {XtNcallback, XtCCallback, XtRCallback, sizeof(XtPointer),
     offset(callbacks), XtRCallback, (XtPointer)NULL},
};   
#undef offset

/*
 * Semi Public function definitions. 
 */

static void Unhighlight(), Highlight(), Notify(), ClassPartInitialize();
static void Initialize();
static XtGeometryResult QueryGeometry();

#define SUPERCLASS (&rectObjClassRec)

SmeClassRec smeClassRec = {
  {
    /* superclass         */    (WidgetClass) SUPERCLASS,
    /* class_name         */    "Sme",
    /* size               */    sizeof(SmeRec),
    /* class_initialize   */	XawInitializeWidgetSet,
    /* class_part_initialize*/	ClassPartInitialize,
    /* Class init'ed      */	FALSE,
    /* initialize         */    Initialize,
    /* initialize_hook    */	NULL,
    /* realize            */    NULL,
    /* actions            */    NULL,
    /* num_actions        */    ZERO,
    /* resources          */    resources,
    /* resource_count     */	XtNumber(resources),
    /* xrm_class          */    NULLQUARK,
    /* compress_motion    */    FALSE, 
    /* compress_exposure  */    FALSE,
    /* compress_enterleave*/ 	FALSE,
    /* visible_interest   */    FALSE,
    /* destroy            */    NULL,
    /* resize             */    NULL,
    /* expose             */    NULL,
    /* set_values         */    NULL,
    /* set_values_hook    */	NULL,
    /* set_values_almost  */	XtInheritSetValuesAlmost,  
    /* get_values_hook    */	NULL,			
    /* accept_focus       */    NULL,
    /* intrinsics version */	XtVersion,
    /* callback offsets   */    NULL,
    /* tm_table		  */    NULL,
    /* query_geometry	  */    QueryGeometry,
    /* display_accelerator*/    NULL,
    /* extension	  */    NULL
  },{
    /* Simple Menu Entry Fields */
      
    /* highlight */             Highlight,
    /* unhighlight */           Unhighlight,
    /* notify */		Notify,		
    /* extension */             NULL				
  }
};

WidgetClass smeObjectClass = (WidgetClass) &smeClassRec;

/************************************************************
 *
 * Semi-Public Functions.
 *
 ************************************************************/

/*	Function Name: ClassPartInitialize
 *	Description: handles inheritance of class functions.
 *	Arguments: class - the widget classs of this widget.
 *	Returns: none.
 */

static void
ClassPartInitialize(class)
WidgetClass class;
{
    SmeObjectClass m_ent, superC;

    m_ent = (SmeObjectClass) class;
    superC = (SmeObjectClass) m_ent->rect_class.superclass;

/* 
 * We don't need to check for null super since we'll get to TextSink
 * eventually.
 */

    if (m_ent->sme_class.highlight == XtInheritHighlight) 
	m_ent->sme_class.highlight = superC->sme_class.highlight;

    if (m_ent->sme_class.unhighlight == XtInheritUnhighlight)
	m_ent->sme_class.unhighlight = superC->sme_class.unhighlight;

    if (m_ent->sme_class.notify == XtInheritNotify) 
	m_ent->sme_class.notify = superC->sme_class.notify;
}

/*      Function Name: Initialize
 *      Description: Initializes the simple menu widget
 *      Arguments: request - the widget requested by the argument list.
 *                 new     - the new widget with both resource and non
 *                           resource values.
 *      Returns: none.
 * 
 * MENU ENTRIES CANNOT HAVE BORDERS.
 */

/* ARGSUSED */
static void
Initialize(request, new)
Widget request, new;
{
    SmeObject entry = (SmeObject) new;

    entry->rectangle.border_width = 0;
}

/*	Function Name: Highlight
 *	Description: The default highlight proceedure for menu entries.
 *	Arguments: w - the menu entry.
 *	Returns: none.
 */

/* ARGSUSED */
static void
Highlight(w)
Widget w;
{
/* This space intentionally left blank. */
}

/*	Function Name: Unhighlight
 *	Description: The default unhighlight proceedure for menu entries.
 *	Arguments: w - the menu entry.
 *	Returns: none.
 */

/* ARGSUSED */
static void
Unhighlight(w)
Widget w;
{
/* This space intentionally left blank. */
}

/*	Function Name: Notify
 *	Description: calls the callback proceedures for this entry.
 *	Arguments: w - the menu entry.
 *	Returns: none.
 */

static void
Notify(w) 
Widget w;
{
    XtCallCallbacks(w, XtNcallback, NULL);
}

/*	Function Name: QueryGeometry.
 *	Description: Returns the preferred geometry for this widget.
 *	Arguments: w - the menu entry object.
 *                 itended, return - the intended and return geometry info.
 *	Returns: A Geometry Result.
 *
 * See the Intrinsics manual for details on what this function is for.
 * 
 * I just return the height and a width of 1.
 */

static XtGeometryResult
QueryGeometry(w, intended, return_val) 
Widget w;
XtWidgetGeometry *intended, *return_val;
{
    SmeObject entry = (SmeObject) w;
    Dimension width;
    XtGeometryResult ret_val = XtGeometryYes;
    XtGeometryMask mode = intended->request_mode;

    width = 1;			/* we can be really small. */

    if ( ((mode & CWWidth) && (intended->width != width)) ||
	 !(mode & CWWidth) ) {
	return_val->request_mode |= CWWidth;
	return_val->width = width;
	mode = return_val->request_mode;
	
	if ( (mode & CWWidth) && (width == entry->rectangle.width) )
	    return(XtGeometryNo);
	return(XtGeometryAlmost);
    }
    return(ret_val);
}
