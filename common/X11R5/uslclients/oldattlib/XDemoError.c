/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	NOIDENT
#ident	"@(#)oldattlib:XDemoError.c	1.1"
#endif
/*
 XDemoError.c (C source file)
	Acc: 575043158 Tue Mar 22 09:12:38 1988
	Mod: 570895207 Wed Feb  3 09:00:07 1988
	Sta: 573929825 Wed Mar  9 11:57:05 1988
	Owner: 2011
	Group: 1985
	Permissions: 664
*/
/*
	START USER STAMP AREA
*/
/*
	END USER STAMP AREA
*/
/* XDemoError - Fatal demo error.  */
#include <stdio.h>

XDemoError (pgm,identifier)
char * pgm;
char * identifier;
{

fprintf(stderr,"%s: XDemoError - Fatal demo error encountered.\n",pgm);
perror(pgm);
fprintf(stderr, "%s: %s\n", pgm, identifier);
	
exit(1);
} /* end of XDemoError */
