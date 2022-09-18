/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:hdr/sigretro.h	1.7.2.1"
#ident "@(#)sigretro.h	1.6 'attmail mail(1) command'"

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Define extra stuff not found in pre-SVr4 signal.h
 */

#ifndef SIG_HOLD

#define	SIG_HOLD	(int (*)()) 3		/* Phony action to hold sig */
#endif /* SIG_HOLD */
