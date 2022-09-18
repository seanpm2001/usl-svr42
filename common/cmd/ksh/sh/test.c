/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:sh/test.c	1.4.6.3"
#ident "$Header: /sms/sinixV5.4es/rcs/s19-full/usr/src/cmd/ksh/sh/test.c,v 1.1 91/02/28 17:41:26 ccs Exp $"

/*
 * test expression
 * [ expression ]
 * Rewritten by David Korn
 */

#include	"defs.h"
#include	"test.h"
#ifdef OLDTEST
#   include	"sym.h"
#endif /* OLDTEST */

#define test_type(file,mask,value)	((test_mode(file)&(mask))==(value))
#define	tio(a,f)	(sh_access(a,f)==0)
static time_t ftime_compare();
static int test_stat();
static struct stat statb;
int	test_binop();
int	unop_test();


#ifdef OLDTEST
/* single char string compare */
#define c_eq(a,c)	(*a==c && *(a+1)==0)
/* two character string compare */
#define c2_eq(a,c1,c2)	(*a==c1 && *(a+1)==c2 && *(a+2)==0)

int b_test();

static char *nxtarg();
static int exp();
static int e3();

static int ap, ac;
static char **av;

int b_test(argn, com)
char *com[];
register int argn;
{
	register char *p = com[0];
	av = com;
	ap = 1;
	if(c_eq(p,'['))
	{
		p = com[--argn];
		if(!c_eq(p, ']'))
			sh_fail(e_test, e_bracket);
	}
	if(argn <= 1)
		return(1);
	ac = argn;
	return(!exp(0));
}

/*
 * evaluate a test expression.
 * flag is 0 on outer level
 * flag is 1 when in parenthesis
 * flag is 2 when evaluating -a 
 */

static exp(flag)
{
	register int r;
	register char *p;
	r = e3();
	while(ap < ac)
	{
		p = nxtarg(0);
		/* check for -o and -a */
		if(flag && c_eq(p,')'))
		{
			ap--;
			break;
		}
		if(*p=='-' && *(p+2)==0)
		{
			if(*++p == 'o')
			{
				if(flag==2)
				{
					ap--;
					break;
				}
				r |= exp(3);
				continue;
			}
			else if(*p == 'a')
			{
				r &= exp(2);
				continue;
			}
		}
		if(flag==0)
			break;
		sh_fail(e_test,  e_synbad);
	}
	return(r);
}

static char *nxtarg(mt)
{
	if(ap >= ac)
	{
		if(mt)
		{
			ap++;
			return(0);
		}
		sh_fail(e_test, e_argexp);
	}
	return(av[ap++]);
}


static e3()
{
	register char *a;
	register char *p2;
	register int p1;
	char *op;
	a=nxtarg(0);
	if(c_eq(a, '!'))
		return(!e3());
	if(c_eq(a, '('))
	{
		p1 = exp(1);
		p2 = nxtarg(0);
		if(!c_eq(p2, ')'))
			sh_fail(e_test,e_paren);
		return(p1);
	}
	p2 = nxtarg(1);
	if(p2!=0 && (c_eq(p2,'=') || c2_eq(p2,'!','=')))
		goto skip;
	if(c2_eq(a,'-','t'))
	{
		if(p2 && isdigit(*p2))
			 return(*(p2+1)?0:tty_check(*p2-'0'));
		else
		{
		/* test -t with no arguments */
			ap--;
			return(tty_check(1));
		}
	}
	if((*a=='-' && *(a+2)==0))
	{
		if(!p2)
		{
			/* for backward compatibility with new flags */
			if(a[1]==0 || !strchr(test_unops+9,a[1]))
				return(1);
			sh_fail(e_test, e_argexp);
		}
		if(strchr(test_unops,a[1]))
			return(unop_test(a[1],p2));
	}
	if(!p2)
	{
		ap--;
		return(*a!=0);
	}
skip:
	p1 = sh_lookup(p2,test_optable);
	op = p2;
	if((p1&TEST_BINOP)==0)
		p2 = nxtarg(0);
	if(p1==0)
		sh_fail(op,e_testop);
	return(test_binop(p1,a,p2));
}
#endif /* OLDTEST */

unop_test(op,arg)
register int op;
register char *arg;
{
	switch(op)
	{
	case 'r':
		return(tio(arg, R_OK));
	case 'w':
		return(tio(arg, W_OK));
	case 'x':
		return(tio(arg, X_OK));
	case 'd':
		return(S_ISDIR(test_mode(arg)));
	case 'c':
		return(S_ISCHR(test_mode(arg)));
	case 'b':
		return(S_ISBLK(test_mode(arg)));
	case 'f':
		return(S_ISREG(test_mode(arg)));
	case 'u':
		return(test_mode(arg)&S_ISUID);
	case 'g':
		return(test_mode(arg)&S_ISGID);
	case 'k':
#ifdef S_ISVTX
		return(test_mode(arg)&S_ISVTX);
#else
		return(0);
#endif /* S_ISVTX */
	case 'V':
#ifdef FS_3D
	{
		struct stat statb;
		if(*arg==0 || lstat(arg,&statb)<0)
			return(0);
		return((statb.st_mode&(S_IFMT|S_ISVTX|S_ISUID))==(S_IFDIR|S_ISVTX|S_ISUID));
	}
#else
		return(0);
#endif /* FS_3D */
	case 'L':
	/* -h is not documented, and hopefully will disappear */
	case 'h':
#ifdef LSTAT
	{
		struct stat statb;
		if(*arg==0 || lstat(arg,&statb)<0)
			return(0);
		return((statb.st_mode&S_IFMT)==S_IFLNK);
	}
#else
		return(0);
#endif	/* S_IFLNK */

	case 'C':
#ifdef S_IFCTG
		return(test_type(arg,S_IFMT,S_IFCTG));
#else
		return(0);
#endif	/* S_IFCTG */

	case 'S':
#ifdef S_IFSOCK
		return(test_type(arg,S_IFMT,S_IFSOCK));
#else
		return(0);
#endif	/* S_IFSOCK */

	case 'p':
#ifdef S_ISFIFO
		return(S_ISFIFO(test_mode(arg)));
#else
		return(0);
#endif	/* S_ISFIFO */
	case 'n':
		return(*arg != 0);
	case 'z':
		return(*arg == 0);
	case 's':
	case 'O':
	case 'G':
	{
		struct stat statb;
		if(*arg==0 || test_stat(arg,&statb)<0)
			return(0);
		if(op=='s')
			return(statb.st_size>0);
		else if(op=='O')
			return(statb.st_uid==sh.userid);
		return(statb.st_gid==sh.groupid);
	}
#ifdef NEWTEST
	case 'a':
		return(tio(arg, F_OK));
	case 'o':
		op = sh_lookup(arg,tab_options);
		return(op && is_option((1L<<op))!=0);

	case 't':
		if(isdigit(*arg) && arg[1]==0)
			 return(tty_check(*arg-'0'));
		return(0);
#endif /* NEWTEST */
#ifdef OLDTEST
	default:
	{
		static char a[3] = "-?";
		a[1]= op;
		sh_fail(a,e_testop);
		/* NOTREACHED  */
	}
#endif /* OLDTEST */
	}
}

test_binop(op,left,right)
char *left, *right;
register int op;
{
	register int int1,int2;
	if(op&TEST_ARITH)
	{
		int1 = sh_arith(left);
		int2 = sh_arith(right);
	}
	switch(op)
	{
		/* op must be one of the following values */
#ifdef OLDTEST
		case TEST_AND:
		case TEST_OR:
			ap--;
			return(*left!=0);
#endif /* OLDTEST */
#ifdef NEWTEST
		case TEST_PEQ:
			return(strmatch(left, right));
		case TEST_PNE:
			return(!strmatch(left, right));
		case TEST_SGT:
			return(strcmp(left, right)>0);
		case TEST_SLT:
			return(strcmp(left, right)<0);
#endif /* NEWTEST */
		case TEST_SEQ:
			return(strcmp(left, right)==0);
		case TEST_SNE:
			return(strcmp(left, right)!=0);
		case TEST_EF:
			return(test_inode(left,right));
		case TEST_NT:
			return(ftime_compare(left,right)>0);
		case TEST_OT:
			return(ftime_compare(left,right)<0);
		case TEST_EQ:
			return(int1==int2);
		case TEST_NE:
			return(int1!=int2);
		case TEST_GT:
			return(int1>int2);
		case TEST_LT:
			return(int1<int2);
		case TEST_GE:
			return(int1>=int2);
		case TEST_LE:
			return(int1<=int2);
	}
	/* NOTREACHED */
}

/*
 * returns the modification time of f1 - modification time of f2
 */

static time_t ftime_compare(file1,file2)
char *file1,*file2;
{
	struct stat statb1,statb2;
	if(test_stat(file1,&statb1)<0)
		statb1.st_mtime = 0;
	if(test_stat(file2,&statb2)<0)
		statb2.st_mtime = 0;
	return(statb1.st_mtime-statb2.st_mtime);
}

/*
 * return true if inode of two files are the same
 */

test_inode(file1,file2)
char *file1,*file2;
{
	struct stat stat1,stat2;
	if(test_stat(file1,&stat1)>=0  && test_stat(file2,&stat2)>=0)
		if(stat1.st_dev == stat2.st_dev && stat1.st_ino == stat2.st_ino)
			return(1);
	return(0);
}


/*
 * This version of access checks against effective uid/gid
 * The static buffer statb is shared with test_mode.
 */

sh_access(name, mode)
register char	*name;
register int mode;
{
	if(*name==0)
		return(-1);
	if(strmatch(name,(char*)e_devfdNN))
		return(io_access(atoi(name+8),mode));
	/* can't use access function for execute permission with root */
	if(mode==X_OK && sh.euserid==0)
		goto skip;
	if(sh.userid==sh.euserid && sh.groupid==sh.egroupid)
		return(access(name,mode));
#ifdef SETREUID
	/* swap the real uid to effective, check access then restore */
	/* first swap real and effective gid, if different */
	if(sh.groupid==sh.euserid || setregid(sh.egroupid,sh.groupid)==0) 
	{
		/* next swap real and effective uid, if needed */
		if(sh.userid==sh.euserid || setreuid(sh.euserid,sh.userid)==0)
		{
			mode = access(name,mode);
			/* restore ids */
			if(sh.userid!=sh.euserid)
				setreuid(sh.userid,sh.euserid);
			if(sh.groupid!=sh.egroupid)
				setregid(sh.groupid,sh.egroupid);
			return(mode);
		}
		else if(sh.groupid!=sh.egroupid)
			setregid(sh.groupid,sh.egroupid);
	}
#endif /* SETREUID */
skip:
	if(test_stat(name, &statb) == 0)
	{
		if(mode == F_OK)
			return(mode);
		else if(sh.euserid == 0)
		{
			if(!S_ISREG(statb.st_mode) || mode!=X_OK)
				return(0);
		    	/* root needs execute permission for someone */
			mode = (S_IXUSR|S_IXGRP|S_IXOTH);
		}
		else if(sh.euserid == statb.st_uid)
			mode <<= 6;
		else if(sh.egroupid == statb.st_gid)
			mode <<= 3;
#ifdef MULTIGROUPS
		/* on some systems you can be in several groups */
		else
		{
#   if MUTLIGROUPS>0	/* pre-posix systems */
			register int n = MULTIGROUPS;
			int groups[MULTIGROUPS];
#   else
			gid_t *groups; 
			register int n = getgroups(0,(gid_t*)0);
			groups = (gid_t*)stakalloc(n*sizeof(gid_t));
#   endif /* MUTLIGROUPS>0 */
			n = getgroups(n,groups);
			while(--n >= 0)
			{
				if(groups[n] == statb.st_gid)
				{
					mode <<= 3;
					break;
				}
			}
		}
#   endif /* MULTIGROUPS */
		if(statb.st_mode & mode)
			return(0);
	}
	return(-1);
}


/*
 * Return the mode bits of file <file> 
 * If <file> is null, then the previous stat buffer is used.
 * The mode bits are zero if the file doesn't exist.
 */

test_mode(file)
register char *file;
{
	if(file && (*file==0 || test_stat(file,&statb)<0))
		return(0);
	return(statb.st_mode);
}

/*
 * do an fstat() for /dev/fd/n, otherwise stat()
 */

static int test_stat(f,buff)
char *f;
struct stat *buff;
{
	if(strmatch(f,(char*)e_devfdNN))
		return(fstat(atoi(f+8),buff));
	else
		return(stat(f,buff));
}
