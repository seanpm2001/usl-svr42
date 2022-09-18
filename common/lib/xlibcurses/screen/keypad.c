/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)curses:common/lib/xlibcurses/screen/keypad.c	1.8.2.3"
#ident  "$Header: keypad.c 1.2 91/06/26 $"

#include	"curses_inc.h"

/* TRUE => special keys should be passed as a single value by getch. */

keypad(win, bf)
WINDOW	*win;
bool	bf;
{
#ifdef __STDC__
    extern  int     _outch(int);
#else
    extern  int     _outch();
#endif
    /*
     * Since _use_keypad is a bit and not a bool, we must check
     * bf, in case someone uses an odd number instead of 1 for TRUE
     */

    win->_use_keypad = (bf) ? TRUE : FALSE;
    if (bf && (!SP->kp_state))
    {
	tputs(keypad_xmit, 1, _outch);
	(void) fflush(SP->term_file);
	SP->kp_state = TRUE;
	return (setkeymap());
    }
    return (OK);
}
