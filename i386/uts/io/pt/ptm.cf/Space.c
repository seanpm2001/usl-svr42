/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-x86:io/pt/ptm.cf/Space.c	1.3"
#ident	"$Header: $"

#include <config.h>	/* to collect tunable parameters */
#include <sys/types.h>
#include <sys/stream.h>
#include <sys/termios.h>
#include <sys/ptms.h>

int pt_cnt = PTM_UNITS;
struct pt_ttys ptms_tty[PTM_UNITS];
