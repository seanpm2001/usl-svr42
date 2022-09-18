/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-x86:svc/hrt.cf/Space.c	1.3"
#ident	"$Header: $"

#include <config.h>
#include <sys/types.h>
#include <sys/hrtsys.h>

uint	timer_resolution = 100;

timer_t	hrtimes[HRTIME];
int	hrtimes_size = HRTIME;

timer_t	itimes[HRVTIME];
int	itimes_size = HRVTIME;
