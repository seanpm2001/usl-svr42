/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)profiler:prfpr.c	1.13.6.5"
#ident	"$Header: $"

/*
 *	prfpr - print profiler log files
 */

#include <stdio.h>
#include <time.h>
#include <a.out.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/prf.h>

struct	profile	{
	time_t	prf_ld;		/* time stamp of prf load 		*/
	time_t	p_date;		/* time stamp of record 		*/
	int	*p_ctr;		/* prf counter values and total		*/
	long	*sum;		/* per processor sums and total		*/
} p[2];

int symcnt;			/* number of symbols			*/
int prfmax;			/* number of text symbols		*/
int Nengine;			/* number of processors			*/
double cutoff = 1.0;		/* cutoff percentage, default 1%	*/
struct mprf *t;			/* addresses and offsets into name_tbl	*/
char *name_tbl;
char *logfile;

main( int argc, char **argv ) {
	register int ff, log, i;
	int tflg, xflg, errflg;
	extern char *optarg;
	extern int optind;
	double *pc;
	int *cdiff;
	int tot_diff;
	int c, printit, n;
	int namesz;

	void warning( const char *s );
	void error( const char *s );
	void shtime( time_t *l );
	void printname( int ent );
	void add_rows( int *p_ctr );
	void add_cols( int *p_ctr, long *sum );

	void *calloc();
	void *malloc();

	tflg = xflg = errflg = 0;
	while(( c = getopt( argc, argv, "tx" ) ) != EOF ) {
		switch( c ) {
		case 't':
			if( tflg++ )
				errflg++;
			break;
		case 'x':
			if( xflg++ )
				errflg++;
			break;
		default:
			errflg++;
			break;
		}
	}

	switch(argc - optind) {
		case 3:
			--argc;
			warning("namelist argument ignored");
		case 2:
			cutoff = atof(argv[--argc]);
		case 1:
			logfile = argv[--argc];
			break;
		default:
			errflg++;
			break;
	}
	if( errflg )
		error("usage: prfpr [-tx] file [ cutoff ]");

	if((log = open(logfile, O_RDONLY)) < 0)
		error("cannot open data file");
	ff = 0;
	if(read(log, &(p[!ff].prf_ld), sizeof (time_t)) 
	  != sizeof (time_t)) 
		exit(0);

new_header:
	if(read(log, &Nengine, sizeof Nengine) != sizeof( Nengine )
	  || Nengine == 0)
		error("bad data file");

	if(read(log, &prfmax, sizeof prfmax) != sizeof( prfmax )
	  || prfmax == 0)
		error("bad data file");

	if(read(log, &namesz, sizeof namesz) != sizeof( namesz )
	  || namesz == 0)
		error("bad data file");
	
	if(cutoff > 100.0 || cutoff < 0.0 )
		error("invalid cutoff percentage");

	p[0].p_ctr = (int *) calloc((prfmax+1)*(Nengine+1),sizeof (int));
	p[1].p_ctr = (int *) calloc((prfmax+1)*(Nengine+1),sizeof (int));
	t = (struct mprf *) malloc(prfmax * sizeof (struct mprf));
	name_tbl =  malloc( namesz );
	pc = (double *)calloc( Nengine + 1, sizeof ( double ) );
	cdiff = (int *)calloc( Nengine + 1, sizeof ( int ) );
	p[0].sum = (long *)calloc( Nengine + 1, sizeof ( int ) );
	p[1].sum = (long *)calloc( Nengine + 1, sizeof ( int ) );
	if( !p[0].p_ctr || !p[1].p_ctr  || !t || !name_tbl
	  || !p[0].sum || !p[1].sum )
		error("cannot malloc space");

	if(read(log, t, prfmax * sizeof (struct mprf)) 
	  != prfmax * sizeof (struct mprf))
		error("cannot read profile addresses");

	if(read(log, name_tbl, namesz) != namesz)
		error("cannot read profile function names");
	(void) read(log, &(p[!ff].p_date), sizeof (time_t));
	(void) read(log, p[!ff].p_ctr, (prfmax + 1) * Nengine * sizeof (int));
	add_rows( p[!ff].p_ctr );
	add_cols( p[!ff].p_ctr, p[!ff].sum );

	for( ;; ff = !ff ) {
		if( read(log, &(p[ff].prf_ld), sizeof (time_t)) !=
		  sizeof(time_t) ) {
			exit(0);
		}
		if( p[ff].prf_ld != p[!ff].prf_ld ) {
			free(p[0].p_ctr);
			free(p[1].p_ctr);
			free(t);
			free(name_tbl);
			free(p[0].sum);
			free(p[1].sum);
			ff = !ff;
			goto new_header;
		}

		if(read(log, &(p[ff].p_date), sizeof (time_t)) 
		  != sizeof (time_t)) {
			exit(0);
		}
		if(read(log, p[ff].p_ctr, (prfmax +1) * Nengine 
		  * sizeof (int)) != (prfmax + 1) * Nengine * sizeof (int))
			error("corrupted log file");
		shtime(&p[!ff].p_date);
		shtime(&p[ff].p_date);
		printf("\n");
		add_rows( p[ff].p_ctr );
		add_cols( p[ff].p_ctr, p[ff].sum );

		tot_diff = p[ff].sum[Nengine] - p[!ff].sum[Nengine];
		if( tot_diff == 0 ) {
			printf("no samples\n\n");
			continue;
		}
		n = 0;
		if( xflg && Nengine > 1 ) {
			printf("function     	");
			for( ; n < Nengine; n++ ) 
				printf(" cpu(%02d)",n);
			printf("   total\n");
		}
		for(i = 0; i <= prfmax; i++) {
			printit = 0;
			n = 0;
			if( !xflg || Nengine == 1 ) 
				n = Nengine;
			for( ; n <= Nengine; n++ ) {
				cdiff[n] = p[ff].p_ctr[n*(prfmax+1)+i] 
				  - p[!ff].p_ctr[n*(prfmax+1)+i];
				pc[n] = 100.0 * (double)cdiff[n]/
				  (double)tot_diff;
				if( pc[n] > cutoff )
					printit = 1;
			}
			if ( printit ) {
				if(i == prfmax)
					printf("user         	");
				else
					printname(t[i].mprf_offset);
				n = 0;
				if( !xflg || Nengine == 1 ) 
					n = Nengine;
				for( ; n <= Nengine; n++ )
					if( !tflg )
						printf(" %7.2f", pc[n]);
					else
						printf(" %7d", cdiff[n]);
				printf("\n");
			}
		}
		if( tflg || ( xflg && Nengine > 1 ) )  {
			n = 0;
			if( !xflg ) 
				n = Nengine;
			for( ; n <= Nengine; n++ ) {
				cdiff[n] = p[ff].sum[n] 
				  - p[!ff].sum[n];
				pc[n] = 100.0 * (double)cdiff[n]
				  /(double)tot_diff;
			}			
			printf("Total      	");
			n = 0;
			if( !xflg || Nengine == 1 ) 
				n = Nengine;
			for( ; n <= Nengine; n++ )
				if( !tflg )
					printf(" %7.2f", pc[n]);
				else
					printf(" %7d", cdiff[n]);
			printf("\n");
		}
		printf("\n");
	}
}

void
error( const char *s ) {
	printf("error: %s\n", s);
	exit(1);
}

void
warning( const char *s ) {
	printf("warning: %s\n", s);
}

void
shtime( time_t *l ) {
	register  struct  tm  *t;

	if(*l == (time_t) 0) {
		printf("initialization\n");
		return;
	}
	t = localtime(l);
	printf("%02.2d/%02.2d/%02.2d %02.2d:%02.2d:%02.2d\n",
	  t->tm_mon + 1, t->tm_mday, t->tm_year, t->tm_hour,
	  t->tm_min, t->tm_sec);
}

void
printname( int ent ) {
	printf("%-13s	", (name_tbl + ent));
}
 
void
add_rows( int *p_ctr ) {
	int i, n;

	for(i = 0; i <= prfmax; i++) {
		p_ctr[Nengine*(prfmax+1)+i] = 0;
		for( n = 0; n < Nengine ; n++ ) {
			p_ctr[Nengine*(prfmax+1)+i] 
			  += p_ctr[n*(prfmax+1)+i];
		}
	}
}

void
add_cols( int *p_ctr, long *sum ) {
	int i,n,cntr;

	sum[ Nengine ] = 0L;
	for( n = 0; n < Nengine ; n++ ) {	
		sum[ n ] = 0L;
		for(i = 0; i <= prfmax; i++) {
			cntr = p_ctr[n*(prfmax+1)+i];
			sum[ n ] += cntr;
			sum[ Nengine ] += cntr;
		}
	}
}

