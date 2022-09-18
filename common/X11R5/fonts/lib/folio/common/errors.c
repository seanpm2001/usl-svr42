/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libfolio1.2:common/errors.c	1.1"

#ifndef lint
static char sccsid[] = "@(#)errors.c 1.3 89/05/11";
#endif
/*
**    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
**   
**                   PROPRIETARY NOTICE (Combined) 
**   
**            This source code is unpublished proprietary 
**            information constituting, or derived under 
**            license from AT&T's UNIX(r) System V. 
**   
**                       Copyright Notice 
**   
**            Notice of copyright on this source code 
**            product does not indicate publication. 
**   
**    Copyright (C) 1986,1987,1988,1989,1990  Sun Microsystems, Inc
**    Copyright (C) 1983,1984,1985,1986,1987,1988,1989,1990  AT&T
**   
**                      All rights reserved. 
**   
**    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
**   
**                    RESTRICTED RIGHTS LEGEND: 
**    Use, duplication, or disclosure by the Government is subject 
**    to restrictions as set forth in subparagraph (c)(1)(ii) of 
**    the Rights in Technical Data and Computer Software clause at 
**    DFARS 52.227-7013 and in similar clauses in the FAR and NASA 
**    FAR Supplement. 
**   
**    +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
*/


#include	<stdio.h>
#include	"cdefs.h"
#include	"errors.h"

#define error_STACKSIZE	((int32)20)

/* how error messages are stored */

typedef struct _error_message {
	char    *errorptr;  /* ptr to string containing error message */
	int8    severity;   /* of the message, FATAL or NOTFATAL */
} error_message;


error_message	error_stack[error_STACKSIZE+1], *error_status;
int32			error_cnt;


/* the errors generated by this module */

char	*error_errs[] = {
	"No errors",
	"Too many errors"
};

#define	error_CLEAR				(error_errs[0])
#define	error_TOOMANYERRORS		(error_errs[1])

/*-------------------------------------------------------------------*/
/*
	Clears all past errors.
*/

void	error_Clear()
{
	error_status = error_stack;
	error_cnt = 0;
}

/*-------------------------------------------------------------------*/

int32	error_GetCount()
{
	return(error_cnt);
}

/*-------------------------------------------------------------------*/
/*
	To report a new error.
*/

void	error_SetStatus(errptr,severity)
char	*errptr;
int8	severity;
{
	if (error_cnt != 0)
		error_status++;
	if (error_cnt < error_STACKSIZE) {
		error_status->errorptr = errptr;
		error_status->severity = severity;
		error_cnt++;
	} else if (error_cnt == error_STACKSIZE) {
		error_status->errorptr = error_TOOMANYERRORS;
		error_status->severity = error_FATAL;
		error_cnt++;
	}
}

/*-------------------------------------------------------------------*/
/*
	This function returns the most recent unread error message or 
	error_CLEAR if no more errors remain.
*/

char	*error_GetNextError()
{
	if (error_cnt > 0) {
		return(error_status--->errorptr);
	} else
		return(error_CLEAR);
}

/*-------------------------------------------------------------------*/
/*
	This function returns the most recent unread error severity or 
	error_NOERROR if no more error severities remain.
*/

int8	error_GetSeverity()
{
	if (error_cnt > 0) {
		return(error_status->severity);
	} else
		return(error_NOERROR);
}

/*-------------------------------------------------------------------*/
/*
	This function returns the most recent unread error severity or 
	error_NOERROR if no more error severities remain. It removes
	the current error from the stack of errors.
*/

int8	error_GetNextSeverity()
{
	if (error_cnt > 0) {
		return(error_status--->severity);
	} else
		return(error_NOERROR);
}

/*-------------------------------------------------------------------*/
