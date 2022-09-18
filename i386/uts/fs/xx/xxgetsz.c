/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-x86:fs/xx/xxgetsz.c	1.1"
#ident	"$Header: $"
#include <util/types.h>
#include <util/param.h>
#include <fs/vnode.h>
#include <fs/s5fs/s5param.h>
#include <fs/xx/xxfblk.h>
#include <fs/xx/xxfilsys.h>
#include <fs/s5fs/s5ino.h>
#include <fs/s5fs/s5inode.h>
#include <fs/s5fs/s5macros.h>
