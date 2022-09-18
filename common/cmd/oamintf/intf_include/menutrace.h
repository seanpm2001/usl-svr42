/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oamintf:common/cmd/oamintf/intf_include/menutrace.h	1.1.4.2"
#ident  "$Header: menutrace.h 2.0 91/07/12 $"

/* define menu search struct */

typedef struct taskdescr {
	int	ident;		/* node counter */
	int	mark;		/* found marker */
	char	*tname;		/* task name */
	char	*action;	/* action field (opt) */
	struct taskdescr *next;	/* sibling in hierarchy */
	struct taskdescr *child; /* child in hierarchy */
	struct taskdescr *parent; /* parent node pointer */
	struct taskdescr *thread; /* thread through hierarchy */
} TASKDESC;
