/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ucb:common/ucbcmd/plot/libplot/hp2648/label.c	1.1"
#ident	"$Header: $"

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */


#include "hp2648.h"

label(s)
char *s;
{
	handshake();
	putchar(ESC);
	putchar(GRAPHIC);
	putchar('l');
	for(;*s!='\0';s++)
		putchar(*s);
	putchar(ESC);
	putchar(GRAPHIC);
	putchar('d');
	putchar('T');
	handshake();
	putchar(ESC);
	putchar(GRAPHIC);
	putchar(PLOT);
	putchar(BINARY);
	buffcount = 4;
}