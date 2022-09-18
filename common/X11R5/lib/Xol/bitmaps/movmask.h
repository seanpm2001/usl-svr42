/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef NOIDENT
#ident	"@(#)olbitmaps:bitmaps/movmask.h	1.1"
#endif

#define movmask_width 18
#define movmask_height 18
#define movmask_x_hot 1
#define movmask_y_hot 1
static unsigned char movmask_bits[] = {
   0x07, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x7f, 0x00, 0x00, 0xfe, 0x01, 0x00,
   0xfe, 0x07, 0x00, 0xfc, 0x0f, 0x00, 0xfc, 0x0f, 0x00, 0xf8, 0xe7, 0x03,
   0xf8, 0xf7, 0x03, 0xf0, 0xff, 0x03, 0xf0, 0xff, 0x03, 0x60, 0xfe, 0x03,
   0x00, 0xff, 0x03, 0x80, 0xff, 0x03, 0x80, 0xff, 0x03, 0x80, 0xff, 0x03,
   0x80, 0xff, 0x03, 0x80, 0xff, 0x03};
