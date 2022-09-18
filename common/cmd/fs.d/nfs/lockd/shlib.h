/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nfs.cmds:nfs/lockd/shlib.h	1.1.4.4"
#ident	"$Header: $"

/*
 * This header file contains all the macros and definitons
 *  needed for importing symbols for libc_s
 * 
 */

#if SHLIB

#define __ctype	(* _libc__ctype)
#define _numeric	(* _libc__numeric)

#define _getflthw	(* _libc__getflthw)

#define _cleanup (* _libc__cleanup)
#define _environ  (* _libc__environ)		
#define end	 (* _libc_end)
#define _exithandle	 (* _libc__exithandle)

#define malloc	(* _libc_malloc)		
#define free	(* _libc_free)	
#define realloc (* _libc_realloc)


#define _sibuf  (* _libc__sibuf)
#define _sobuf  (* _libc__sobuf)
#define _smbuf  (* _libc__smbuf)
#define __iob 	(* _libc__iob)
#define _lastbuf	(* _libc__lastbuf)
#define _bufendtab	(* _libc__bufendtab)

#define _lib_version	(* _libc__lib_version)

#define strspn	(* _libc_strspn)


#endif
