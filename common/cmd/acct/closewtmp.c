/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)acct:common/cmd/acct/closewtmp.c	1.2.7.3"
#ident "$Header: $"

/*	fudge an entry to wtmp for each user who is still logged on when
 *	acct is being run. This entry marks a DEAD_PROCESS, and the
 *	current time as time stamp. This should be done before connect
 *	time is processed. Called by runacct.
 */

#include <stdio.h>
#include <sys/types.h>
#include <utmp.h>

main(argc, argv)
int	argc;
char	**argv;
{
	struct utmp *getutent(), *utmp;
	FILE *fp;

	if ((fp = fopen(WTMP_FILE, "a+")) == NULL) {
		fprintf(stderr, "%s: cannot open %s for writing\n",
			argv[0], WTMP_FILE);
		exit(2);
	}
	while ((utmp=getutent()) != NULL) {
		if (utmp->ut_type == USER_PROCESS) {
			utmp->ut_type = DEAD_PROCESS;
			time( &utmp->ut_time );
			fwrite( utmp, sizeof(*utmp), 1, fp);
		}
	}
	fclose(fp);
}
