/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-head:dlfcn.h	1.3"

#ifndef _DLFCN_H
#define _DLFCN_H

/* declarations used for dynamic linking support routines */

#ifdef __STDC__
extern void *dlopen(const char *, int );
extern void *dlsym(void *, const char *);
extern int dlclose(void *);
extern char *dlerror(void);
#else
extern void *dlopen();
extern void *dlsym();
extern int dlclose();
extern char *dlerror();
#endif

/* valid values for mode argument to dlopen */

#define RTLD_LAZY	1	/* lazy function call binding */
#define RTLD_NOW	2	/* immediate function call binding */

#endif  /* _DLFCN_H */
