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

#ident	"@(#)banner:banner.c	1.4.1.3"
#ident  "$Header: banner.c 1.2 91/06/26 $"
#include <locale.h>
#include <stdio.h>
#include <pfmt.h>

#define nchars 128	/*number of chars in char set*/
#define nlines  7	/*number of lines in a banner character*/
#define pposs  85	/*number of print positions on a line (must be multiple of 4)*/
			/*followed by end of string character*/
#define pospch 8	/*number of char positions per banner char*/
#define chpln  10	/*number of banner characters per line*/

struct bann{
	  char alpha[nlines][pposs];
};
struct bann buffer, *bp = &buffer;
char ctbl[nchars][nlines] = {
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/*below 040*/
	0,000,000,000,000,000,000,	/* */
	034,034,034,010,0,034,034,	/*!*/
	0167,0167,042,0,0,0,0,	/*"*/
	024,024,0177,024,0177,024,024,	/*#*/
	076,0111,0110,076,011,0111,076,	/*$*/
	0161,0122,0164,010,027,045,0107,	/*%*/
	030,044,030,070,0105,0102,071,	/*&*/
	034,034,010,020,0,0,0,	/*'*/
	014,020,040,040,040,020,014,	/*(*/
	030,4,2,2,2,4,030,	/*)*/
	0,042,024,0177,024,042,0,	/***/
	0,010,010,076,010,010,0,	/*+*/
	0,0,0,034,034,010,020,	/*,*/
	0,0,0,076,0,0,0,	/*-*/
	0,0,0,0,034,034,034,	/*.*/
	1,2,4,010,020,040,0100,	/*/*/
	034,042,0101,0101,0101,042,034,	/*0*/
	010,030,050,010,010,010,076,	/*1*/
	076,0101,1,076,0100,0100,0177,	/*2*/
	076,0101,1,076,1,0101,076,	/*3*/
	0100,0102,0102,0102,0177,2,2,	/*4*/
	0177,0100,0100,0176,1,0101,076,	/*5*/
	076,0101,0100,0176,0101,0101,076,	/*6*/
	0177,0102,04,010,020,020,020,	/*7*/
	076,0101,0101,076,0101,0101,076,	/*8*/
	076,0101,0101,077,1,0101,076,	/*9*/
	010,034,010,0,010,034,010,	/*:*/
	034,034,0,034,034,010,020,	/*;*/
	4,010,020,040,020,010,4,	/*<*/
	0,0,076,0,076,0,0,	/*=*/
	020,010,4,2,4,010,020,	/*>*/
	076,0101,1,016,010,0,010,	/*?*/
	076,0101,0135,0135,0136,0100,076,	/*@*/
	010,024,042,0101,0177,0101,0101,	/*A*/
	0176,0101,0101,0176,0101,0101,0176,	/*B*/
	076,0101,0100,0100,0100,0101,076,	/*C*/
	0176,0101,0101,0101,0101,0101,0176,	/*D*/
	0177,0100,0100,0174,0100,0100,0177,	/*E*/
	0177,0100,0100,0174,0100,0100,0100,	/*F*/
	076,0101,0100,0117,0101,0101,076,	/*G*/
	0101,0101,0101,0177,0101,0101,0101,	/*H*/
	034,010,010,010,010,010,034,	/*I*/
	1,1,1,1,0101,0101,076,	/*J*/
	0102,0104,0110,0160,0110,0104,0102,	/*K*/
	0100,0100,0100,0100,0100,0100,0177,	/*L*/
	0101,0143,0125,0111,0101,0101,0101,	/*M*/
	0101,0141,0121,0111,0105,0103,0101,	/*N*/
	0177,0101,0101,0101,0101,0101,0177,	/*O*/
	0176,0101,0101,0176,0100,0100,0100,	/*P*/
	076,0101,0101,0101,0105,0102,075,	/*Q*/
	0176,0101,0101,0176,0104,0102,0101,	/*R*/
	076,0101,0100,076,1,0101,076,	/*S*/
	0177,010,010,010,010,010,010,	/*T*/
	0101,0101,0101,0101,0101,0101,076,	/*U*/
	0101,0101,0101,0101,042,024,010,	/*V*/
	0101,0111,0111,0111,0111,0111,066,	/*W*/
	0101,042,024,010,024,042,0101,	/*X*/
	0101,042,024,010,010,010,010,	/*Y*/
	0177,2,4,010,020,040,0177,	/*Z*/
	076,040,040,040,040,040,076,/*[*/
	0100,040,020,010,004,002,001,	/*\*/
	076,2,2,2,2,2,076,	/*]*/
	010,024,042,0,0,0,0,	/*^*/
	0,000,000,000,000,000,0177,	/*_*/
	034,034,010,04,0,0,0,	/*`*/
	0,014,022,041,077,041,041,	/*A*/
	0,076,041,076,041,041,076,	/*B*/
	0,036,041,040,040,041,036,	/*C*/
	0,076,041,041,041,041,076,	/*D*/
	0,077,040,076,040,040,077,	/*E*/
	0,077,040,076,040,040,040,	/*F*/
	0,036,041,040,047,041,036,	/*G*/
	0,041,041,077,041,041,041,	/*H*/
	0,004,004,004,004,004,004,	/*I*/
	0,001,001,001,001,041,036,	/*J*/
	0,041,042,074,044,042,041,	/*K*/
	0,040,040,040,040,040,077,	/*L*/
	0,041,063,055,041,041,041,	/*M*/
	0,041,061,051,045,043,041,	/*N*/
	0,036,041,041,041,041,036,	/*O*/
	0,076,041,041,076,040,040,	/*P*/
	0,036,041,041,045,042,035,	/*Q*/
	0,076,041,041,076,042,041,	/*R*/
	0,036,040,036,001,041,036,	/*S*/
	0,037,004,004,004,004,004,	/*T*/
	0,041,041,041,041,041,036,	/*U*/
	0,041,041,041,041,022,014,	/*V*/
	0,041,041,041,055,063,041,	/*W*/
	0,041,022,014,014,022,041,	/*X*/
	0,021,012,004,004,004,004,	/*Y*/
	0,077,002,004,010,020,077,	/*Z*/
	034,040,040,0140,040,040,034,	/*{*/
	010,010,010,0,010,010,010,	/*|*/
	034,2,2,3,2,2,034,	/*}*/
	060,0111,06,0,0,0,0,	/*~*/
	0,000,000,000,000,000,000 	/*DEL*/
};
char blank = ' ';
char plot = '#';
int  msk = 0100; /* ? */

main(argc,argp)
char **argp;int argc;
{
	int i;

	(void)setlocale(LC_ALL, "");
	(void)setcat("uxue");
	(void)setlabel("UX:banner");

	/*if invoked with no arguments, prints error comment;
	  if invoked with an argument, prints it in banner form.
	*/

	if(argc<2){
		pfmt(stderr, MM_ERROR, ":1:Incorrect usage\n");
		pfmt(stderr, MM_ACTION,
			":2:Usage: banner \"up to 10 char arg string\" . . .\n");
		exit(1);
	}
	for(i = 1; i<argc ; ++i) {
		banner(argp[i],bp);
		banprt(bp);
		printf("\n");
	}
	exit(0);
}

banner(s,bufp)
char *s;struct bann *bufp;
{
	char c,*p,*q;
	struct bann *r;
	p=s;
	r=bufp;
	banset(blank,bufp);

	while((c= *s++)!=0){
	  if((s-p)>chpln)return(s-p);
	  q=ctbl[c];
	  banfil(q,r);
	  r = (struct bann *)((char *)r + pospch);
	}
}

banfil(c,p)
char *c;
struct bann *p;
{
	int i,j;
	for(i=0;i<nlines;i++){
	  for(j=0;j<pospch;j++){
	    if(((c[i]<<j)&msk)!=0)p->alpha[i][j] = plot;
	  }
	}
	return(0);
}

banset(c,p)
char c;
struct bann *p;
{
	int i,j;
	for(i=0;i<nlines;i++)
	  for(j=0;j<pposs-1;j++)
	    p->alpha[i][j] = c;
}

banprt(ptr)
struct bann *ptr;
{
	int i,j;
	for(i=0;i<nlines;i++){
	  ptr->alpha[i][pposs-1]='\0';
	  for(j=pposs-2;j>=0;j--){
	    if(ptr->alpha[i][j]!=blank)break;
	    ptr->alpha[i][j]='\0';
	  }
	printf("%s\n",ptr->alpha[i]);
	}
}
