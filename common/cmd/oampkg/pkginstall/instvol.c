/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)oampkg:common/cmd/oampkg/pkginstall/instvol.c	1.17.14.11"
#ident  "$Header: $"

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pkgstrct.h>
#include <pkgdev.h>
#include <pkglocs.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "install.h"

#define	ERR_ULIMIT	"unable to set ulimit to <%ld>, errno=%d"

extern struct pkgdev 	pkgdev;
extern struct cfent 	**eptlist;
extern struct mergstat 	*mstat;
extern struct rlimit 	rlp,
			unlimited;

extern char	tmpdir[],
		pkgsav[],
		pkgbin[],
		errbuf[],
		instdir[],
		*errstr,
		*pkginst,
		*category,
		**class;
extern char	*pkgstderr;
extern char	*pkgstdout;
extern unsigned	nclass;
extern int	dbchg,
		errno,
		limit,
		nosetuid,
		nocnflct,
		warnflag;

extern char	*qstrdup(),
		*getenv(),
		*srcpath();
extern struct pinfo
		*eptstat();
extern void	*calloc(),
		progerr(),
		logerr(),
		ckreturn(),
		pkgvolume(),
		backup(),
		echo(),
		quit(),
		killdd();
extern int	access(),
		chdir(),
		cppath(),
		averify(),
		cverify(),
		finalck(),
		ocfile(),
		srchcfile(),
		swapcfile(),
		putcfile(),
		pkgexecl();

#define ck_efile(s, p)	\
		((p->cinfo.modtime >= 0) && \
		p->ainfo.local && \
		cverify(0, &p->ftype, s, &p->cinfo))

#define LSIZE	(2*PATH_MAX+1)
#define MAXCHAR		14

#define ERR_CASFAIL	"class action script did not complete successfully"
#define ERR_TMPFILE	"unable to open temp file <%s>"
#define ERR_CORRUPT	"source path <%s> is corrupt"
#define ERR_CHDIR	"unable to change directory to <%s>"
#define ERR_CFBAD	"bad entry read of contents file"
#define ERR_CFMISSING	"missing entry in contents file for <%s>"
#define	ERR_FOPEN	"unable to open file <%s>"
#define	ERR_UNCOMPR	"unable to uncompress file <%s>"

static char	**script;
static int	eocflag;
static int	domerg();
static void	findscripts(), endofclass();
static char	*trunc();

char	uncompdir[PATH_MAX];	/* directory for uncompressing files into	*/

void
instvol(srcinst, part, nparts)
char	*srcinst;
int	part;
int	nparts;
{
	FILE	*listfp;
	int	i, n, count, tcount;
	char	*listfile,
		*srcp,
		*rsrcp,
		*dstp;

	/*
	 * Definitions for compression feature.
	 */
	char	*bname;			/* basename of file to be installed		*/
	char 	tpath[PATH_MAX];	/* temporary path to uncompressed file		*/
	char	cmdline[BUFSIZ];	/* build command line for uncompress command	*/
	FILE	*fp;			/* file pointer to source media file		*/
	int	c;			/* used to extract magic number from file	*/
	int 	compressed;		/* if file is compressed this is true		*/
	pid_t	pid;
	struct	stat dirstat;

	if(part == 1) {
		pkgvolume(&pkgdev, srcinst, part, nparts);
		findscripts();
	}

	tcount = 0;
	for(i=0; i < nclass; i++) {
		eocflag = count = 0;
		listfp = (FILE *) 0;
		for(;;) {
			n = domerg((count ? 0 : part), nparts, class[i], 
				&srcp, &dstp);
			if(n < 0)
				break; /* no more entries to process */
			if(!tcount++) {
				/* first file to install */
				nqtext(stderr, " ## Installing part %d of %d.", 
					part, nparts);
			}
			count++;
			if(script[i] && !listfp) {
				/* create list file */
				listfile = tempnam(tmpdir, "list");
				if((listfp = fopen(listfile, "w")) == NULL) {
					progerr(ERR_TMPFILE, listfile);
					quit(99);
				}
			}
			pkgvolume(&pkgdev, srcinst, part, nparts);

			/* check if can access srcp - if not,
			 * truncate all parts of the path to 14 chars
			 */
			if(srcp) {
				if(access(srcp, F_OK) == -1) {
					rsrcp = trunc(srcp);
					strcpy(srcp, rsrcp);
				}

				/*
				 * If environment parameter COMPRESSED is set to "true", then
				 * we know that the package was compressed via the -c option
				 * to the pkgmk command.  If so, check if file on source media
				 * is compressed (information files are never compressed).  If
				 * it is, let's uncompress it before checking if it is corrupted.
				 * This must be done because pkgmk creates the pkgmap entry for
				 * this file before compressing it.
				 */

				if(!strcmp(getenv("COMPRESSED"), "true")) {
					if ((fp = fopen(srcp, "r")) == NULL) {
						progerr(ERR_FOPEN, srcp);
						logerr("pathname=%s", srcp);
						warnflag++;
						continue;
					}
		
					compressed = 0;		/* reset compressed flag to untrue */
					if (((c = getc(fp)) == 037) && ((c = getc(fp) == 0235))) {
						compressed = 1;
						/*
						 * Since we don't want to change the source media file,
						 * we create a temporary uncompressed version of the file
						 * that will be used as the file to be copied to dstp.
						 */
						bname = strrchr(srcp, '/');
						/*
						 * If directory for uncompression doesn't yet exist, create it.
						 */
						pid = getpid();
						(void) sprintf(uncompdir, "/tmp/uncompr%d", pid);
						if(stat(uncompdir, &dirstat) < 0) 
							if(errno == ENOENT) { /* if it doesn't yet exist */
								if(mkdir(uncompdir, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) < 0) {
									progerr("could not create directory <%s> for uncompression",
										uncompdir);
									quit(99);
								}
							}
						(void) sprintf(tpath, "%s/%s", uncompdir, ++bname);
						(void) sprintf(cmdline, "/usr/bin/uncompress < \"%s\" > \"%s\"", srcp, tpath);
						if(esystem(cmdline, -1, -1)) {
							/*
							 * The uncompress command failed.
							 */
							rpterr();
							progerr(ERR_UNCOMPR, tpath);
							quit(99);
						}
					}
					(void) fclose(fp);
					 /* If the file uncompressed, point to the uncompressed version */ 
					if (compressed)
						srcp = tpath;
				}
			}


			if(script[i] || strchr("en", eptlist[n]->ftype)) {
				if(ck_efile(srcp, eptlist[n])) {
					progerr(ERR_CORRUPT, srcp);
					logerr(errbuf);
					warnflag++;
					continue;
				}
			}

			if(script[i]) {
				/* just collect names in a temporary file 
				 * that will be used as the stdin when the
				 * class action script is invoked
				 */
				(void) fprintf(listfp, "%s %s\n", 
						(srcp ? srcp: "/dev/null"), dstp);
			} else {
				echo(dstp);
				if(srcp) {
					/* copy from source media to target
					 * path and fix file mode and
					 * permission now in case installation
					 * is halted
					 */
					if(cppath(1, srcp, dstp, eptlist[n]->cinfo.modtime))
						warnflag++;
					else if(!finalck(eptlist[n], 1, 1)) {
						/* everything checks here */
						mstat[n].attrchg = 0;
						mstat[n].contchg = 0;
					}
					/*
					 * If no class action script exists for this class, it's safe
					 * to remove the temporary uncompressed version of the file.
					 * Otherwise, we need to keep it around for access by that script.
					 */
					if(compressed && !script[i])
							(void) unlink(tpath);
					compressed = 0;		/* reset */
				}
			}
		}

		/* we have now completed processing of all pathnames
		 * associated with this volume and class
		 */

		if(script[i]) {
			/* 
			 * execute appropriate class action script
			 * with list of source/destination pathnames
			 * as the input to the script
			 */
			if(chdir(pkgbin)) {
				progerr(ERR_CHDIR, pkgbin);
				quit(99);
			}
			if(listfp) 
				(void) fclose(listfp);
			if(eocflag) {
				/* since there are no more volumes which
				 * contain pathnames associated with this
				 * class, execute class action script with
				 * the ENDOFCLASS argument; we do this even
				 * if none of the pathnames associated with
				 * this class and volume needed installation
				 * to guarantee the class action script is
				 * executed at least once during package
				 * installation
				 */
				n = pkgexecl((listfp ? listfile : "/dev/null"), 
				   pkgstdout, pkgstderr, SHELL, script[i], "ENDOFCLASS", NULL);
				ckreturn(n, ERR_CASFAIL);
			} else if(count) {
				/* execute class action script */
				n = pkgexecl(listfile, pkgstdout, pkgstderr, SHELL, script[i], 
					NULL);
				ckreturn(n, ERR_CASFAIL);
			}
		}
				
		if(eocflag) {
			/* 
			 * Since we're going to update the contents file and the pkginfo ULIMIT
			 * parameter may have been used to reset the ulimit during package
			 * handling, we must make sure the ulimit is sufficient to allow access.
			 */
			if(limit) {
				if(setrlimit(RLIMIT_FSIZE, &unlimited) < 0) {
					progerr(ERR_ULIMIT, unlimited.rlim_cur, errno);
					(void) killdd();
					quit(99);
				}
			}

			/* 
			 * finalize merg; this checks to make sure file 
			 * attributes are correct and any links specified
			 * are created
			 */
			endofclass(class[i], (script[i] ? 0 : 1));

			/* Re-establish package ulimit */
			if((limit) && setrlimit(RLIMIT_FSIZE, &rlp) < 0) {
				progerr(ERR_ULIMIT, limit, errno);
				(void) killdd();
				quit(99);
			}
			/*
			 * We've finished handling a class, it's safe to remove the
			 * temporary uncompressed versions of files in uncompdir*.
			 */
			if(script[i] && stat(uncompdir, &dirstat) == 0)
				(void) rrmdir(uncompdir);
		}
	}

	if(tcount == 0 && strcmp(category, "set"))
		nqtext(stderr, "## Installation of part %d of %d is complete.", 
			part, nparts);
}

static void
findscripts()
{
	int	i;
	char	path[PATH_MAX];

	script = (char **) calloc(nclass, sizeof(char *));
	for(i=0; i < nclass; i++) {
		/* 
		 * locate appropriate installation class action script, if any;
		 * look on media for script, since it might be on the system due
		 * to a previous installation
		 */
		(void) sprintf(path, "%s/install/i.%s", instdir, class[i]);
		if(access(path, 0) == 0) {
			(void) sprintf(path, "%s/i.%s", pkgbin, class[i]);
			script[i] = qstrdup(path);
			continue;
		}

		(void) sprintf(path, "%s/i.%s", PKGSCR, class[i]);
		if(access(path, 0) == 0) {
			script[i] = qstrdup(path);
			continue;
		}
		script[i] = NULL;
	}
}

static int
domerg(part, nparts, myclass, srcp, dstp)
int	part, nparts;
char	*myclass;
char	**srcp, **dstp;
{
	static int	svindx = 0;
	static int	svpart = 0;
	static int	maxvol = 0;
	int	i;

	if(part) {
		maxvol = 0;
		svindx = 0;
		svpart = part;
	} else {
		i = svindx;
		part = svpart;
	}

	for(i=svindx; eptlist[i]; i++) {
		if(eptlist[i]->ftype == 'i')
			continue; /* ignore information files */
		if(strcmp(myclass, eptlist[i]->class))
			continue;
		if(eptlist[i]->volno > maxvol)
			maxvol = eptlist[i]->volno;
		if(part != eptlist[i]->volno)
			continue;
			
		if(nosetuid && (mstat[i].setuid || mstat[i].setgid)) {
			echo("%s <setuid/setgid process ignored>", 
				eptlist[i]->path);
			continue;
		}
		if(nocnflct && mstat[i].shared) {
			if(mstat[i].contchg || mstat[i].attrchg) {
				echo("%s <shared pathname ignored>", 
					eptlist[i]->path);
			}
			continue;
		}

		switch(eptlist[i]->ftype) {
		  case 'l':
			continue; /* defer to final proc */

		  case 's':
			if(averify(0, &eptlist[i]->ftype, eptlist[i]->path, 
			&eptlist[i]->ainfo, 0))
				echo("%s <symbolic link>", eptlist[i]->path);
			/* fall through */
		  case 'd':
		  case 'x':
		  case 'c':
		  case 'b':
		  case 'p':
			if(averify(1, &eptlist[i]->ftype, eptlist[i]->path, 
				&eptlist[i]->ainfo, mstat[i].shared) == 0) {
				mstat[i].contchg = mstat[i].attrchg = 0;
			}
		}

		if(mstat[i].contchg) {
			*dstp = eptlist[i]->path;
			if(strchr("fev", eptlist[i]->ftype)) {
				*srcp = eptlist[i]->ainfo.local;
				if(*srcp[0] == '~') {
					/* translate source pathname */
					*srcp = srcpath(instdir,
						&(eptlist[i]->ainfo.local[1]), 
						part, nparts);
				}
			} else
				*srcp = NULL;
			svindx = i+1;
			backup(*dstp, 1);
			return(i);
		}
		if(mstat[i].attrchg) {
			backup(eptlist[i]->path, 0);
			echo("%s <attribute change only>", eptlist[i]->path);
			/* fix the attributes now for robustness sake */
			if(averify(1, &eptlist[i]->ftype, eptlist[i]->path, 
				&eptlist[i]->ainfo, 1) == 0) {
				mstat[i].attrchg = 0;
			}
		}
	}
	if(maxvol == part)
		eocflag++; /* endofclass */
	return(-1); /* no entry on this volume */
}

static void
endofclass(myclass, ckflag)
char	*myclass;
int	ckflag;
{
	struct cfent entry;
	struct pinfo *pinfo;
	int	n, indx, flag;
	FILE	*fp;
	FILE	*fpo;
	char	en_path[PATH_MAX];

	if(ocfile(&fp, &fpo))
		quit(99);

	echo("[ verifying class <%s> ]", myclass);

	entry.pinfo = NULL;
	for(indx=0; ;indx++) {
		/* find next package object in this class */
		while(eptlist[indx]) {
			if((eptlist[indx]->ftype != 'i') && 
			   !strcmp(myclass, eptlist[indx]->class))
				break;
			indx++;
		}
	
		n = srchcfile(&entry, 
			(eptlist[indx] ? eptlist[indx]->path : NULL), fp, fpo);

		if(n == 0)
			break;
		if(n == -1) {
			progerr(ERR_CFBAD);
			if(entry.path)
				logerr("pathname=%s", entry.path);
		}
		if(n < 0) {
			logerr("problem=%s", errstr);
			quit(99);
		} else if(n != 1) {
			/* check if path should not be in the contents file */
			if((mstat[indx].setuid && nosetuid) ||
			(mstat[indx].setgid && nosetuid) ||
			(mstat[indx].shared && nocnflct)) 
				continue;
			progerr(ERR_CFMISSING, eptlist[indx]->path);
			quit(99);
		} 

		/*
		 * validate this entry and change the status
		 * flag in the 'contents' file
		 */
		if(eptlist[indx]->ftype == '-')
			(void) eptstat(&entry, pkginst, '@');
		else {
			if(eptlist[indx]->ftype == 'l') {
				if(averify(0, &eptlist[indx]->ftype, 
				eptlist[indx]->path, &eptlist[indx]->ainfo, 
				1)) {
					echo("%s <linked pathname>", 
						eptlist[indx]->path);
					mstat[indx].attrchg++;
				}
			}

			/* strcpy is needed so that entry.path is not
			 * overwritten when we do a srchcfile in averify
			 */
			(void) strcpy(en_path, entry.path);
			flag = finalck(eptlist[indx], mstat[indx].attrchg, 
				(ckflag ? mstat[indx].contchg : (-1)));
			(void) strcpy(entry.path, en_path);

			pinfo = entry.pinfo;
			while(pinfo) {
				if(!strcmp(pkginst, pinfo->pkg))
					break;
				pinfo = pinfo->next;
			}
			if(pinfo)
				pinfo->status = (flag ? '!' : '\0');
		}
		if(entry.npkgs) {
			if(putcfile(&entry, fpo))
				quit(99);
		}
	}
	(void) fclose(fp);
	if(swapcfile(fpo, (dbchg ? pkginst : NULL)))
		quit(99);
}

char 
*trunc(f1)
char *f1;
{
	static char	truncp[PATH_MAX];
	char		*path;

	truncp[0] = '\0';
	path = strtok(f1, "/");
	while(path) {
		strcat(truncp, "/");
		strncat(truncp, path, MAXCHAR);
		path = strtok(NULL, "/");
	}
	
	return(truncp);
}
