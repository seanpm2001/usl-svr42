/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)curses:common/lib/xlibcurses/screen/mbcharlen.c	1.2.2.2"
#ident  "$Header: mbcharlen.c 1.2 91/06/26 $"
#include	"curses_inc.h"

/*
**	Get the # of valid characters
*/
mbcharlen(sp)
char	*sp;
{
	reg int		n, m, k, ty;
	reg chtype	c;

	n = 0;
	for(; *sp != '\0'; ++sp, ++n)
		if(ISMBIT(*sp))
		{
			c = RBYTE(*sp);
			ty = TYPE(c & 0377);
			m  = cswidth[ty] - (ty == 0 ? 1 : 0);
			for(sp += 1, k = 1; *sp != '\0' && k <= m; ++k, ++sp)
			{
				c = RBYTE(*sp);
				if(TYPE(c) != 0)
					break;
			}
			if(k <= m)
				break;
		}
	return n;
}
