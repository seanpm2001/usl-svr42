/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:cmd/lpadmin/ismodel.c	1.4.4.3"
#ident	"$Header: $"

#include "lp.h"
#include "lpadmin.h"

extern int		Access();

int			ismodel (name)
	char			*name;
{
	if (!name || !*name)
		return (0);

	return (Access(makepath(Lp_Model, name, (char *)0), 04) != -1);
}
