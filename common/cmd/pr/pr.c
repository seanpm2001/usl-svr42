/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pr:pr.c	1.19.2.5"
#ident  "$Header: pr.c 1.2 91/06/27 $"

/*
 *	PR command (print files in pages and columns, with headings)
 *	2+head+2+page[56]+5
 */

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/euc.h>
#include <getwidth.h>
#include <pfmt.h>
#include <string.h>
#include <errno.h>

#define ESC	'\033'
#define LENGTH	66
#define LINEW	72
#define NUMW	5
#define MARGIN	10
#define DEFTAB	8
#define NFILES	9

#define STDINNAME()	nulls
#define PROMPT()	putc('\7', stderr) /* BEL */
#define NOFILE	nulls
#define TABS(N,C)	if ((N = intopt(argv, &C)) < 0) N = DEFTAB
#define ETABS	(Inpos % Etabn)
#define ITABS	(Itabn > 0 && Nspace >= (nc = Itabn - Outpos % Itabn))
#define NSEPC	'\t'
#define PR_HEAD()	pfmt(stdout, MM_NOSTD, ":347:%s  %s Page %d\n\n\n", date, head, Page)
#define done()	if (Ttyout) chmod(Ttyout, Mode)

#define FORMAT	"%b %e %H:%M %Y"	/* ---date time format--- 
					   b -- abbreviated month name 
					   e -- day of month
					   H -- Hour (24 hour version)
					   M -- Minute
					   Y -- Year in the form ccyy */
#define FORMATID ":348"

typedef char CHAR;
typedef int ANY;
typedef unsigned UNS;
typedef struct { FILE *f_f; char *f_name; int f_nextc; } FILS;
typedef struct {int fold; int skip; int eof; } foldinf;

ANY *getspace();

FILS *Files;
FILE *fopen(), *mustopen();

mode_t Mode;
int Multi = 0, Nfiles = 0, Error = 0;
void onintr();

char nulls[] = "";
char *Ttyout, obuf[BUFSIZ];

static char	time_buf[50];	/* array to hold the time and date */

#define OFF	0
#define ON	1
#define ROUNDUP	0.9
eucwidth_t	wp;
int	setswitch = 0;
int	eucwcount = 0;
int	scrwcount = 0;
int	eucstart = OFF;

void
fixtty()
{
	struct stat sbuf;

	setbuf(stdout, obuf);
	if (signal(SIGINT, SIG_IGN) != SIG_IGN) signal(SIGINT, onintr);
	if (Ttyout= ttyname(fileno(stdout))) {		/* is stdout a tty? */
		stat(Ttyout, &sbuf);
		Mode = sbuf.st_mode;		/* save permissions */
		chmod(Ttyout, (S_IREAD|S_IWRITE));
	}
	return;
}

char *GETDATE() /* return date file was last modified */
{
	static char *now = NULL;
	static struct stat sbuf, nbuf;

	if (Nfiles > 1 || Files->f_name == nulls) {
		if (now == NULL)
			{
			time(&nbuf.st_mtime);
			cftime(time_buf, gettxt(FORMATID, FORMAT),
				&nbuf.st_mtime);
			now = time_buf;
			}
		return (now);
	} else {
		stat(Files->f_name, &sbuf);
		cftime(time_buf, gettxt(FORMATID, FORMAT), &sbuf.st_mtime);
		return (time_buf);
	}
}


main(argc, argv) char *argv[];
{
	FILS fstr[NFILES];
	int nfdone = 0;

	(void)setlocale(LC_ALL, "");
	(void)setcat("uxcore");
	(void)setlabel("UX:pr");

	getwidth(&wp);
	wp._eucw2++;
	wp._eucw3++;

	Files = fstr;
	for (argc = findopt(argc, argv); argc > 0; --argc, ++argv)
		if (Multi == 'm') {
			if (Nfiles >= NFILES - 1) die(":349:Too many files\n");
			if (mustopen(*argv, &Files[Nfiles++]) == NULL)
				++nfdone; /* suppress printing */
		} else {
			if (print(*argv))
				fclose(Files->f_f);
			++nfdone;
		}
	if (!nfdone) /* no files named, use stdin */
		print(NOFILE); /* on GCOS, use current file, if any */
	errprint(); /* print accumulated error reports */
	exit(Error);	/*NOTREACHED*/
}


long Lnumb = 0;
FILE *Ttyin = stdin;
int Dblspace = 1, Fpage = 1, Formfeed = 0,
	Length = LENGTH, Linew = 0, Offset = 0, Ncols = 1, Pause = 0, Sepc = 0,
	Colw, Plength, Margin = MARGIN, Numw, Nsepc = NSEPC, Report = 1,
	Etabn = 0, Etabc = '\t', Itabn = 0, Itabc = '\t', fold = 0,
	foldcol = 0, alleof=0;
char *Head = NULL;
CHAR *Buffer = NULL, *Bufend, *Bufptr;
UNS Buflen;
typedef struct { CHAR *c_ptr, *c_ptr0; long c_lno; int c_skip; } *COLP;
COLP Colpts;
double eucbuf = 1, eucbufw = 0;
foldinf *Fcol;
					/* findopt() returns eargc modified to be    */
					/* the number of explicitly supplied         */
					/* filenames, including '-', the explicit    */
					/* request to use stdin.  eargc == 0 implies */
					/* that no filenames were supplied and       */
					/* stdin should be used.		     */
findopt(argc, argv) char *argv[];
{
	char **eargv = argv;
	int eargc = 0, c;
	int mflg = 0, aflg = 0, i;
	void usage();
	fixtty();
	while (--argc > 0) {
		switch (c = **++argv) {
		case '-':
			if ((c = *++*argv) == '\0') break;
		case '+':
			do {
				if (isdigit(c)) {
					--*argv;
					Ncols = atoix(argv);
					}
				else
					switch (c) {
					case '+': if ((Fpage = atoix(argv)) < 1)
							Fpage = 1;
						  continue;
					case 'd': Dblspace = 2;
						  continue;
					case 'e': TABS(Etabn, Etabc);
						  continue;
					case 'f': ++Formfeed;
						  continue;
					case 'h': if (--argc > 0)
						  Head = argv[1];
						  continue;
					case 'i': TABS(Itabn, Itabc);
						  continue;
					case 'l': Length = atoix(argv);
						  continue;
					case 'a': aflg++;
						  Multi = c;
						  continue;
					case 'm': mflg++; 
						  Multi = c; 
						  continue;
					case 'o': Offset = atoix(argv);
						  continue;
					case 'p': ++Pause;
						  continue;
					case 'r': Report = 0;
						  continue;
					case 's': if ((Sepc = (*argv)[1]) != '\0')
							++*argv;
						  else
							Sepc = '\t';
						  continue;
					case 't': Margin = 0;
						  continue;
					case 'w': Linew = atoix(argv);
						  continue;
					case 'n': ++Lnumb;
						  if ((Numw = intopt(argv,&Nsepc)) <= 0)
							Numw = NUMW;
						  continue;
					case 'F': fold++;
						  break;
					default : 
						  pfmt(stderr, MM_ERROR,
						  	"uxlibc:1:Illegal option -- %c\n",
						  	c);
						  usage();
					}
							/* Advance over options    */
							/* until null is found.    */
							/* Allows clumping options */
							/* as in "pr -pm f1 f2".   */
			} while ((c = *++*argv) != '\0');

			if (Head == argv[1]) ++argv;
			continue;
		}
		*eargv++ = *argv;
		++eargc;	/* count the filenames */
	}

	if (mflg && (Ncols > 1)) {
		pfmt(stderr, MM_ERROR,
			":350:Only one of either -m or -column allowed\n");
		usage();
	}
	if (aflg && (Ncols < 2)) {
		pfmt(stderr, MM_ERROR, ":351:-a valid only with -column\n");
		usage();
	}

	if (Ncols == 1 && fold)
		Multi = 'm';

	if (Length == 0) Length = LENGTH;
	if (Length <= Margin) Margin = 0;
	Plength = Length - Margin/2;
	if (Multi == 'm') Ncols = eargc;
	switch (Ncols) {
	case 0:
		Ncols = 1;
	case 1:
		break;
	default:
		if (Multi != 'm') {  /* do not set Etabn and Itabn for -m */
			if (Etabn == 0) /* respect explicit tab specification */
				Etabn = DEFTAB;
			if (Itabn == 0)
				Itabn = DEFTAB;
		}
	}
	if ((Fcol = (foldinf *)malloc(sizeof(foldinf) * Ncols)) == NULL) {
		pfmt(stderr,MM_ERROR, ":31:Out of memory: %s\n", strerror(errno));
		exit(1);
	}
	for ( i=0; i<Ncols; i++)
		Fcol[i].fold = Fcol[i].skip = 0;
	if (Linew == 0) Linew = Ncols != 1 && Sepc == 0 ? LINEW : 512;
	if (Lnumb) {
		int numw;

		if (Nsepc == '\t') {
			if(Itabn == 0)
					numw = Numw + DEFTAB - (Numw % DEFTAB);
			else
					numw = Numw + Itabn - (Numw % Itabn);
		}else {
				numw = Numw + ((ISPRINT(Nsepc, wp)) ? 1 : 0);
		}
		Linew -= (Multi == 'm') ? numw : numw * Ncols;
	}
	if ((Colw = (Linew - Ncols + 1)/Ncols) < 1)
		die(":352:Width too small\n");
	if (Ncols != 1 && Multi == 0) {
		Buflen = ((UNS)(Plength/Dblspace + 1))*(Linew+1)*sizeof(CHAR);
		if (wp._multibyte) {
			if (wp._scrw1>0)
				if ((eucbufw = (double)wp._eucw1/(double)wp._scrw1) > 0)
					if(eucbufw > 1)
						eucbuf = eucbufw;
					else
						eucbuf = 1;
			if (wp._scrw2>0)
				if ((eucbufw = (double)wp._eucw2/(double)wp._scrw2) > 0) {
					if(eucbufw > eucbuf)
						eucbuf = eucbufw;
				}
			if (wp._scrw3>0)
				if ((eucbufw = (double)wp._eucw3/(double)wp._scrw3) > 0) {
					if(eucbufw > eucbuf)
						eucbuf = eucbufw;
				}
		Buflen = (UNS)(Buflen * eucbuf + ROUNDUP);
		}
		Buffer = (CHAR *)getspace(Buflen);
		Bufptr = Bufend = &Buffer[Buflen];
		Colpts = (COLP)getspace((UNS)((Ncols+1)*sizeof(*Colpts)));
		Colpts[0].c_lno = 0;
	}
							/* is stdin not a tty? */
	if (Ttyout && (Pause || Formfeed) && !ttyname(fileno(stdin)))
		Ttyin = fopen("/dev/tty", "r");
	return (eargc);
}

intopt(argv, optp) char *argv[]; int *optp;
{
	int c;

	if ((c = (*argv)[1]) != '\0' && !isdigit(c)) { *optp = c; ++*argv; }
	return ((c = atoix(argv)) != 0 ? c : -1);
}

int Page, C = '\0', Nspace, Inpos;

print(name) char *name;
{
	static int notfirst = 0;
	char *date = NULL, *head = NULL;
	int c;

	if (Multi != 'm' && mustopen(name, &Files[0]) == NULL) return (0);
	if (Multi == 'm' && Nfiles == 0 && mustopen(name, &Files[0]) == NULL)
		die(":353:Cannot open stdin\n");
	if (Buffer) ungetc(Files->f_nextc, Files->f_f);
	if (Lnumb) Lnumb = 1;
	for (Page = 0; ; putpage()) {
		if (C==EOF && !(fold && Buffer)) break;
		if (Buffer) nexbuf();
		Inpos = 0;
		if (get(0) == EOF) break;
		fflush(stdout);
		if (++Page >= Fpage) {
			if (Ttyout && (Pause || Formfeed && !notfirst++)) {
				PROMPT(); /* prompt with bell and pause */
				while ((c = getc(Ttyin)) != EOF && c != '\n') ;
			}
			if (Margin == 0) continue;
			if (date == NULL) date = GETDATE();
			if (head == NULL) head = Head != NULL ? Head :
				Nfiles < 2 ? Files->f_name : nulls;
			printf("\n\n");
			Nspace = Offset;
			putspace();
			PR_HEAD();
		}
	}
	C = '\0';
	return (1);
}

int Outpos, Lcolpos, Pcolpos, Line;

putpage()
{
	register int colno;

	if (fold) {
		foldpage();
		return;
	}
	for (Line = Margin/2; ; get(0)) {
		for (Nspace = Offset, colno = 0, Outpos = 0; C != '\f'; ) {
			if (Lnumb && C != EOF && ((colno == 0 && Multi == 'm') || Multi != 'm')) {
				if (Page >= Fpage) {
					putspace();
					printf("%*ld%c", Numw, Buffer ?
						Colpts[colno].c_lno++ : Lnumb, Nsepc);
				}
				++Lnumb;
			}
			for (Lcolpos = 0, Pcolpos = 0;
				C != '\n' && C != '\f' && C != EOF; get(colno))
					put(C);
			if (C == EOF || ++colno == Ncols ||
				C == '\n' && get(colno) == EOF) break;
			if (Sepc) put(Sepc);
			else if ((Nspace += Colw - Lcolpos + 1) < 1) Nspace = 1;
		}
		if (C == EOF) {
			if (Margin != 0) break;
			if (colno != 0) put('\n');
			return;
		}
		if (C == '\f') break;
		put('\n');
		if (Dblspace == 2 && Line < Plength) put('\n');
		if (Line >= Plength) break;
	}
	if (Formfeed) put('\f');
	else while (Line < Length) put('\n');
}

foldpage()
{
	register int colno;
	int keep, c, i;
	static int  sl;

	for (Line = Margin/2; ; get(0)) {
		for (Nspace = Offset, colno = 0, Outpos = 0; C != '\f'; ) {
			if (Lnumb && Multi == 'm' && foldcol) {
				if (!Fcol[colno].skip) {
					unget(colno);
					putspace();
					if (!colno) {
						for(i=0;i<=Numw;i++) printf(" ");
						printf("%c",Nsepc);
					}
					for(i=0;i<=Colw;i++) printf(" ");
					put(Sepc);
					if (++colno == Ncols) break;
					get(colno);
					continue;
				}
				else if (!colno) Lnumb = sl;
			}

			if (Lnumb && (C != EOF)
			&& ((colno == 0 && Multi == 'm') || Multi != 'm')) {
				if (Page >= Fpage) {
					putspace();
					if ((foldcol &&
					Fcol[colno].skip && Multi!='a') ||
					(Fcol[0].fold && Multi == 'a') ||
					(Buffer && Colpts[colno].c_skip)) {
						for (i=0; i<Numw;i++) printf(" ");
						printf("%c",Nsepc);
						if (Buffer) {
							Colpts[colno].c_lno++;
							Colpts[colno].c_skip = 0;
						}
					}
					else
					printf("%*ld%c", Numw, Buffer ?
					Colpts[colno].c_lno++ : Lnumb, Nsepc);
				}
				sl = Lnumb++;
			}
			for (Lcolpos = 0, Pcolpos = 0;
				C != '\n' && C != '\f' && C != EOF; get(colno))
					if (put(C)) {
						unget(colno);
						Fcol[(Multi=='a')?0:colno].fold = 1;
						break;
					}
					else if (Multi == 'a') {
						Fcol[0].fold = 0;
					}
			if (Buffer) {
				alleof = 1;
				for (i=0; i<Ncols; i++)
					if (!Fcol[i].eof) alleof = 0;
				if (alleof || ++colno == Ncols)
					break;
			} else if (C == EOF || ++colno == Ncols)
				break;
			keep = C;
			get(colno);
			if (keep == '\n' && C == EOF)
				break;
			if (Sepc) put(Sepc);
			else if ((Nspace += Colw - Lcolpos + 1) < 1) Nspace = 1;
		}
		foldcol = 0;
		if (Lnumb && Multi != 'a') {
			for (i=0; i<Ncols; i++) {
				if (Fcol[i].skip = Fcol[i].fold)
					foldcol++;
				Fcol[i].fold = 0;
			}
		}
		if (C == EOF) {
			if (Margin != 0) break;
			if (colno != 0) put('\n');
			return;
		}
		if (C == '\f') break;
		put('\n');
		fflush(stdout);
		if (Dblspace == 2 && Line < Plength) put('\n');
		if (Line >= Plength) break;
	}
	if (Formfeed) put('\f');
	else while (Line < Length) put('\n');
}

nexbuf()
{
	register CHAR *s = Buffer;
	register COLP p = Colpts;
	int j, c, bline = 0;
	int bufcolw;

	if (wp._multibyte)
		bufcolw = (int)(Colw * eucbuf + ROUNDUP);
	else
		bufcolw = Colw;

	if (fold) {
		foldbuf();
		return;
	}
	for ( ; ; ) {
		p->c_ptr0 = p->c_ptr = s;
		if (p == &Colpts[Ncols]) return;
		(p++)->c_lno = Lnumb + bline;
		for (j = (Length - Margin)/Dblspace; --j >= 0; ++bline)
			for (Inpos = 0; ; ) {
				if ((c = getc(Files->f_f)) == EOF) {
					for (*s = EOF; p <= &Colpts[Ncols]; ++p)
						p->c_ptr0 = p->c_ptr = s;
					balance(bline);
					return;
				}
				if (ISPRINT(c, wp)) ++Inpos;
				if (Inpos <= bufcolw || c == '\n') {
					*s = c;
					if (++s >= Bufend)
						die(":354:Page-buffer overflow\n");
				}
				if (c == '\n') break;
				switch (c) {
				case '\b': if (Inpos == 0) --s;
				case ESC:  if (Inpos > 0) --Inpos;
				}
			}
	}
}

foldbuf()
{
	int num, i, colno=0;
	int size = Buflen;
	char *s, *d;
	register COLP p=Colpts;

	for (i =0; i< Ncols; i++)
		Fcol[i].eof = 0;
	d = Buffer;
	if (Bufptr != Bufend) {
		s = Bufptr;
		while (s < Bufend) *d++ = *s++;
		size -= (Bufend - Bufptr);
	}
	Bufptr = Buffer;
	p->c_ptr0 = p->c_ptr = Buffer;
	if (p->c_lno == 0) {
		p->c_lno = Lnumb;
		p->c_skip = 0; 
	}
	else {
		p->c_lno = Colpts[Ncols-1].c_lno;
		if (p->c_skip = Colpts[Ncols].c_skip)
			p->c_lno--;
	}
	if ((num = fread(d, 1, size, Files->f_f)) != size) {
		for (*(d+num) = EOF; (++p)<= &Colpts[Ncols]; ) 
			p->c_ptr0 = p->c_ptr = (d+num);
		balance(0);
		return;
	}
	i = (Length - Margin) / Dblspace;
	do {
		readbuf(&Bufptr, i, p++);
	} while (++colno < Ncols);
}


balance(bline) /* line balancing for last page */
{
	CHAR *s = Buffer;
	register COLP p = Colpts;
	int colno = 0, j, c, l;
	int lines;

	if (!fold) {
		c = bline % Ncols;
		l = (bline + Ncols - 1)/Ncols;
		bline = 0;
		do {
			for (j = 0; j < l; ++j)
				while (*s++ != '\n') ;
			(++p)->c_lno = Lnumb + (bline += l);
			p->c_ptr0 = p->c_ptr = s;
			if (++colno == c) --l;
		} while (colno < Ncols - 1);
	} else {
		lines = readbuf(&s, 0, 0);
		l = (lines + Ncols - 1)/Ncols;
		if (l > ((Length - Margin) / Dblspace)) {
			l = (Length - Margin) / Dblspace;
			c = Ncols;
		} else
			c = lines % Ncols;
		s = Buffer;
		do {
			readbuf(&s, l, p++);
			if (++colno == c) --l;
		} while (colno < Ncols);
		Bufptr = s;
	}
}


readbuf(s, lincol, p)
char **s;
int lincol;
COLP p;
{
	int lines = 0;
	int chars = 0, width;
	int nls = 0, move;
	int skip = 0;
	int decr = 0;

	width = (Ncols == 1) ? Linew : Colw;
	while (**s != (char)EOF) {
		switch (**s) {
			case '\n': lines++; nls++; chars=0; skip = 0;
				  break;
			case '\b':
			case ESC: if (chars) chars--;
				   break;
			case '\t':
				move = Itabn - ((chars + Itabn) % Itabn);
				move = (move < width-chars) ? move : width-chars;
				chars += move;
			default: if (isprint(**s)) chars++;
		}
		if (chars > width) {
			lines++; skip++; decr++; chars = 0; 
		}
		if (lincol && lines == lincol) {
			(p+1)->c_lno = p->c_lno + nls;
			(++p)->c_skip = skip;
			if (**s == '\n') (*s)++;
			p->c_ptr0 = p->c_ptr = (CHAR *)*s;
			return;
		}
		if (decr) decr = 0;
		else (*s)++;
	}
	return(lines);
}

get(colno)
{
	static int peekc = 0;
	register COLP p;
	register FILS *q;
	char c;
	int i;

	if (peekc)
		{ peekc = 0; c = Etabc; }
	else if (Buffer) {
		p = &Colpts[colno];
		if (p->c_ptr >= (p+1)->c_ptr0) c = (char)EOF;
		else if ((c = *p->c_ptr) != (char)EOF) ++p->c_ptr;
		if (fold && c == (char)EOF) Fcol[colno].eof = 1;
	} else if ((c = 
		(q = &Files[Multi == 'a' ? 0 : colno])->f_nextc) == (char)EOF) {
		for (q = &Files[Nfiles]; --q >= Files && q->f_nextc == EOF; ) ;
		if (q >= Files) c = '\n';
	} else
		q->f_nextc = getc(q->f_f);
	if (Etabn != 0 && c == Etabc) {
		++Inpos;
		peekc = ETABS;
		c = ' ';
	} else if (isprint(c) && ISASCII(c))
		++Inpos;
	else
		switch (c) {
		case '\b':
		case ESC:
			if (Inpos > 0) --Inpos;
			break;
		case '\f':
			if (Ncols == 1) break;
			c = '\n';
		case '\n':
		case '\r':
			Inpos = 0;
			break;
		default  :
			if (NOTASCII(c)) {
				if (ISSET2(c)) {
					setswitch = 2;
					eucwcount = wp._eucw2;
					scrwcount = wp._scrw2;
					eucstart = ON;
				} else if (ISSET3(c)) {
					setswitch = 3;
					eucwcount = wp._eucw3;
					scrwcount = wp._scrw3;
					eucstart = ON;
				} else if (!setswitch) {
					setswitch = 1;
					eucwcount = wp._eucw1;
					scrwcount = wp._scrw1;
					eucstart = ON;
				}
				if (eucstart == ON)
					Inpos += scrwcount;
			}
		}
	if (c == (char)EOF) {
		C = EOF;
		return(EOF);
	}
	else
		return (C = c);
}

int put(c)
{
	int move=0;
	int width=Colw;
	int sp=Lcolpos;

	if (fold && Ncols == 1) width = Linew;
	switch (c) {
	case ' ':
		if((!fold && Ncols < 2) || (Lcolpos < width)) {
			++Nspace;
			++Lcolpos;
		}
		goto rettab;
	case '\t':
		if(Itabn == 0)
			break;
		if(Lcolpos < width) {
			move = Itabn - ((Lcolpos + Itabn) % Itabn);
			move = (move < width-Lcolpos) ? move : width-Lcolpos;
			Nspace += move;
			Lcolpos += move;
		}
rettab:
		if (fold && sp == Lcolpos)
		if (Lcolpos >= width)
				return(1);
		return(0);	
	case '\b':
		if (Lcolpos == 0) return(0);
		if (Nspace > 0) { --Nspace; --Lcolpos; return(0); }
		if (Lcolpos > Pcolpos) { --Lcolpos; return(0); }
	case ESC:
		move = -1;
		break;
	case '\n':
		++Line;
	case '\r':
	case '\f':
		Pcolpos = 0; Lcolpos = 0; Nspace = 0; Outpos = 0;
		setswitch = 0;
		eucstart = OFF;
	default:
		move = (isprint(c) != 0);
		if (NOTASCII(c)) {
			if (eucstart == ON)
				move = scrwcount;
			else
				move = 0;
			if (--eucwcount <= 0)
				setswitch = 0;
		}
	}
	if (Page < Fpage) return(0);
	if (Lcolpos > 0 || move > 0) Lcolpos += move;
	putspace();
	if ((!fold && Ncols < 2) || (Lcolpos <= width)) {
		putchar(c);
		Outpos += move;
		Pcolpos = Lcolpos;
		if (eucstart == ON) eucstart = OFF;
	}
	else if (eucstart == ON) {
		eucstart = OFF;
		setswitch = 0;
		if (Lcolpos - scrwcount < Colw) {
			Nspace += (Colw + scrwcount - Lcolpos);
			putspace();
		}
	}
	if (fold && Lcolpos > width)
		return(1);

	return(0);
}

putspace()
{
	int nc;

	for ( ; Nspace > 0; Outpos += nc, Nspace -= nc)
		if (ITABS && !fold)
			putchar(Itabc);
		else {
			nc = 1;
			putchar(' ');
		}
}

unget(colno)
int colno;
{
	if (Buffer) {
		if (*(Colpts[colno].c_ptr-1) != '\t')
			--(Colpts[colno].c_ptr);
		if (Colpts[colno].c_lno) Colpts[colno].c_lno--;
	}
	else {
		if ((Multi == 'm' && colno == 0) || Multi != 'm')
			if (Lnumb && !foldcol) Lnumb--;
		colno = (Multi == 'a') ? 0 : colno;
		ungetc(Files[colno].f_nextc, Files[colno].f_f);
		Files[colno].f_nextc = C;
	}
}


atoix(p) register char **p;
{
	register int n = 0, c;

	while (isdigit(c = *++*p)) n = 10*n + c - '0';
	--*p;
	return (n);
}

/* Defer message about failure to open file to prevent messing up
   alignment of page with tear perforations or form markers.
   Treat empty file as special case and report as diagnostic.
*/
typedef struct err { struct err *e_nextp; char *e_name; int e_errno; } ERR;
ERR *Err = NULL, *Lasterr = (ERR *)&Err;

FILE *mustopen(s, f) char *s; register FILS *f;
{
	int lasterrno;

	if (*s == '\0') {
		f->f_name = STDINNAME();
		f->f_f = stdin;
	} else if ((f->f_f = fopen(f->f_name = s, "r")) == NULL) {
		lasterrno = errno;
		s = strcpy((char *)getspace((UNS)(strlen(f->f_name) + 1)), 
			f->f_name);
	}
	if (f->f_f != NULL) {
		if ((f->f_nextc = getc(f->f_f)) != EOF || Multi == 'm')
			return (f->f_f);
		lasterrno = -1;
		s = strcpy((char *)getspace((UNS)(strlen(f->f_name) + 1)), 
			f->f_name);
		fclose(f->f_f);
	}
	Error = 1;
	if (Report)
		if (Ttyout) { /* accumulate error reports */
			Lasterr = Lasterr->e_nextp = (ERR *)getspace((UNS)sizeof(ERR));
			Lasterr->e_nextp = NULL;
			Lasterr->e_name = s;
			Lasterr->e_errno = lasterrno;
		} else { /* ok to print error report now */
			cerror(s, lasterrno);
		}
	return ((FILE *)NULL);
}

ANY *getspace(n) UNS n;
{
	ANY *t;

	if ((t = (ANY *)malloc(n)) == NULL) die(":31:Out of memory: %s\n", strerror(errno));
	return (t);
}

die(s, a1, a2, a3) char *s, *a1, *a2, *a3;
{
	++Error;
	errprint();
	pfmt(stderr, MM_ERROR, s, a1, a2, a3);
	exit(1);
}

void onintr()
{
	++Error;
	errprint();
	_exit(1);
}

errprint() /* print accumulated error reports */
{
	fflush(stdout);
	for ( ; Err != NULL; Err = Err->e_nextp)
		cerror(Err->e_name, Err->e_errno);
	done();
}
void
usage()
{
char *usage2 = "[-column [-wwidth] [-a]] [-ect] [-ict] [-drtfp] [+page] [-nsk]";
char *usage3 = "[-ooffset] [-llength] [-sseparator] [-h header] [-F] [file ...]";
char *usage4 = "[-m      [-wwidth]]      [-ect] [-ict] [-drtfp] [+page] [-nsk]";
char *usage5 = "[-ooffset] [-llength] [-sseparator] [-h header] [-F] file1 file2 ...";
char *usage2id = ":355";
char *usage3id = ":356";
char *usage4id = ":357";
char *usage5id = ":358";

	pfmt(stderr, MM_ACTION,
		":359:Usage:\n\tpr %s  \\\n\t   %s\n\n\tpr %s  \\\n\t   %s\n",
		gettxt(usage2id, usage2), gettxt(usage3id, usage3),
		gettxt(usage4id, usage4), gettxt(usage5id, usage5));
	exit(1);
}

cerror(name, error)
char *name;
int error;
{
	if (error == -1)
		pfmt(stderr, MM_ERROR, ":360:Empty file -- %s\n", name);
	else
		pfmt(stderr, MM_ERROR, ":92:Cannot open %s: %s\n", name,
			strerror(error));
}
