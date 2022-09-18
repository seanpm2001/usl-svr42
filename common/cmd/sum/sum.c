/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sum:sum.c	1.7.1.2"
#ident "$Header: /sms/sinixV5.4es/rcs/s19-full/usr/src/cmd/sum/sum.c,v 1.1 91/02/28 20:11:20 ccs Exp $"
/*
 * Sum bytes in file mod 2^16
 */


#define WDMSK 0177777L
#define BUFSIZE 512
#include <stdio.h>
#include <locale.h>
#include <pfmt.h>
#include <errno.h>
#include <string.h>
struct part {
	short unsigned hi,lo;
};
union hilo { /* this only works right in case short is 1/2 of long */
	struct part hl;
	long	lg;
} tempa, suma;

main(argc,argv)
char **argv;
{
	register unsigned sum;
	register i, c;
	register FILE *f;
	register long nbytes;
	int	alg, ca, errflg = 0;
	unsigned lsavhi,lsavlo;

	(void)setlocale(LC_ALL, "");
	(void)setcat("uxdfm");
	(void)setlabel("UX:sum");

	alg = 0;
	i = 1;
	if((argc > 1) && (argv[1][0]=='-' && argv[1][1]=='r')) {
		alg = 1;
		i = 2;
	}

	do {
		if(i < argc) {
			if((f = fopen(argv[i], "r")) == NULL) {
				(void) pfmt(stderr, MM_ERROR, 
					":3:Cannot open %s: %s\n",
					argv[i], strerror(errno));
				errflg += 10;
				continue;
			}
		} else
			f = stdin;
		sum = 0;
		suma.lg = 0;
		nbytes = 0;
		if(alg == 1) {
			while((c = getc(f)) != EOF) {
				nbytes++;
				if(sum & 01)
					sum = (sum >> 1) + 0x8000;
				else
					sum >>= 1;
				sum += c;
				sum &= 0xFFFF;
			}
		} else {
			while((ca = getc(f)) != EOF) {
				nbytes++;
				suma.lg += ca & WDMSK;
			}
		}
		if(ferror(f)) {
			errflg++;
			if (argc > 1)
				(void) pfmt(stderr, MM_ERROR, 
					":59:Read error on %s: %s\n",
					argv[i], strerror(errno));
			else
				(void) pfmt(stderr, MM_ERROR,
					":60:Read error on stdin: %s\n",
					strerror(errno));
		}
		if (alg == 1)
			(void) printf("%.5u%6ld", sum, (nbytes+BUFSIZE-1)/BUFSIZE);
		else {
			tempa.lg = (suma.hl.lo & WDMSK) + (suma.hl.hi & WDMSK);
			lsavhi = (unsigned) tempa.hl.hi;
			lsavlo = (unsigned) tempa.hl.lo;
			(void) printf("%u %ld", (unsigned)(lsavhi + lsavlo), (nbytes+BUFSIZE-1)/BUFSIZE);
		}
		if(argc > 1)
			(void) printf(" %s", argv[i]==(char *)0?"":argv[i]);
		(void) printf("\n");
		(void) fclose(f);
	} while(++i < argc);
	exit(errflg);
}