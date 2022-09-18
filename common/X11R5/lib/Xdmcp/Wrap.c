/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)r5Xdmcp:Wrap.c	1.2"
/*
 * $XConsortium: Wrap.c,v 1.8 91/09/10 14:34:11 keith Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 *
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Xos.h>
#include <X11/X.h>
#include <X11/Xmd.h>
#include <X11/Xdmcp.h>

#ifdef HASXDMAUTH

/*
 * The following function exists only to demonstrate the
 * desired functional interface for this routine.  You will
 * need to add the appropriate algorithm if you wish to
 * use XDM-AUTHENTICATION-1/XDM-AUTHORIZATION-1.
 *
 * Examine the XDMCP specification for the correct algorithm
 */

#include "Wrap.h"

void
XdmcpWrap (input, wrapper, output, bytes)
    unsigned char	*input, *output;
    unsigned char	*wrapper;
    int			bytes;
{
    int			i, j;
    int			len;
    unsigned char	tmp[8];
    unsigned char	expand_wrapper[8];
    auth_wrapper_schedule	schedule;

    _XdmcpWrapperToOddParity (wrapper, expand_wrapper);
    _XdmcpAuthSetup (expand_wrapper, schedule);
    for (j = 0; j < bytes; j += 8)
    {
	len = 8;
	if (bytes - j < len)
	    len = bytes - j;
	/* block chaining */
	for (i = 0; i < len; i++)
	{
	    if (j == 0)
		tmp[i] = input[i];
	    else
		tmp[i] = input[j + i] ^ output[j - 8 + i];
	}
	for (; i < 8; i++)
	{
	    if (j == 0)
		tmp[i] = 0;
	    else
		tmp[i] = 0 ^ output[j - 8 + i];
	}
	_XdmcpAuthDoIt (tmp, (output + j), schedule, 1);
    }
}

/*
 * Given a 56 bit wrapper in XDMCP format, create a 56
 * bit wrapper in 7-bits + odd parity format
 */

static int
OddParity (c)
    unsigned char   c;
{
    c = c ^ (c >> 4);
    c = c ^ (c >> 2);
    c = c ^ (c >> 1);
    return ~c & 0x1;
}

/*
 * Spread the 56 bit wrapper among 8 bytes, using the upper 7 bits
 * of each byte, and storing an odd parity bit in the low bit
 */

void
_XdmcpWrapperToOddParity (in, out)
    unsigned char   *in, *out;
{
    int		    ashift, bshift;
    int		    i;
    unsigned char   c;

    ashift = 7;
    bshift = 1;
    for (i = 0; i < 7; i++)
    {
	c = ((in[i] << ashift) | (in[i+1] >> bshift)) & 0x7f;
	out[i] = (c << 1) | OddParity (c);
	ashift--;
	bshift++;
    }
    c = in[i];
    out[i] = (c << 1) | OddParity(c);
}

#endif
