/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)oamintf:i386/cmd/oamintf/xx/FACE3.2/bin/getword.c	1.2"
#ident	"$Header: $"

main()
{
	short c;
	while ((c=getchar()) != '\n')
		putchar(c);
	putchar('\n');
}
