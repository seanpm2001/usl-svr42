/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)r5Xdmcp:WA32.c	1.2"
/*
 * $XConsortium: WA32.c,v 1.3 91/01/23 22:14:30 gildea Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 *
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

int
XdmcpWriteARRAY32 (buffer, array)
    XdmcpBufferPtr  buffer;
    ARRAY32Ptr	    array;
{
    int	i;

    if (!XdmcpWriteCARD8 (buffer, array->length))
	return FALSE;
    for (i = 0; i < (int)array->length; i++)
	if (!XdmcpWriteCARD32 (buffer, array->data[i]))
	    return FALSE;
    return TRUE;
}
