/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)curses:common/lib/xlibcurses/screen/delterm.c	1.6.2.2"
#ident  "$Header: delterm.c 1.2 91/06/26 $"
#include "curses_inc.h"

/*
 * Relinquish the storage associated with "terminal".
 */
extern	TERMINAL	_first_term;
extern	char		_called_before;
extern	char		_frst_tblstr[];

delterm(terminal)
register	TERMINAL	*terminal;
{
    if (!terminal)
	return (ERR);
    delkeymap(terminal);
    if (terminal->_check_fd >= 0)
	close(terminal->_check_fd);
    if (terminal == &_first_term)
    {
	/* next setupterm can re-use static areas */
	_called_before = FALSE;
	if (terminal->_strtab != _frst_tblstr)
	    free((char *)terminal->_strtab);
    }
    else
    {
	free((char *)terminal->_bools);
	free((char *)terminal->_nums);
	free((char *)terminal->_strs);
	free((char *)terminal->_strtab);
	free((char *)terminal);
    }
    if (terminal->_pairs_tbl)
	free((char *)terminal->_pairs_tbl);
    if (terminal->_color_tbl)
	free((char *)terminal->_color_tbl);
    return (OK);
}
