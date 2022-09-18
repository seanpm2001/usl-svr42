/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-x86:proc/class/ts.cf/Space.c	1.8"
#ident	"$Header: $"

#include <config.h>   /* for tunable parameters */

#include <sys/types.h>
#include <sys/tspriocntl.h>
#include <sys/ts.h>

short	ts_maxupri=TSMAXUPRI;


tsdpent_t	ts_dptbl[] = {
				   0,   40,   0,   10,   3,  10,
				   1,   40,   0,   11,   3,  11,
				   2,   40,   1,   12,   3,  12,
				   3,   40,   1,   13,   3,  13,
				   4,   40,   2,   14,   3,  14,
				   5,   40,   2,   15,   3,  15,
				   6,   40,   3,   16,   3,  16,
				   7,   40,   3,   17,   3,  17,
				   8,   40,   4,   18,   3,  18,
				   9,   40,   4,   19,   3,  19,
				  10,   30,   5,   20,   3,  20,
				  11,   30,   5,   21,   3,  21,
				  12,   30,   6,   22,   3,  22,
				  13,   30,   6,   23,   3,  23,
				  14,   30,   7,   24,   3,  24,
				  15,   30,   7,   25,   2,  25,
				  16,   30,   8,   26,   2,  26,
				  17,   30,   8,   27,   2,  27,
				  18,   30,   9,   28,   2,  28,
				  19,   30,   9,   29,   2,  29,
				  20,   20,  10,   30,   2,  30,
				  21,   20,  11,   31,   2,  31,
				  22,   20,  12,   32,   2,  32,
				  23,   20,  13,   33,   2,  33,
				  24,   20,  14,   34,   2,  34,
				  25,   20,  15,   35,   2,  35,
				  26,   20,  16,   36,   2,  36,
				  27,   20,  17,   37,   2,  37,
				  28,   20,  18,   38,   2,  38,
				  29,   20,  19,   39,   2,  39,
				  30,   15,  20,   40,   2,  40,
				  31,   15,  21,   40,   2,  40,
				  32,   15,  22,   41,   2,  41,
				  33,   15,  23,   41,   2,  41,
				  34,   15,  24,   42,   2,  42,
				  35,   15,  25,   42,   2,  42,
				  36,   15,  26,   43,   2,  43,
				  37,   15,  27,   43,   2,  43,
				  38,   15,  28,   44,   2,  44,
				  39,   15,  29,   44,   2,  44,
				  40,   10,  30,   45,   2,  45,
				  41,   10,  31,   45,   2,  45,
				  42,   10,  32,   46,   2,  46,
				  43,   10,  33,   46,   2,  46,
				  44,   10,  34,   47,   2,  47,
				  45,    5,  35,   47,   1,  47,
				  46,    5,  36,   48,   1,  48,
				  47,    5,  37,   48,   1,  48,
				  48,    5,  38,   49,   1,  49,
				  49,    5,  39,   49,   1,  49,
				  };

int	ts_kmdpris[] = {
			50,51,52,53,54,55,56,57,58,59,
			60,61,62,63,64,65,66,67,68,69,
			70,71,72,73,74,75,76,77,78,79,
			80,81,82,83,84,85,86,87,88,89,
			90,91,92,93,94,95,96,97,98,99
			};

short	ts_maxkmdpri = sizeof(ts_kmdpris)/4 - 1;
short	ts_maxumdpri = sizeof(ts_dptbl)/16 - 1;
