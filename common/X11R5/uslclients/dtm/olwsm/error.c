/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/* ident	"@(#)dtm:olwsm/error.c	1.3" */

/*
 *************************************************************************
 *
 * Description:
 *		The error diagnostic functions are held in this file.
 *	There are two types of error procedures and two warning procedures.
 *
 *		OlError()		- is a simple fatal error handler
 *		OlWarning()		- is a simple non-fatal error handler
 *		OlVaDisplayErrorMsg()	- Variable argument list fatal
 *					  error handler
 *		OlVaDisplayWarningMsg() - Variable argument list non-fatal
 *					  error handler
 *
 *******************************file*header*******************************
 */

						/* #includes go here	*/

#include <stdio.h>
#include <string.h>
#if defined(__STDC__) || defined(c_plusplus) || defined(__cplusplus)
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <X11/IntrinsicP.h>

#include "wsmmsgstrs"
#include "error.h"

