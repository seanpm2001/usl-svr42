/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ifndef _IO_LP_LP_H	/* wrapper symbol for kernel use */
#define _IO_LP_LP_H	/* subject to change without notice */

#ident	"@(#)uts-x86at:io/lp/lp.h	1.2"
#ident	"$Header: $"

#define SPL()		splstr()/* protection from interrupts */


#define	UNBUSY		0x80
#define	READY		0x40
#define	NOPAPER		0x20
#define	ONLINE		0x10
#define	ERROR		0x08

#define	INTR_ON		0x10
#define	SEL		0x08
#define	RESET		0x04
#define	AUTOLF		0x02
#define	STROBE		0x01

/* States: */
#define OPEN	0x01
#define LPPRES	0x10    /* set if parallel adapter present */


/*
 * Structures for the LP 
 * ____________________________
 *
 *
 */

struct lpcfg{
	int		flag;		/* lp is configured in */
	unsigned	data;		/* data latch address */
	unsigned	status;		/* printer status address */
	unsigned	control;	/* printer controls address */
	unsigned	vect;		/* printer controls address */
};

#endif	/* _IO_LP_LP_H */
