/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1991, 1992  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION CONFIDENTIAL INFORMATION	*/

/*	This software is supplied to USL under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)cdfs.cmds:cdfs/cdvd/cdvd.h	1.4"
#ident	"$Header: $"

/* Tabstops: 4 */

/*
 * Local include file for cdvd(1M) utility.
 */

#include <sys/cdrom.h>
#include <sys/types.h>


/*
 * Global variables:
 */
static uchar_t		*ProgName;						/* Program name			*/
static char			*Msg;							/* Error message pointer*/
static char			*File;							/* File to get DREC from*/
static boolean_t	Binary = B_FALSE;				/* Binary output?		*/
static char			Buf[CD_PVDLEN];					/* PVD buffer			*/


/*
 * Local definitions.
 */
#define ONLY_NOTIFY			0
#define EXIT_CODE_1			1
#define EXIT_CODE_2			2

#define SHOW_USAGE			12
#define NO_USAGE			13

#define UNKNOWN_EXIT_STATUS	0


/*
 * Function prototypes:
 */
#ifdef __STDC__

static void			DoLocale ();
static void			ParseOpts (int, char * []);
static void			DispUsage ();
static void			ErrMsg (long, uint_t, uint_t, const char *, void *,
								void *, void *);
static void			InterpretFailure (uint_t);
static void			GetPVD ();
static void			PrintPVD ();

#else

static void			DoLocale ();
static void			ParseOpts ();
static void			DispUsage ();
static void			ErrMsg ();
static void			InterpretFailure ();
static void			GetPVD ();
static void			PrintPVD ();

#endif		/* __STDC__ */
