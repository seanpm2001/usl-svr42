/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

#ident	"@(#)sh:i386/cmd/sh/echo.c	1.6.6.2"
#ident "$Header: echo.c 1.2 91/03/19 $"
/*
 *	UNIX shell
 */
#include	"defs.h"

#define	exit(a)	flushb();return(a)

extern int exitval;

echo(argc, argv)
unsigned char **argv;
{
	register unsigned char	*cp;
	register int	i, wd;
	int	j;
	int	nonl = 0;
	
	if (ucb_builtins) {
		register int nflg;

		nflg = 0;
                if(argc > 1 && argv[1][0] == '-' && argv[1][1] == 'n') {
                        nflg++;
                        argc--;
                        argv++;
                }
                for(i=1; i<argc; i++) {
                        sigchk();
                        prs_buff(argv[i]);
                        if (i < argc-1)
                                prc_buff(' ');
                }
                if(nflg == 0)
                        prc_buff('\n');
                exit(0);
        }
	else {	
		if(--argc == 0) {
			prc_buff('\n');
			exit(0);
		}

		if (!strncmp(argv[1], "-n", 3)) {
			nonl++;
			*++argv;
			argc--;
		}
	
		for(i = 1; i <= argc; i++) 
		{
			sigchk();
			for(cp = argv[i]; *cp; cp++) 
			{
				if(*cp == '\\')
				switch(*++cp) 
				{
					case 'b':
						prc_buff('\b');
						continue;
	
					case 'c':
						exit(0);
	
					case 'f':
						prc_buff('\f');
						continue;
	
					case 'n':
						prc_buff('\n');
						continue;
	
					case 'r':
						prc_buff('\r');
						continue;
	
					case 't':
						prc_buff('\t');
						continue;
	
					case 'v':
						prc_buff('\v');
						continue;
	
					case '\\':
						prc_buff('\\');
						continue;
					case '0':
						j = wd = 0;
						while ((*++cp >= '0' && *cp <= '7') && j++ < 3) {
							wd <<= 3;
							wd |= (*cp - '0');
						}
						prc_buff(wd);
						--cp;
						continue;
	
					default:
						cp--;
				}
				prc_buff(*cp);
			}

			if (!nonl || i != argc)
				prc_buff(i == argc? '\n': ' ');

		}
		exit(0);
	}
}
