/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)R5Xlib:XMapRaised.c	1.1"
/* $XConsortium: XMapRaised.c,v 1.9 91/01/06 11:46:57 rws Exp $ */
/* Copyright    Massachusetts Institute of Technology    1986	*/

/*
*/

#include "Xlibint.h"

XMapRaised (dpy, w)
    Window w;
    register Display *dpy;
{
    register xConfigureWindowReq *req;
    register xResourceReq *req2;
    unsigned long val = Above;		/* needed for macro */

    LockDisplay(dpy);
    GetReqExtra(ConfigureWindow, 4, req);
    req->window = w;
    req->mask = CWStackMode;
    OneDataCard32 (dpy, NEXTPTR(req,xConfigureWindowReq), val);
    GetResReq (MapWindow, w, req2);
    UnlockDisplay(dpy);
    SyncHandle();
}

