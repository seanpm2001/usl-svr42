/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef NOIDENT
#ident	"@(#)dtadmin:print/setup/remote.h	1.2"
#endif

#ifndef REMOTE_H
#define REMOTE_H

extern Boolean	IsNetworking (void);
extern void	RemoteSystemsCB (Widget, XtPointer, XtPointer);

#endif /* REMOTE_H */
