/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* utimbuf is used by utime(2) */

#ifndef _UTIME_H
#define _UTIME_H

#ident	"@(#)sgs-head:utime.h	1.3"

#include <sys/utime.h>

#if defined(__STDC__)
extern int utime(const char *, const struct utimbuf *);
#else
extern int utime();
#endif /* __STDC__ */

#endif /* _UTIME_H */
