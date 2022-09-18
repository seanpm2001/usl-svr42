/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)sfs.cmds:sfs/fsck/pass2.c	1.3.3.4"
#ident "$Header: /sms/sinixV5.4es/rcs/s19-full/usr/src/cmd/fs.d/sfs/fsck/pass2.c,v 1.1 91/02/28 17:27:00 ccs Exp $"

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <acl.h>
#include <sys/mntent.h>
#include <sys/fs/sfs_fs.h>
#include <sys/vnode.h>
#include <sys/fs/sfs_inode.h>
#define _KERNEL
#include <sys/fs/sfs_fsdir.h>
#undef _KERNEL
#include <sys/mnttab.h>
#include <string.h>
#include "fsck.h"

int	pass2check();
static int pass2sechk();


/*
 * Procedure:     pass2
 *
 * Restrictions:
 *                printf: none
*/

pass2()
{
	register DINODE *dp;
	struct inodesc rootdesc;

	bzero((char *)&rootdesc, sizeof(struct inodesc));
	rootdesc.id_type = ADDR;
	rootdesc.id_func = pass2check;
	rootdesc.id_secfunc = 0;
	rootdesc.id_number = SFSROOTINO;
	pathp = pathname;
	switch (statemap[SFSROOTINO]) {

	case USTATE:
		pfatal("ROOT INODE UNALLOCATED");
		if (reply("ALLOCATE") == 0)
			errexit("");
		if (allocdir(SFSROOTINO, SFSROOTINO) != SFSROOTINO)
			errexit("CANNOT ALLOCATE ROOT INODE\n");
		descend(&rootdesc, SFSROOTINO);
		break;

	case DCLEAR:
		pfatal("DUPS/BAD IN ROOT INODE");
		if (reply("REALLOCATE")) {
			printf("pass2: DCLEAR\n");
			freeino(SFSROOTINO);
			if (allocdir(SFSROOTINO, SFSROOTINO) != SFSROOTINO)
				errexit("CANNOT ALLOCATE ROOT INODE\n");
			descend(&rootdesc, SFSROOTINO);
			break;
		}
		if (reply("CONTINUE") == 0)
			errexit("");
		statemap[SFSROOTINO] = DSTATE;
		descend(&rootdesc, SFSROOTINO);
		break;

	case FSTATE:
	case FCLEAR:
		pfatal("ROOT INODE NOT DIRECTORY");
		if (reply("REALLOCATE")) {
			printf("pass2: FSTATE/FCLEAR\n");
			freeino(SFSROOTINO);
			if (allocdir(SFSROOTINO, SFSROOTINO) != SFSROOTINO)
				errexit("CANNOT ALLOCATE ROOT INODE\n");
			descend(&rootdesc, SFSROOTINO);
			break;
		}
		if (reply("FIX") == 0)
			errexit("");
		dp = ginode(SFSROOTINO);
		dp->di_mode &= ~IFMT;
		dp->di_mode |= IFDIR;
		dp->di_smode = dp->di_mode;
		inodirty();
		statemap[SFSROOTINO] = DSTATE;
		/* fall into ... */

	case DSTATE:
		descend(&rootdesc, SFSROOTINO);
		break;

	default:
		errexit("BAD STATE %d FOR ROOT INODE", statemap[SFSROOTINO]);
	}
}


/*
 * Procedure:     pass2check
 *
 * Restrictions:
 *                printf: none
*/

pass2check(idesc)
	struct inodesc *idesc;
{
	register DIRECT *dirp = idesc->id_dirp;
	char *curpathloc;
	int n, entrysize, ret = 0;
	DINODE *dp;
	DIRECT proto;
	char namebuf[BUFSIZ];

	/* 
	 * check for "."
	 */
	if (idesc->id_entryno != 0)
		goto chk1;
	if (dirp->d_ino != 0 && strcmp(dirp->d_name, ".") == 0) {
		if (dirp->d_ino != idesc->id_number) {
			direrr(idesc->id_number, "BAD INODE NUMBER FOR '.'");
			dirp->d_ino = idesc->id_number;
			if (reply("FIX") == 1)
				ret |= ALTERED;
		}
		goto chk1;
	}
	direrr(idesc->id_number, "MISSING '.'");
	proto.d_ino = idesc->id_number;
	proto.d_namlen = 1;
	(void)strcpy(proto.d_name, ".");
	entrysize = DIRSIZ(&proto);
	if (dirp->d_ino != 0 && strcmp(dirp->d_name, "..") != 0) {
		pfatal("CANNOT FIX, FIRST ENTRY IN DIRECTORY CONTAINS %s\n",
			dirp->d_name);
	} else if ((int)dirp->d_reclen < entrysize) {
		pfatal("CANNOT FIX, INSUFFICIENT SPACE TO ADD '.'\n");
	} else if ((int)dirp->d_reclen < 2 * entrysize) {
		proto.d_reclen = dirp->d_reclen;
		bcopy((char *)&proto, (char *)dirp, entrysize);
		if (reply("FIX") == 1)
			ret |= ALTERED;
	} else {
		n = dirp->d_reclen - entrysize;
		proto.d_reclen = entrysize;
		bcopy((char *)&proto, (char *)dirp, entrysize);
		idesc->id_entryno++;
		lncntp[dirp->d_ino]--;
		dirp = (DIRECT *)((char *)(dirp) + entrysize);
		bzero((char *)dirp, n);
		dirp->d_reclen = n;
		if (reply("FIX") == 1)
			ret |= ALTERED;
	}
chk1:
	if (idesc->id_entryno > 1)
		goto chk2;
	proto.d_ino = idesc->id_parent;
	proto.d_namlen = 2;
	(void)strcpy(proto.d_name, "..");
	entrysize = DIRSIZ(&proto);
	if (idesc->id_entryno == 0) {
		n = DIRSIZ(dirp);
		if ((int)dirp->d_reclen < n + entrysize)
			goto chk2;
		proto.d_reclen = dirp->d_reclen - n;
		dirp->d_reclen = n;
		idesc->id_entryno++;
		lncntp[dirp->d_ino]--;
		dirp = (DIRECT *)((char *)(dirp) + n);
		bzero((char *)dirp, n);
		dirp->d_reclen = n;
	}
	if (dirp->d_ino != 0 && strcmp(dirp->d_name, "..") == 0) {
		if (dirp->d_ino != idesc->id_parent) {
			direrr(idesc->id_number, "BAD INODE NUMBER FOR '..'");
			dirp->d_ino = idesc->id_parent;
			if (reply("FIX") == 1)
				ret |= ALTERED;
		}
		goto chk2;
	}
	direrr(idesc->id_number, "MISSING '..'");
	if (dirp->d_ino != 0 && strcmp(dirp->d_name, ".") != 0) {
		pfatal("CANNOT FIX, SECOND ENTRY IN DIRECTORY CONTAINS %s\n",
			dirp->d_name);
	} else if ((int)dirp->d_reclen < entrysize) {
		pfatal("CANNOT FIX, INSUFFICIENT SPACE TO ADD '..'\n");
	} else {
		proto.d_reclen = dirp->d_reclen;
		bcopy((char *)&proto, (char *)dirp, entrysize);
		if (reply("FIX") == 1)
			ret |= ALTERED;
	}
chk2:
	if (dirp->d_ino == 0)
		return (ret|KEEPON);
	if (dirp->d_namlen <= 2 &&
	    dirp->d_name[0] == '.' &&
	    idesc->id_entryno >= 2) {
		if (dirp->d_namlen == 1) {
			direrr(idesc->id_number, "EXTRA '.' ENTRY");
			dirp->d_ino = 0;
			if (reply("FIX") == 1)
				ret |= ALTERED;
			return (KEEPON | ret);
		}
		if (dirp->d_name[1] == '.') {
			direrr(idesc->id_number, "EXTRA '..' ENTRY");
			dirp->d_ino = 0;
			if (reply("FIX") == 1)
				ret |= ALTERED;
			return (KEEPON | ret);
		}
	}
	curpathloc = pathp;
	*pathp++ = '/';
	if (pathp + dirp->d_namlen >= endpathname) {
		*pathp = '\0';
		errexit("NAME TOO LONG %s%s\n", pathname, dirp->d_name);
	}
	bcopy(dirp->d_name, pathp, dirp->d_namlen + 1);
	pathp += dirp->d_namlen;
	idesc->id_entryno++;
	n = 0;
	if (dirp->d_ino > imax || dirp->d_ino <= 0) {
		direrr(dirp->d_ino, "I OUT OF RANGE");
		n = reply("REMOVE");
	} else {
again:
		switch (statemap[dirp->d_ino]) {
		case USTATE:
			direrr(dirp->d_ino, "UNALLOCATED");
			n = reply("REMOVE");
			break;

		case DCLEAR:
		case FCLEAR:
			direrr(dirp->d_ino, "DUP/BAD");
			if ((n = reply("REMOVE")) == 1)
				break;
			dp = ginode(dirp->d_ino);
			statemap[dirp->d_ino] = DIRCT(dp) ? DSTATE : FSTATE;
			goto again;

		case DFOUND:
			if (idesc->id_entryno > 2) {
				getpathname(namebuf, dirp->d_ino, dirp->d_ino);
				pwarn("%s %s %s\n", pathname,
				    "IS AN EXTRANEOUS HARD LINK TO DIRECTORY",
				    namebuf);
				if (preen)
					printf(" (IGNORED)\n");
				else if ((n = reply("REMOVE")) == 1)
					break;
			}
			/* fall through */

		case FSTATE:
                       	if ((n = pass2sechk(dirp->d_ino)) == 1)
                                break;
			lncntp[dirp->d_ino]--;
			break;

		case DSTATE:
                        if ((n = pass2sechk(dirp->d_ino)) == 1)
                                break;
			descend(idesc, dirp->d_ino);
			if (statemap[dirp->d_ino] == DFOUND) {
				lncntp[dirp->d_ino]--;
			} else if (statemap[dirp->d_ino] == DCLEAR) {
				dirp->d_ino = 0;
				ret |= ALTERED;
			} else
				errexit("BAD RETURN STATE %d FROM DESCEND",
				    statemap[dirp->d_ino]);
			break;

		default:
			errexit("BAD STATE %d FOR INODE I=%d",
			    statemap[dirp->d_ino], dirp->d_ino);
		}
	}
	pathp = curpathloc;
	*pathp = '\0';
	if (n == 0)
		return (ret|KEEPON);
	dirp->d_ino = 0;
	return (ret|KEEPON|ALTERED);
}


/*
 * Procedure:     pass2sechk
 *
 * Restrictions:  none
 *
 * Notes: 				pass2 security check 
 * 		
 *      1. This function is not to be used by ckinode; it could as
 *         easily have been a macro.
 *      2. return is 1 if the file inode is to be removed, 0 otherwise.
 */

static int
pass2sechk(ino)
        ino_t ino;
{
        if (((secstatemap[ino] & SEC_INOVIS) == 0)
        &&  (secstatemap[ino] & (SEC_BADACL|SEC_DUPACL))) {
                secstatemap[ino] |= SEC_INOVIS;
                direrr(ino, "DUP/BAD/INVALID ACL");
                if (reply("REMOVE") == 1) {
                        statemap[ino] = (statemap[ino]==FSTATE) ? FCLEAR : DCLEAR;
                        return(1);
                }
        }
        return(0);
}
