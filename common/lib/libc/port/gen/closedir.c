/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/closedir.c	1.11"
/*
	closedir -- C library extension routine

*/

#ifdef __STDC__
	#pragma weak closedir = _closedir
#endif
#include "synonyms.h"
#include "shlib.h"
#include	<sys/types.h>
#include	<dirent.h>
#include	<unistd.h>
#include	<malloc.h>

int
closedir( dirp )
register DIR	*dirp;		/* stream from opendir() */
{
	register int 	tmp_fd = dirp->dd_fd;

	free( dirp->dd_buf );
	free( (char *)dirp );
	return(close( tmp_fd ));
}
