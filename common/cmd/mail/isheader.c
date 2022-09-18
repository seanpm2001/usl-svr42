/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:common/cmd/mail/isheader.c	1.6.2.3"
#ident "@(#)isheader.c	1.8 'attmail mail(1) command'"
#include "mail.h"
/*
    NAME
	isheader - check for header field

    SYNOPSIS
	int isheader(char *line, int *continuation, int sending, int fnuhdrtype)

    DESCRIPTION
	Isheader() looks to see if the line is a header field
	and returns its type.

	Continuation is a flag that indicates if a continuation
	line is allowed. (It should be FALSE the first time
	isheader() is called on a message.  isheader() sets
	it for the remaining calls for that message.)

	sending is an indication of whether we're sending mail
	or not. If so, From/>From headers are disallowed.
	
	Fnuhdrtype is the first non-UNIX From header found. Any "From "
	headers found after that have to be part of the body.

    RETURNS
	FALSE	->	not header line
	H_*     ->	type of header line found.
*/

int isheader(lp, ctfp, sending, fnuhdrtype)
char	*lp;
int	*ctfp;
int	sending;
int	fnuhdrtype;
{
	static const char pn[] = "isheader";
	register char	*p, *q;
	register int	i;

	Dout(pn, 30, "? '%s'\n", lp);
	p = lp;
	while((*p) && (*p != '\n') && (isspace(*p))) {
		p++;
	}
	if((*p == NULL) || (*p == '\n')) {
		/* blank line */
		Dout(pn, 30, "=> 0\n");
		return (FALSE);
	}

	if ((*ctfp) && ((*lp == ' ') || (*lp == '\t'))) {
		Dout(pn, 30, "=> %s\n", header[H_CONT].tag);
		return(H_CONT);
	}

	*ctfp = FALSE;
	for (i = H_FROM; i < H_NAMEVALUE; i++) {
		if (!isit(lp, i)) {
			continue;
		}
		if ((i == H_FROM) || (i == H_FROM1)) {
			/* From and >From must be case sensitive matches. */
			if (strncmp(lp, header[i].tag, strlen(header[i].tag)) != 0) {
				Dout(pn, 30, "case sensitivity check; => 0\n");
				return (FALSE);
			}
			/*
			 * Should NEVER get 'From ' or '>From ' line on stdin
			 * if invoked as mail (rather than rmail) since
			 * 'From ' and/or '>From ' lines are generated by
			 * program itself. Therefore, if it DOES match and
			 * ismail == TRUE, it must be part of the content.
			 */
			if (sending && ismail) {
				Dout(pn, 30, "not for rmail; => 0\n");
				return (FALSE);
			}
			/* If we've already seen another header, this From */
			/* must be part of the body. */
			if (fnuhdrtype != 0) {
				Dout(pn, 30, "not after other headers; => 0\n");
				return (FALSE);
			}
		}
		*ctfp = TRUE;
		Dout(pn, 30, "=> %s\n", header[i].tag);
		return (i);
	}

	/*
	 * Check if name: value pair
 	 */
	if ((p = strpbrk(lp, ":")) != NULL ) {
		for(q = lp; q < p; q++)  {
			if ( (!isalnum(*q)) && (*q != '-') && (*q != '>'))  {
				Dout(pn, 30, "=> 0\n");
				return(FALSE);
			}
		}
		/* the : must be followed by white-space or the end of the line */
		q++;
		if (*q && (*q != ' ') && (*q != '\t') && (*q != '\n') && (*q != '\r')) {
			Dout(pn, 30, "=> 0\n");
			return(FALSE);
		}
		*ctfp = TRUE;
		Dout(pn, 30, "=> %s\n", header[H_NAMEVALUE].tag);
		return(H_NAMEVALUE);
	}
	Dout(pn, 30, "=> 0\n");
	return(FALSE);
}