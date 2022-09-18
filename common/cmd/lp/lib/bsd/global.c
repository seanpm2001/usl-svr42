/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:lib/bsd/global.c	1.2.5.3"
#ident	"$Header: $"

#include "msgs.h"

char	*Lhost;		/* Local host name */
char	*Rhost;		/* Remote host name */
char	*Name;		/* Program name */
char	*Printer;	/* Printer name */
char	 Msg[MSGMAX];	/* Message buffer */
