/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _STDARG_H
#define _STDARG_H

#ident	"@(#)debugger:inc/CC/stdarg.h	1.2"

typedef void *va_list;

#define va_start(list, name) (void) (list = \
 	(void *)((char *)&name + ((sizeof(name)+(sizeof(int)-1)) & ~(sizeof(int)-1))))

#if #machine(u370)
#define va_arg(list, mode) ((mode *)(list = \
        (char *) ((int)list + 2*sizeof(mode) - 1 & -sizeof(mode))))[-1]
#else
#define va_arg(list, mode) ((mode *)(list = (char *)list + sizeof(mode)))[-1]
#endif	/* u370 */

extern void va_end(va_list);

#define va_end(list) (void)0

#endif 	/* _STDARG_H */
