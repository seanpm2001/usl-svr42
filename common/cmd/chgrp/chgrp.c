/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*	Portions Copyright (c) 1988, Sun Microsystems, Inc.	*/
/*	All Rights Reserved.					*/

#ident	"@(#)chgrp:chgrp.c	1.9.2.1"
#ident "$Header: /sms/sinixV5.4es/rcs/s19-full/usr/src/cmd/chgrp/chgrp.c,v 1.1 91/02/28 16:28:00 ccs Exp $"

/*
 * chgrp  [-h] [-R] gid file ...
 */

/***************************************************************************
 * Command: chgrp
 * Inheritable Privileges: P_OWNER,P_MACREAD,P_MACWRITE,P_DACREAD
 *       Fixed Privileges: None
 * Notes: change the group ownership of a file
 *
 ***************************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <grp.h>
#include <dirent.h>
#include <unistd.h>
#include <priv.h>
#include <locale.h>
#include <pfmt.h>
#include <string.h>
#include <errno.h>

struct	group	*gr;
struct	stat	stbuf;
gid_t	gid;
char	*group;
int	hflag = 0;
int	status;
int 	acode = 0;
extern  int optind;

/*
 * Procedure:     main
 *
 * Restrictions:
                 setlocale:	none
                 getopt:	none
                 pfmt:		none
                 getgrnam:	P_MACREAD
                 lstat(2):	none
                 stat(2):	none
                 lchown(2):	none
                 chown(2):	none
*/
main(argc, argv)
int  argc;
char *argv[];
{
	register c;
	int rflag = 0;
	int errflg = 0;

	(void)setlocale(LC_ALL, "");
	(void)setcat("uxcore.abi");
	(void)setlabel("UX:chgrp");

	while ((c = getopt(argc, argv, "hR")) != EOF)
		switch(c) {
			case 'R':
				rflag++;
				break;
			case 'h':
				hflag++;
				break;
			default:
				errflg++;
				break;
		}

	/*
	 * Check for sufficient arguments
	 * or a usage error.
	 */
	argc -= optind;
	argv = &argv[optind];
	if (argc < 2 || errflg) {
		if (!errflg)
			pfmt(stderr, MM_ERROR, ":8:Incorrect usage\n");
		pfmt(stderr, MM_ACTION, ":1115:Usage: chgrp [-h] [-R] gid file ...\n");
		exit(4);
	}

	if ((group = strdup(argv[0])) == NULL) {
		(void) Perror("strdup");
		exit(255);
	}

	/*
	 * If group given is not numeric,
	 * get gid from /etc/group.
	 */
	if(isnumber(group)) {
		gid = atol(group); /* gid_t is long */
	}
	else {
		if((gr=getgrnam(group)) == NULL) {
			pfmt(stderr, MM_ERROR, ":10:Unknown group: %s\n", group);
			exit(4);
		}
		gid = gr->gr_gid;
	}

	for(c=1; c<argc; c++) {

		/* do stat for directory arguments */
		if (hflag) {
			if (lstat(argv[c], &stbuf) < 0) {
				status = Perror(argv[c]);
				continue;
			}
		} else {
			if (stat(argv[c], &stbuf) < 0) {
				status = Perror(argv[c]);
				continue;
			}
		}
		if (rflag && ((stbuf.st_mode & S_IFMT) == S_IFDIR)) {
			status += chownr(argv[c], stbuf.st_uid, gid);
			continue;
		}
		if (hflag) {
			if (lchown(argv[c], stbuf.st_uid, gid) < 0) {
				status = ChownPerror(argv[c]);
			}
		} else {
			if (chown(argv[c], stbuf.st_uid, gid) < 0) {
				status = ChownPerror(argv[c]);
			}
		}
	}
	exit(status += acode);
	/* NOTREACHED */
}

/*
 * Procedure:     chownr
 *
 * Restrictions:
                 getcwd:	none
                 lchown(2):	none
                 chown(2):	none
                 chdir(2):	none
                 opendir:	none
                 lstat(2):	none
                 stat(2):	none
                 pfmt:		none
*/
chownr(dir, uid, gid)
	char *dir;
	uid_t uid;
	gid_t gid;
{
	register struct dirent *dp;
	register DIR *dirp;
	struct stat st;
	char savedir[1024];
	int ecode = 0;

	if (getcwd(savedir,1024) == 0) {
		(void) Perror("getcwd");
		exit(255);
	}

	/*
	 * Change what we are given before doing its contents.
	 */
	if (chown(dir, uid, gid) < 0) 
		return (ChownPerror(dir));
	
	if (chdir(dir) < 0) 
		return(Perror(dir));

	if ((dirp = opendir(".")) == NULL)
		return(Perror(dir));

	dp = readdir(dirp);
	dp = readdir(dirp); /* read "." and ".." */
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
		if (hflag) {
			if (lstat(dp->d_name, &st) < 0) {
				ecode += Perror(dp->d_name);
				break;
			}
		} else {
			if (stat(dp->d_name, &st) < 0) {
				ecode += Perror(dp->d_name);
				break;
			}
		}

		if ((st.st_mode & S_IFMT) == S_IFDIR) {
			acode += chownr(dp->d_name, st.st_uid, gid);
			continue;
		}
		if (hflag) {
			if (lchown(dp->d_name, stbuf.st_uid, gid) < 0) {
				acode += ChownPerror(dp->d_name);
			}
		} else {
			if (chown(dp->d_name, stbuf.st_uid, gid) < 0) {
				acode += ChownPerror(dp->d_name);
			}
		}
	}
	closedir(dirp);
	if (chdir(savedir) < 0) {
		pfmt(stderr, MM_ERROR, ":11:Cannot change back to %s: %s\n",
			savedir, strerror(errno));
		exit(255);
	}
	return (ecode);
}


isnumber(s)
char *s;
{
	register c;

	while(c = *s++)
		if(!isdigit(c))
			return(0);
	return(1);
}	

/*
 * Procedure:     Perror
 *
 * Restrictions:
 *                pfmt:	none
 */
Perror(s)
	char *s;
{
	pfmt(stderr, MM_ERROR, ":12:%s: %s\n", s, strerror(errno));
	return(1);
}

/*
 * Procedure:     ChownPerror
 *
 * Note: This routine specifically prints out error messages for
 *	 chown(2) or lchown(2).  Depending on the error, the 
 *	 routine either returns or exits.
 *
 * Restrictions:
 *               pfmt: 		none
 *               strerror: 	none
 */
ChownPerror(s)
char *s;
{
	/*
	 * EINVAL indicates that the numeric id given is out of range.
	 * Print the id and error message.  No use repeating
	 * error for each call to chown() and thus, exit now.
	 */
	if (errno == EINVAL) {
		pfmt(stderr, MM_ERROR, ":12:%s: %s\n", group, strerror(errno));
		exit(1);
	}
	pfmt(stderr, MM_ERROR, ":12:%s: %s\n", s, strerror(errno));
	return(1);
}

