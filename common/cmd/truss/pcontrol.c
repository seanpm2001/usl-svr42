/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)truss:common/cmd/truss/pcontrol.c	1.3.7.2"
#ident  "$Header: pcontrol.c 1.4 91/07/09 $"
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <memory.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "systable.h"
#include "proto.h"
#include "machdep.h"

/* Process Management */

/* This module is carefully coded to contain only read-only data */
/* All read/write data is defined in ramdata.c (see also ramdata.h) */


extern int fork();
extern int ioctl();
extern unsigned alarm();
extern long lseek();
extern void perror();
extern void _exit();

extern char * malloc();
extern void free();

extern unsigned short getuid();
extern unsigned short getgid();
extern unsigned short geteuid();
extern unsigned short getegid();

/*
 * Function prototypes for static routines in this module.
 */
#if	defined(__STDC__)

static	void	prdump( process_t * );
static	void	deadcheck( process_t * );

#else	/* defined(__STDC__) */

static	void	prdump();
static	void	deadcheck();

#endif	/* defined(__STDC__) */

#include "registers.h"

int
Pcreate(P, args)	/* create new controlled process */
register process_t *P;	/* program table entry */
char **args;		/* argument array, including the command name */
{
	register int i;
	register int upid;
	register int fd;
	char procname[100];

	upid = fork();

	if (upid == 0) {		/* child process */
		(void) pause();		/* wait for PRSABORT from parent */

		/* if running setuid or setgid, reset credentials to normal */
		if ((i = getgid()) != getegid())
			(void) setgid(i);
		if ((i = getuid()) != geteuid())
			(void) setuid(i);

		(void) execvp(*args, args);	/* execute the command */
		_exit(127);
	}

	if (upid == -1) {		/* failure */
		perror("Pcreate fork()");
		return -1;
	}

	/* initialize the process structure */
	(void) memset((char *)P, 0, sizeof(*P));

	P->cntrl = TRUE;
	P->child = TRUE;
	P->state = PS_RUN;
	P->upid  = upid;

	/* open the /proc/upid file */
	(void) sprintf(procname, "%s/%d", procdir, upid);

	/* exclusive open prevents others from interfering */
	if ((fd = open(procname, (O_RDWR|O_EXCL))) < 0) {
		perror("Pcreate open()");
		(void) kill(upid, SIGKILL);
		return -1;
	}

	/* make sure it's not one of 0, 1, or 2 */
	/* this allows truss to work when spawned by init(1m) */
	if (0 <= fd && fd <= 2) {
		int dfd = fcntl(fd, F_DUPFD, 3);

		(void) close(fd);
		if (dfd < 0) {
			perror("Pcreate fcntl()");
			(void) kill(upid, SIGKILL);
			return -1;
		}
		fd = dfd;
	}
	/* mark it close-on-exec so any created process doesn't inherit it */
	(void) fcntl(fd, F_SETFD, 1);

	/* mark it run-on-last-close so it runs even if we die on a signal */
	if (Ioctl(fd, PIOCSRLC, 0) == -1)
		perror("Pcreate PIOCSRLC");

	P->pfd = fd;

	for (;;) {		/* wait for process to sleep in pause() */
		(void) Pstop(P);	/* stop the controlled process */

		if (P->state == PS_STOP
		 && P->why.pr_why == PR_REQUESTED
		 && (P->why.pr_flags & PR_ASLEEP)
		 && Pgetsysnum(P) == SYS_pause)
			break;

		if (P->state != PS_STOP		/* interrupt or process died */
		 || Psetrun(P, 0, 0) != 0) {	/* can't restart */
			int sig = SIGKILL;

			(void) Ioctl(fd, PIOCKILL, (int)&sig);	/* kill !  */
			(void) close(fd);
			(void) kill(upid, sig);			/* kill !! */
			P->state = PS_DEAD;
			P->pfd = 0;
			return -1;
		}
	}

	(void) Psysentry(P, SYS_exit, 1);	/* catch these sys calls */
	(void) Psysentry(P, SYS_exec, 1);
	(void) Psysentry(P, SYS_execve, 1);

	/* kick it off the pause() */
	if (Psetrun(P, 0, PRSABORT) == -1) {
		int sig = SIGKILL;

		perror("Pcreate PIOCRUN");
		(void) Ioctl(fd, PIOCKILL, (int)&sig);
		(void) kill(upid, sig);
		(void) close(fd);
		P->state = PS_DEAD;
		P->pfd = 0;
		return -1;
	}

	(void) Pwait(P);	/* wait for exec() or exit() */

	return 0;
}

/* This is for the !?!*%! call to sleep() in execvp() */
unsigned int
sleep(n)
unsigned n;
{
	if (n) {
		(void) alarm(n);
		(void) pause();
		(void) alarm(0);
		timeout = FALSE;
	}
	return 0;
}

#if 0
static void
fddump(fd)	/* debugging code -- dump fstat(fd) */
int fd;
{
	struct stat statb;
	CONST char * s;

	(void) fprintf(stderr, "fd = %d", fd);
	if (fstat(fd, &statb) == -1)
		goto out;
	switch (statb.st_mode & S_IFMT) {
	case S_IFDIR: s="S_IFDIR"; break;
	case S_IFCHR: s="S_IFCHR"; break;
	case S_IFBLK: s="S_IFBLK"; break;
	case S_IFREG: s="S_IFREG"; break;
	case S_IFIFO: s="S_IFIFO"; break;
	default:      s="???"; break;
	}
	(void) fprintf(stderr, "  %s  mode = 0%o  dev = 0x%.4X  ino = %d",
		s,
		statb.st_mode & ~S_IFMT,
		statb.st_dev,
		statb.st_ino);
	(void) fprintf(stderr, "  uid = %d,  gid = %d,  size = %ld\n",
		statb.st_uid,
		statb.st_gid,
		statb.st_size);

out:
	(void) fputc('\n', stderr);
}
#endif /* 0 */

int
Pgrab(P, upid, force)		/* grab existing process */
register process_t *P;		/* program table entry */
register pid_t upid;		/* UNIX process ID */
int force;			/* if TRUE, grab regardless */
{
	register int fd = -1;
	int nmappings;
	int ruid;
	struct prcred prcred;
	char procname[100];

again:	/* Come back here if we lose it in the Window of Vulnerability */
	if (fd >= 0) {
		(void) close(fd);
		fd = -1;
	}

	(void) memset((char *)P, 0, sizeof(*P));

	if (upid <= 0)
		return -1;

	/* generate the /proc/upid filename */
	(void) sprintf(procname, "%s/%d", procdir, upid);

	/* Request exclusive open to avoid grabbing someone else's	*/
	/* process and to prevent others from interfering afterwards.	*/
	/* If this fails and the 'force' flag is set, attempt to	*/
	/* open non-exclusively (effective only for the super-user).	*/
	if ((fd = open(procname, (O_RDWR|O_EXCL))) < 0
	 && (fd = (force? open(procname, O_RDWR) : -1)) < 0) {
		if (errno == EBUSY && !force)
			return 1;
		if (debugflag)
			perror("Pgrab open()");
		return -1;
	}

	/* ---------------------------------------------------- */
	/* We are now in the Window of Vulnerability (WoV).	*/
	/* The process may exec() a setuid/setgid or unreadable	*/
	/* object file between the open() and the PIOCSTOP.	*/
	/* We will get EAGAIN in this case and must start over.	*/
	/* ---------------------------------------------------- */

	/* If the process is a system process, we can't control it	*/
	/* even if we are super-user.  The number of mappings is the	*/
	/* way to determine a system process.				*/
	if (Ioctl(fd, PIOCNMAP, (int)&nmappings) == -1) {
		if (errno == EAGAIN)	/* WoV */
			goto again;
		if (errno != ENOENT)	/* Don't complain about zombies */
			perror("Pgrab PIOCNMAP");
		nmappings = 0;
	}
	/* there must be at least text, data, and stack */
	if (nmappings < 3) {
		(void) close(fd);
		return -1;
	}

	/* Verify process credentials in case we are running setuid root.   */
	/* We only verify that our real uid matches the process's real uid. */
	/* This means that the user really did create the process, even     */
	/* if using a different group id (via newgrp(1) for example).       */
	if (Ioctl(fd, PIOCCRED, (int)&prcred) == -1) {
		if (errno == EAGAIN)	/* WoV */
			goto again;
		if (errno != ENOENT)	/* Don't complain about zombies */
			perror("Pgrab PIOCCRED");
		(void) close(fd);
		return -1;
	}
	if ((ruid = getuid()) != 0	/* super-user allowed anything */
	 && ruid != prcred.pr_ruid) {	/* credentials check failed */
		(void) close(fd);
		return -1;
	}

	/* make sure it's not one of 0, 1, or 2 */
	/* this allows truss to work when spawned by init(1m) */
	if (0 <= fd && fd <= 2) {
		int dfd = fcntl(fd, F_DUPFD, 3);

		(void) close(fd);
		if (dfd < 0) {
			perror("Pgrab fcntl()");
			return -1;
		}
		fd = dfd;
	}
	/* mark it close-on-exec so any created process doesn't inherit it */
	(void) fcntl(fd, F_SETFD, 1);

	/* mark it run-on-last-close so it runs even if we die from SIGKILL */
	if (Ioctl(fd, PIOCSRLC, 0) == -1) {
		if (errno == EAGAIN)	/* WoV */
			goto again;
		if (errno != ENOENT)	/* Don't complain about zombies */
			perror("Pgrab PIOCSRLC");
	}

	P->cntrl = TRUE;
	P->child = FALSE;
	P->state = PS_RUN;
	P->upid  = upid;
	P->pfd   = fd;

	/* before stopping the process, make sure it's not ourself */
	if (upid == getpid()) {
		/* write a magic number, read it through /proc file */
		/* and see if the results match. */
		long magic1 = 0;
		long magic2 = 2;

		errno = 0;

		if (Pread(P, (long)&magic1, (char *)&magic2, sizeof(magic2))
		    == sizeof(magic2)
		 && magic2 == 0
		 && (magic1 = 0xfeedbeef)
		 && Pread(P, (long)&magic1, (char *)&magic2, sizeof(magic2))
		    == sizeof(magic2)
		 && magic2 == 0xfeedbeef) {
			(void) close(fd);
			(void) fprintf(stderr,
			"Pgrab(): process attempted to grab itself\n");
			return -1;
		}
	}
	
	/* Stop the process, get its status and its signal/syscall masks. */
	if (Pstatus(P, PIOCSTOP, 2) != 0) {
		if (P->state == PS_LOST)	/* WoV */
			goto again;
		if (errno != EINTR
		 || (P->state != PS_STOP && !(P->why.pr_flags&PR_DSTOP))) {
			if (P->state != PS_RUN) {
				if (errno != ENOENT)
					perror("Pgrab PIOCSTOP");
#if 0
				fddump(fd);
#endif
			}
			(void) close(fd);
			return -1;
		}
	}

	/* Process is or will be stopped, these will "certainly" not fail */
	if (Ioctl(fd, PIOCGTRACE, (int)&P->sigmask) == -1)
		perror("Pgrab PIOCGTRACE");
	if (Ioctl(fd, PIOCGFAULT, (int)&P->faultmask) == -1)
		perror("Pgrab PIOCGFAULT");
	if (Ioctl(fd, PIOCGENTRY, (int)&P->sysentry) == -1)
		perror("Pgrab PIOCGENTRY");
	if (Ioctl(fd, PIOCGEXIT,  (int)&P->sysexit)  == -1)
		perror("Pgrab PIOCGEXIT");

	return 0;
}

int
Preopen(P, force)	/* reopen the /proc file (after PS_LOST) */
register process_t *P;
int force;			/* if TRUE, grab regardless */
{
	register int fd;
	char procname[100];

	/* reopen the /proc/upid file */
	(void) sprintf(procname, "%s/%d", procdir, P->upid);
	if ((fd = open(procname, (O_RDWR|O_EXCL))) < 0
	 && (fd = (force? open(procname, O_RDWR) : -1)) < 0) {
		if (debugflag)
			perror("Preopen open()");
		return -1;
	}

	if (P->pfd == 0		/* close the old filedescriptor */
	 || close(P->pfd) != 0)
		P->pfd = 3;

	/* make the new filedescriptor the same as the old */
	if (fd != P->pfd) {
		int dfd = fcntl(fd, F_DUPFD, P->pfd);

		P->pfd = 0;
		(void) close(fd);
		if (dfd < 0) {
			perror("Preopen fcntl()");
			return -1;
		}
		fd = dfd;
	}
	/* mark it close-on-exec so any created process doesn't inherit it */
	(void) fcntl(fd, F_SETFD, 1);

	/* set run-on-last-close so it runs even if we die from SIGKILL */
	if (Ioctl(fd, PIOCSRLC, 0) == -1)
		perror("Preopen PIOCSRLC");

	P->state = PS_RUN;
	P->pfd   = fd;
	
	/* process should be stopped on exec (REQUESTED) */
	/* or else should be stopped on exit from exec() (SYSEXIT) */
	if (Pwait(P) == 0
	 && P->state == PS_STOP
	 && (P->why.pr_why == PR_REQUESTED
	  || (P->why.pr_why == PR_SYSEXIT
	   && (P->why.pr_what == SYS_exec || P->why.pr_what == SYS_execve)))) {
		/* fake up stop-on-exit-from-execve */
		if (P->why.pr_why == PR_REQUESTED) {
			P->why.pr_why = PR_SYSEXIT;
			P->why.pr_what = SYS_execve;
		}
	}
	else {
		(void) fprintf(stderr,
		"Preopen: expected REQUESTED or SYSEXIT(SYS_execve) stop\n");
	}

	return 0;
}

int
Prelease(P)		/* release process to run freely */
register process_t *P;
{
	register int fd = P->pfd;

	if (fd == 0)
		return -1;

	if (debugflag)
		(void) fprintf(stderr, "Prelease: releasing pid # %d\n",
			P->upid);

	/* attempt to stop it if we have to reset its registers */
	if (P->sethold || P->setregs) {
		register int count;
		for (count = 10;
		     count > 0 && (P->state == PS_RUN || P->state == PS_STEP);
		     count--) {
			(void) Pstop(P);
		}
	}

	/* if we lost control, all we can do is close the file */
	if (P->state == PS_STOP) {
		if (P->sethold
		 && Ioctl(fd, PIOCSHOLD, (int)&P->why.pr_sighold) != 0)
			perror("Prelease PIOCSHOLD");
		if (P->setregs
		 && Ioctl(fd, PIOCSREG, (int)&P->REG[0]) != 0)
			perror("Prelease PIOCSREG");
	}

	(void) Ioctl(fd, PIOCRFORK, 0);
	(void) Ioctl(fd, PIOCSRLC, 0);
	(void) close(fd);	/* this sets the process running */

	/* zap the process structure */
	(void) memset((char *)P, 0, sizeof(*P));

	return 0;
}

/* debugging */
static void
prdump(P)
process_t *P;
{
	long bits = *((long *)&P->why.pr_sigpend);

	if (P->why.pr_cursig)
		(void) fprintf(stderr, "  p_cursig  = %d", P->why.pr_cursig);
	if (bits)
		(void) fprintf(stderr, "  p_sigpend = 0x%.8X", bits);
	(void) fputc('\n', stderr);
}

#if 0
/* debugging */
static void
dumpwhy(P, str)
register process_t *P;
char *str;
{
	register int i;

	if (str)
		(void) fprintf(stderr, "%s\n", str);
	(void) fprintf(stderr, "pr_flags  = 0x%.8X\n", P->why.pr_flags);
	(void) fprintf(stderr, "pr_why    = 0x%.8X\n", P->why.pr_why);
	(void) fprintf(stderr, "pr_what   = 0x%.8X\n", P->why.pr_what);
	(void) fprintf(stderr, "pr_pid    = 0x%.8X\n", P->why.pr_pid);
	(void) fprintf(stderr, "pr_ppid   = 0x%.8X\n", P->why.pr_ppid);
	(void) fprintf(stderr, "pr_pgrp   = 0x%.8X\n", P->why.pr_pgrp);
	(void) fprintf(stderr, "pr_cursig = 0x%.8X\n", P->why.pr_cursig);
	(void) fprintf(stderr, "pr_sigpend= 0x%.8X\n", P->why.pr_sigpend.bits[0]);
	(void) fprintf(stderr, "pr_sighold= 0x%.8X\n", P->why.pr_sighold.bits[0]);
	(void) fprintf(stderr, "pr_instr  = 0x%.8X\n", P->why.pr_instr);
	(void) fprintf(stderr, "pr_utime  = 0x%.8X\n", P->why.pr_utime);
	(void) fprintf(stderr, "pr_stime  = 0x%.8X\n", P->why.pr_stime);
	for (i = 0; i < NGREG; i++)
		(void) fprintf(stderr, "%%%s = %.8x\n",
			regname[i], P->REG[i]);
}
#endif

int
Pwait(P)	/* wait for process to stop for any reason */
register process_t *P;
{
	return Pstatus(P, PIOCWSTOP, 0);
}

int
Pstop(P)	/* direct process to stop; wait for it to stop */
register process_t *P;
{
	return Pstatus(P, PIOCSTOP, 0);
}

int
Pstatus(P, request, sec) /* wait for specified process to stop or terminate */
register process_t *P;	/* program table entry */
register int request;	/* PIOCSTATUS, PIOCSTOP, PIOCWSTOP */
unsigned sec;		/* if non-zero, alarm timeout in seconds */
{
	register int status = 0;
	int err = 0;

	switch (P->state) {
	case PS_NULL:
	case PS_LOST:
	case PS_DEAD:
		return -1;
	case PS_STOP:
		if (request != PIOCSTATUS)
			return 0;
	}

	switch (request) {
	case PIOCSTATUS:
	case PIOCSTOP:
	case PIOCWSTOP:
		break;
	default:
		/* programming error */
		(void) fprintf(stderr, "Pstatus: illegal request\n");
		return -1;
	}

	timeout = FALSE;
	if (sec)
		(void) alarm(sec);
	if (Ioctl(P->pfd, request, (int)&P->why) != 0) {
		err = errno;
		if (sec)
			(void) alarm(0);
		if (request != PIOCSTATUS && err == EINTR
		 && Ioctl(P->pfd, PIOCSTATUS, (int)&P->why) != 0)
			err = errno;
	}
	else if (sec)
		(void) alarm(0);

	if (err) {
		switch (err) {
		case EINTR:		/* timeout or user typed DEL */
			if (debugflag)
				(void) fprintf(stderr, "Pstatus: EINTR\n");
			break;
		case EAGAIN:		/* we lost control of the the process */
			if (debugflag)
				(void) fprintf(stderr, "Pstatus: EAGAIN\n");
			P->state = PS_LOST;
			break;
		default:		/* check for dead process */
			if (debugflag || err != ENOENT) {
				CONST char * errstr;

				switch (request) {
				case PIOCSTATUS:
					errstr = "Pstatus PIOCSTATUS"; break;
				case PIOCSTOP:
					errstr = "Pstatus PIOCSTOP"; break;
				case PIOCWSTOP:
					errstr = "Pstatus PIOCWSTOP"; break;
				default:
					errstr = "Pstatus PIOC???"; break;
				}
				perror(errstr);
			}
			deadcheck(P);
			break;
		}
		if (!timeout || err != EINTR) {
			errno = err;
			return -1;
		}
	}

	if (!(P->why.pr_flags&PR_STOPPED)) {
		if (request == PIOCSTATUS || timeout) {
			timeout = FALSE;
			return 0;
		}
		(void) fprintf(stderr, "Pstatus: process is not stopped\n");
		return -1;
	}

	P->state = PS_STOP;
	timeout = FALSE;

#if 0	/* debugging */
    {
	struct prstatus pstats;

	if (Ioctl(P->pfd, PIOCSTATUS, (int)&pstats) == -1)
		perror("Pstatus: PIOCSTATUS");
	else if (memcmp((char *)&P->why, (char *)&pstats, sizeof(pstats)) != 0)
		(void) fprintf(stderr,
			"Pstatus: PIOCWSTOP and PIOCSTATUS disagree!\n");
    }
#endif

#if 0	/* debugging */
    {
	gregset_t regs;		/* array of registers from process */
	register int i;

	/* read the process registers */
	if (Ioctl(P->pfd, PIOCGREG, (int)&regs[0]) == -1)
		perror("Pstatus: PIOCGREG failure");
	else for (i = 0; i < NGREG; i++) {
		if (debugflag && P->REG[i] != regs[i])
			(void) fprintf(stderr,
				"  %%%s: 0x%.8X 0x%.8X\n",
				regname[i], P->REG[i], regs[i]);
		P->REG[i] = regs[i];
	}
    }
#endif

	switch (P->why.pr_why) {
	case PR_REQUESTED:
		if (debugflag) {
			(void) fprintf(stderr, "Pstatus: why: REQUESTED");
			prdump(P);
		}
		break;
	case PR_SIGNALLED:
		if (debugflag) {
			(void) fprintf(stderr, "Pstatus: why: SIGNALLED %s",
				signame(P->why.pr_what));
			prdump(P);
		}
		break;
	case PR_FAULTED:
		if (debugflag) {
			(void) fprintf(stderr, "Pstatus: why: FAULTED %s",
				fltname(P->why.pr_what));
			prdump(P);
		}
		break;
	case PR_SYSENTRY:
		/* remember syscall address */
		P->sysaddr = P->REG[R_PC]-SYSCALL_OFF;

		if (debugflag) {
			(void) fprintf(stderr, "Pstatus: why: SYSENTRY %s",
				sysname(P->why.pr_what, -1));
			prdump(P);
		}
		break;
	case PR_SYSEXIT:
		/* remember syscall address */
		P->sysaddr = P->REG[R_PC]-SYSCALL_OFF;

		if (debugflag) {
			(void) fprintf(stderr, "Pstatus: why: SYSEXIT %s",
				sysname(P->why.pr_what, -1));
			prdump(P);
		}
		break;
	case PR_JOBCONTROL:
		if (debugflag) {
			(void) fprintf(stderr, "Pstatus: why: JOBCONTROL %s",
				signame(P->why.pr_what));
			prdump(P);
		}
		break;
	default:
		if (debugflag) {
			(void) fprintf(stderr, "Pstatus: why: Unknown");
			prdump(P);
		}
		status = -1;
		break;
	}

#if 0	/* debugging */
	if (debugflag)
		dumpwhy(P, 0);
#endif

	return status;
}

static void
deadcheck(P)
register process_t *P;
{
	if (P->pfd == 0)
		P->state = PS_DEAD;
	else {
		while (Ioctl(P->pfd, PIOCSTATUS, (int)&P->why) != 0) {
			switch (errno) {
			default:
				/* process is dead */
				if (P->pfd != 0)
					(void) close(P->pfd);
				P->pfd = 0;
				P->state = PS_DEAD;
				break;
			case EINTR:
				continue;
			case EAGAIN:
				P->state = PS_LOST;
				break;
			}
			break;
		}
	}
}

int
Pgetregs(P)	/* get values of registers from stopped process */
register process_t *P;
{
	if (P->state != PS_STOP)
		return -1;
	return 0;		/* registers are always available */
}

int
Pgetareg(P, reg)	/* get the value of one register from stopped process */
register process_t *P;
register int reg;		/* register number */
{
	if (reg < 0 || reg >= NGREG) {
		(void) fprintf(stderr,
			"Pgetareg(): invalid register number, %d\n", reg);
		return -1;
	}
	if (P->state != PS_STOP)
		return -1;
	return 0;		/* registers are always available */
}

int
Pputregs(P)	/* put values of registers into stopped process */
register process_t *P;
{
	if (P->state != PS_STOP)
		return -1;
	P->setregs = TRUE;	/* set registers before continuing */
	return 0;
}

int
Pputareg(P, reg)	/* put value of one register into stopped process */
register process_t *P;
register int reg;		/* register number */
{
	if (reg < 0 || reg >= NGREG) {
		(void) fprintf(stderr,
			"Pputareg(): invalid register number, %d\n", reg);
		return -1;
	}
	if (P->state != PS_STOP)
		return -1;
	P->setregs = TRUE;	/* set registers before continuing */
	return 0;
}

int
Psetrun(P, sig, flags)
register process_t *P;
int sig;		/* signal to pass to process */
register int flags;	/* flags: PRCSIG|PRSTEP|PRSABORT|PRSTOP */
{
	register int request;		/* for setting signal */
	register int why = P->why.pr_why;
	siginfo_t info;
	struct prrun prrun;

	if (sig < 0 || sig > PRMAXSIG
	 || P->state != PS_STOP)
		return -1;

	if (sig) {
		if (flags & PRCSIG)
			request = PIOCKILL;
		else {
			switch (why) {
			case PR_REQUESTED:
			case PR_SIGNALLED:
				request = PIOCSSIG;
				break;
			default:
				request = PIOCKILL;
				break;
			}
		}
	}

	/* must be initialized to zero */
	(void) memset((char *)&prrun, 0, sizeof(prrun));
	(void) memset((char *)&info, 0, sizeof(info));
	info.si_signo = sig;

	prrun.pr_flags = flags & ~(PRSTRACE|PRSHOLD|PRSFAULT|PRSVADDR);

	if (P->setsig) {
		prrun.pr_flags |= PRSTRACE;
		prrun.pr_trace = P->sigmask;
	}
	if (P->sethold) {
		prrun.pr_flags |= PRSHOLD;
		prrun.pr_sighold = P->why.pr_sighold;
	}
	if (P->setfault) {
		prrun.pr_flags |= PRSFAULT;
		prrun.pr_fault = P->faultmask;
	}
	if ((P->setentry && Ioctl(P->pfd, PIOCSENTRY, (int)&P->sysentry) == -1)
	 || (P->setexit  && Ioctl(P->pfd, PIOCSEXIT,  (int)&P->sysexit)  == -1)
	 || (P->setregs  && Ioctl(P->pfd, PIOCSREG,  (int)&P->REG[0])  == -1)
	 || (sig && Ioctl(P->pfd, request, (int)&info) == -1)) {
bad:
		if (errno != ENOENT) {
			perror("Psetrun");
			return -1;
		}
		goto out;
	}
	P->setentry = FALSE;
	P->setexit  = FALSE;
	P->setregs  = FALSE;

	if (Ioctl(P->pfd, PIOCRUN, prrun.pr_flags? (int)&prrun : 0) == -1) {
		if ((why != PR_SIGNALLED && why != PR_JOBCONTROL)
		 || errno != EBUSY)
			goto bad;
		goto out;	/* ptrace()ed or jobcontrol stop -- back off */
	}

	P->setsig   = FALSE;
	P->sethold  = FALSE;
	P->setfault = FALSE;
out:
	P->state    = (flags&PRSTEP)? PS_STEP : PS_RUN;
	return 0;
}

int
Pstart(P, sig)
register process_t *P;
int sig;		/* signal to pass to process */
{
	return Psetrun(P, sig, 0);
}

int
Pterm(P)
register process_t *P;
{
	int sig = SIGKILL;

	if (debugflag)
		(void) fprintf(stderr,
			"Pterm: terminating pid # %d\n", P->upid);
	if (P->state == PS_STOP)
		(void) Pstart(P, SIGKILL);
	(void) Ioctl(P->pfd, PIOCKILL, (int)&sig);	/* make sure */
	(void) kill((int)P->upid, SIGKILL);		/* make double sure */

	if (P->pfd != 0)
		(void) close(P->pfd);

	/* zap the process structure */
	(void) memset((char *)P, 0, sizeof(*P));

	return 0;
}

int
Pread(P, address, buf, nbyte)
register process_t *P;
long address;		/* address in process */
char *buf;		/* caller's buffer */
int nbyte;		/* number of bytes to read */
{
	register int rc = -1;

	if (nbyte <= 0)
		return 0;

	if (lseek(P->pfd, (long)address, 0) == address)
		rc = read(P->pfd, buf, (unsigned)nbyte);

	return rc;
}

int
Pwrite(P, address, buf, nbyte)
register process_t *P;
long address;		/* address in process */
CONST char *buf;	/* caller's buffer */
int nbyte;		/* number of bytes to write */
{
	register int rc = -1;

	if (nbyte <= 0)
		return 0;

	if (lseek(P->pfd, (long)address, 0) != address
	 || (rc = write(P->pfd, buf, (unsigned)nbyte)) == -1)
		perror("Pwrite");

	return rc;
}

int
Psignal(P, which, stop)		/* action on specified signal */
register process_t *P;		/* program table exit */
register int which;		/* signal number */
register int stop;		/* if TRUE, stop process; else let it go */
{
	register int oldval;

	if (which <= 0 || which > PRMAXSIG || (which == SIGKILL && stop))
		return -1;

	oldval = prismember(&P->sigmask, which)? TRUE : FALSE;

	if (stop) {	/* stop process on receipt of signal */
		if (!oldval) {
			praddset(&P->sigmask, which);
			P->setsig = TRUE;
		}
	}
	else {		/* let process continue on receipt of signal */
		if (oldval) {
			prdelset(&P->sigmask, which);
			P->setsig = TRUE;
		}
	}

	return oldval;
}

int
Pfault(P, which, stop)		/* action on specified fault */
register process_t *P;		/* program table exit */
register int which;		/* fault number */
register int stop;		/* if TRUE, stop process; else let it go */
{
	register int oldval;

	if (which <= 0 || which > PRMAXFAULT)
		return -1;

	oldval = prismember(&P->faultmask, which)? TRUE : FALSE;

	if (stop) {	/* stop process on receipt of fault */
		if (!oldval) {
			praddset(&P->faultmask, which);
			P->setfault = TRUE;
		}
	}
	else {		/* let process continue on receipt of fault */
		if (oldval) {
			prdelset(&P->faultmask, which);
			P->setfault = TRUE;
		}
	}

	return oldval;
}

int
Psysentry(P, which, stop)	/* action on specified system call entry */
register process_t *P;		/* program table entry */
register int which;		/* system call number */
register int stop;		/* if TRUE, stop process; else let it go */
{
	register int oldval;

	if (which <= 0 || which > PRMAXSYS)
		return -1;

	oldval = prismember(&P->sysentry, which)? TRUE : FALSE;

	if (stop) {	/* stop process on sys call */
		if (!oldval) {
			praddset(&P->sysentry, which);
			P->setentry = TRUE;
		}
	}
	else {		/* don't stop process on sys call */
		if (oldval) {
			prdelset(&P->sysentry, which);
			P->setentry = TRUE;
		}
	}

	return oldval;
}

int
Psysexit(P, which, stop)	/* action on specified system call exit */
register process_t *P;		/* program table exit */
register int which;		/* system call number */
register int stop;		/* if TRUE, stop process; else let it go */
{
	register int oldval;

	if (which <= 0 || which > PRMAXSYS)
		return -1;

	oldval = prismember(&P->sysexit, which)? TRUE : FALSE;

	if (stop) {	/* stop process on sys call exit */
		if (!oldval) {
			praddset(&P->sysexit, which);
			P->setexit = TRUE;
		}
	}
	else {		/* don't stop process on sys call exit */
		if (oldval) {
			prdelset(&P->sysexit, which);
			P->setexit = TRUE;
		}
	}

	return oldval;
}


int
execute(P, sysindex)	/* execute the syscall instruction */
register process_t *P;	/* process control structure */
int sysindex;		/* system call index */
{
	sigset_t hold;		/* mask of held signals */
	int sentry;		/* old value of stop-on-syscall-entry */

	/* move current signal back to pending */
	if (P->why.pr_cursig) {
		int sig = P->why.pr_cursig;
		(void) Ioctl(P->pfd, PIOCSSIG, 0);
		(void) Ioctl(P->pfd, PIOCKILL, (int)&sig);
		P->why.pr_cursig = 0;
	}

	sentry = Psysentry(P, sysindex, TRUE);	/* set stop-on-syscall-entry */
	hold = P->why.pr_sighold;	/* remember signal hold mask */
	prfillset(&P->why.pr_sighold);	/* hold all signals */
	P->sethold = TRUE;

	if (Psetrun(P, 0, PRCSIG) == -1)
		goto bad;
	while (P->state == PS_RUN)
		(void) Pwait(P);

	if (P->state != PS_STOP)
		goto bad;
	P->why.pr_sighold = hold;		/* restore hold mask */
	P->sethold = TRUE;
	(void) Psysentry(P, sysindex, sentry);	/* restore sysentry stop */
	if (P->why.pr_why  == PR_SYSENTRY
	 && P->why.pr_what == sysindex)
		return 0;
bad:
	return -1;
}

int
checksyscall(P)		/* check syscall instruction in process */
process_t *P;
{
	/* this should always succeed--we always have a good syscall address */
	syscall_t instr;		/* holds one syscall instruction */

	return(
	   (Pread(P,P->sysaddr,(char *)&instr,sizeof(instr)) == sizeof(instr)
	    && instr == SYSCALL)?
		0 : -1 );
}

int
Ioctl(fd, request, arg)		/* deal with RFS congestion */
int fd;
int request;
int arg;
{
	register int rc;
	char str[40];

	for(;;) {
		if ((rc = ioctl(fd, request, arg)) != -1
		 || errno != ENOMEM)
			return rc;

		if (debugflag) {
			(void) sprintf(str, "\t *** Ioctl(0x%X): ENOMEM\n",
				request);
			(void) write(2, str, (unsigned)strlen(str));
		}
	}
}

/* test for empty set */
/* support routine used by isemptyset() macro */
int
is_empty(sp, n)
register CONST long * sp;	/* pointer to set (array of longs) */
register unsigned n;		/* number of longs in set */
{
	if (n) {
		do {
			if (*sp++)
				return FALSE;
		} while (--n);
	}

	return TRUE;
}
