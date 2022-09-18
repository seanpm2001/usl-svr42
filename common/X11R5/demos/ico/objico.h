/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)r4ico:objico.h	1.1"
/* objico.h - structure values for icosahedron */

{	"icosahedron", "ico",	/* long and short names */
	"dodecahedron",		/* long name of dual */
	12, 30, 20,	/* number of vertices, edges, and faces */
	{		/* vertices (x,y,z) */
			/* all points must be within radius 1 of the origin */
		{ 0.00000000,  0.00000000, -0.95105650},
		{ 0.00000000,  0.85065080, -0.42532537},
		{ 0.80901698,  0.26286556, -0.42532537},
		{ 0.50000000, -0.68819095, -0.42532537},
		{-0.50000000, -0.68819095, -0.42532537},
		{-0.80901698,  0.26286556, -0.42532537},
		{ 0.50000000,  0.68819095,  0.42532537},
		{ 0.80901698, -0.26286556,  0.42532537},
		{ 0.00000000, -0.85065080,  0.42532537},
		{-0.80901698, -0.26286556,  0.42532537},
		{-0.50000000,  0.68819095,  0.42532537},
		{ 0.00000000,  0.00000000,  0.95105650}
	},
	{	/* faces (numfaces + indexes into vertices) */
		/*  faces must be specified clockwise from the outside */
		 3,	0,  2,  1,
		 3,	0,  3,  2,
		 3,	0,  4,  3,
		 3,	0,  5,  4,
		 3,	0,  1,  5,
		 3,	1,  6, 10,
		 3,	1,  2,  6,
		 3,	2,  7,  6,
		 3,	2,  3,  7,
		 3,	3,  8,  7,
		 3,	3,  4,  8,
		 3,	4,  9,  8,
		 3,	4,  5,  9,
		 3,	5, 10,  9,
		 3,	5,  1, 10,
		 3,	10,  6, 11,
		 3,	6,  7, 11,
		 3,	7,  8, 11,
		 3,	8,  9, 11,
		 3,	9, 10, 11
	}
},		/* leave a comma to separate from the next include file */
/* end */
