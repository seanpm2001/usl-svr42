#ident	"@(#)xpr:devices/terminfo/R_16.c	1.2"
/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

#include "Xlib.h"

#include "xpr.h"

#include "xpr_term.h"
#include "text.h"

static Word		bits[] = {
	0x007c0000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000800,
	0x00000000,0x03ff0000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x0000cb00,0x00000000,0x0fffc000,0x00000000,0x01000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00003e00,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x0003cbc0,0x00000000,0x1f87e000,0x03330062,
	0x03802020,0x07801805,0x00300000,0x0000000f,0x0040e00e,
	0x00207e03,0x8ff8780e,0x00000000,0x00000000,0x3800ffc0,
	0x0200fe00,0x1f07f007,0xff9ffe07,0xc0fc7f7f,0x1fbf1f9f,
	0x807800f7,0x80f81f03,0xf800f81f,0xc003c000,0x0fc3f7e0,
	0xf7efc3df,0xc7cfc1e7,0xff8f807c,0x60000200,0x08000003,
	0x0000e000,0x60030630,0x03000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x000308c0,0x10000000,0x3e38f000,
	0x03330066,0x0ff0f8e0,0x0fc01c0d,0x80300080,0x0000011f,
	0x81c1f83f,0x00607e0f,0x8ff8fe1f,0x80000000,0x00000000,
	0x7e038060,0x0200ffe0,0x7fe7ff07,0xff9ffe1f,0xf8fc7f7f,
	0x1fbf1f9f,0x807c01f7,0xc0f87fc3,0xff03fe1f,0xfc07f3ff,
	0xefc3f7e0,0xf7efc3df,0xc7cfc1e7,0xff8fa07c,0xf0000600,
	0x38000007,0x0001f000,0xe00706f0,0x0f000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00020843,0xf60c0000,
	0x38fc7800,0x03330066,0x1d31cfc0,0x0c401c10,0xc1b20080,
	0x00000338,0xc3c3fc3f,0x80e0fe1c,0x1ff18631,0xc0000018,
	0x00018000,0x87060010,0x070030f0,0xf0e1cfc1,0x8186063c,
	0x38301c1c,0x0e0e0c06,0x001c01c1,0xc060e0e0,0xc7870f06,
	0x1c0c331c,0xe300c3c0,0x63838187,0x838780c6,0x070c200c,
	0xd8000c00,0x18000003,0x00033000,0x60070670,0x03000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00020843,
	0xf1c00000,0x71ff3c00,0x033300c6,0x191180c0,0x0c400c30,
	0x61d60080,0x00000230,0xc0c61c41,0x80e08038,0x10318260,
	0xc00000f8,0x0001f000,0xc70c0008,0x07003071,0xc06181e1,
	0x80860270,0x08301c1c,0x060c1806,0x001e03c1,0xe021c070,
	0xc1ce0386,0x0e0c1a1c,0x2300c1c0,0x61c38103,0x8303c184,
	0x0f0c200d,0x88000e00,0x18000003,0x00030000,0x60000030,
	0x03000000,0x00000000,0x00000001,0x00000000,0x00000000,
	0x00020842,0x070f0000,0x73ff1c00,0x032200cc,0x19138980,
	0x0cc00c60,0x21fe0080,0x00000260,0x60c40c41,0x81e1c070,
	0x10318260,0xc00003e0,0x00007c00,0xc3181c88,0x07803031,
	0xc02180e1,0x80060270,0x0c301c1c,0x060c3006,0x001e03c1,
	0xf0218070,0xc1cc0386,0x0e0c0a1c,0x230041c0,0x41c18301,
	0xc601c300,0x0e0c300c,0x00000e00,0x18000003,0x00030000,
	0x60000030,0x03000000,0x00000000,0x00000003,0x00000000,
	0x00000000,0x00020840,0x02380000,0xe3ff0c00,0x03000fff,
	0x9d030980,0x0dc01860,0x30780080,0x00000660,0x60c40c01,
	0x8361f860,0x0021c660,0xe6601f00,0x00000f80,0xc7183f80,
	0x0d803033,0x80018071,0x810604e0,0x04301c1c,0x060c6006,
	0x001702c1,0x78238038,0xc1dc01c6,0x0e0e081c,0x030040e0,
	0xc0c1c201,0xec00e300,0x1c0c100c,0x00000e3c,0x19c0f03b,
	0x070fe0f0,0x67030633,0xe309c702,0x701e06e0,0x3d06c3e7,
	0xce38f1de,0x78df73e7,0x3fc20840,0x00400000,0xe3038e00,
	0x03001fff,0x8f030b00,0x0fbe10c0,0x31ce0080,0x00000460,
	0x60c00c03,0x0261fcff,0x0060ec60,0x66607c00,0xfff803e0,
	0x06306384,0x09c03073,0x80018071,0x810604e0,0x00301c1c,
	0x060cc006,0x001706c1,0x3c238038,0xc1dc01c6,0x0e0f801c,
	0x030040e0,0x80e3c200,0xf800f600,0x3c0c100c,0x0000047e,
	0x1be1f87f,0x1fcfe1fe,0x6f8f1e33,0xe33bef8e,0xf83f9df8,
	0x7f1fe7e7,0xce38f1de,0x78df73e7,0x3fc20840,0x00310000,
	0xc3038e00,0x01001fff,0x8fc31238,0x1e1c00c0,0x39960080,
	0x00000460,0x60c0180f,0x86607cff,0x8060f870,0x6663e000,
	0xfff80078,0x0630c184,0x19c03fe3,0x80018031,0xff07fce0,
	0x3e3ffc1c,0x060f8006,0x001384c1,0x1c238038,0xc39c01c6,
	0x3c07c01c,0x03004071,0x80e2c600,0x70007400,0x780c180c,
	0x000000c6,0x1ef318c7,0x10c3031e,0x79871e33,0x033e79c7,
	0x98638f78,0xc31fe623,0x0618708c,0x30c661c2,0x03860860,
	0x001c0000,0xc3038600,0x01000318,0x03e3e67c,0x3f1800c0,
	0x18303ffe,0x00000c60,0x60c0181f,0x8c601ee1,0xc0407c70,
	0xe0038000,0x0000001c,0x0c31c304,0x10c03fe3,0x80018031,
	0xff07fce0,0x3e3ffc1c,0x060fc006,0x00138cc1,0x0e238038,
	0xff9c01c7,0xf803f01c,0x03004071,0x0062e400,0x78003c00,
	0x700c080c,0x000000c3,0x18361983,0x3fe3030c,0x61830636,
	0x030830c6,0x18c1c619,0x83060623,0x0618308e,0x3087c0c2,
	0x070e0870,0x060c0000,0xc7038600,0x01000310,0x01f1c4c4,
	0x671800c0,0x18303ffe,0x00000860,0x60c03003,0xd86006c1,
	0xc0c0de3f,0xe003e000,0x0000007c,0x08218300,0x30e030f3,
	0x80018031,0x81060ce0,0x1c301c1c,0x060fe006,0x0011c8c1,
	0x0f238038,0xfe1c01c7,0xf001f01c,0x03004039,0x0076e400,
	0x78003800,0xe00c0c0c,0x0000004f,0x18360183,0x3fe3030c,
	0x6183063c,0x030830c6,0x18c0c61d,0x83060783,0x06183906,
	0x398380e6,0x071c0838,0x07ff0000,0xcf01ce00,0x01003fff,
	0x01700dc4,0xe3b000c0,0x18000080,0x03f00860,0x60c06001,
	0xdff806c0,0xc0c18f1e,0xc000f800,0xfff801f0,0x08218308,
	0x3fe0303b,0x80018071,0x810604e0,0x0c301c1c,0x060cf006,
	0x0011d8c1,0x07a38038,0xc01c01c6,0x7800781c,0x0300403b,
	0x00347c00,0xdc001801,0xe00c040c,0x0000003b,0x18360183,
	0x3003030c,0x6183063c,0x030830c6,0x18c0c60d,0x830603e3,
	0x06181906,0x39018064,0x0e0e0870,0x00380000,0xcfffce00,
	0x00003fff,0x01301984,0xc3e000c0,0x18000080,0x03f01860,
	0x60c0c000,0xdff806c0,0xc0c18701,0xc0001f00,0xfff80f80,
	0x00218708,0x3ff0383b,0x80018071,0x800600e0,0x0c301c1c,
	0x060c7806,0x0010f0c1,0x03e38038,0xc01c01c6,0x3c10381c,
	0x0300801a,0x003c7801,0x9e001801,0xc00c060c,0x00000062,
	0x18360183,0x300301f8,0x6083063e,0x030830c6,0x18c0c60d,
	0x830601e3,0x06181b03,0x5d03c074,0x1c060840,0x001c0000,
	0xefffcc00,0x00000660,0x11101984,0xc1e000c0,0x18000080,
	0x00001070,0x60c08000,0xdff806c0,0xc1818301,0x800007c0,
	0x00003e00,0x00318610,0x60703839,0xc00180e1,0x80460070,
	0x0c301c1c,0x060c3c06,0x0110f0c1,0x01e18070,0xc00e0386,
	0x1c10181c,0x0380801e,0x00383803,0x0f001803,0x800c020c,
	0x000000c2,0x18360583,0x302301f0,0x61830637,0x030830c6,
	0x18e0c619,0x830600f3,0x06181e03,0x4e02e03c,0x1c420840,
	0x00000000,0x6fffdc00,0x01000660,0x11303188,0xe0f100c0,
	0x30000080,0x30019030,0xc0c10200,0x80600461,0x81818303,
	0x866000f8,0x0001f000,0x1030db70,0x40383839,0xe03181e1,
	0x80c60078,0x0c301c1c,0x660c1e06,0x0310e0c1,0x00e1c070,
	0xc00e0386,0x0e18181c,0x0381800c,0x00183806,0x07001807,
	0x808c020c,0x000000c6,0x18370dc3,0x38630300,0x61830633,
	0x830830c6,0x18e18619,0xc3060433,0x07180e03,0xce066038,
	0x38420840,0x00000000,0x70383c00,0x03000c60,0x1d702198,
	0xf7ff0040,0x30000080,0x3801b038,0xc0c3fe73,0x80639c71,
	0x8181c60f,0x06700038,0x0001c000,0x1818f3c0,0xc03838f0,
	0xf0e187c1,0xc3c7003c,0x3c381c1c,0x660e0f07,0x0f1860e1,
	0x80e0e1e0,0xc0070f07,0x0f1c381c,0x01e3800c,0x0018300e,
	0x0780380f,0x038c030c,0x000000ff,0x9c63f8ff,0x9fc303fc,
	0x61c30633,0xc71c70c6,0x18718631,0xff0e0633,0xa7fe0c01,
	0x860c7038,0x70c20840,0x07fc0000,0x3c38f800,0x03000cc0,
	0x1fe061f0,0x7f3e0060,0x70000080,0x3801a01f,0x83f7fc7f,
	0x0063f83f,0x0300fe3c,0x06700008,0x00000000,0x181c0011,
	0xe07effe0,0x7fc7ff07,0xffdf801f,0xf8fc7f7f,0x7c3f1fdf,
	0xfe7c63f7,0xc0607fc3,0xf003fe1f,0x87dff07f,0x00ff000c,
	0x0010101f,0x0fe0fe0f,0xff8c010c,0x000000fb,0x9fc1f0ff,
	0x9f8fc3fe,0xfbef86fb,0xefbefbef,0xbe3f07f0,0xff1f07e1,
	0xc3fe0401,0x841c7810,0x7fc20840,0x00000000,0x1f83e000,
	0x00000000,0x01000000,0x00000030,0x40000000,0x18000000,
	0x00000000,0x00000000,0x00000000,0x00300000,0x00000000,
	0x00070060,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00007c00,0x00000000,
	0x00000000,0x00000000,0x00000000,0x000c000c,0x03ff8000,
	0x00000000,0x00000602,0x00000600,0x00000000,0x00000600,
	0x03000000,0x00000000,0x00000030,0x000208c0,0x00000000,
	0x0fffc000,0x00000000,0x00000000,0x00000018,0xc0000000,
	0x30000000,0x00000000,0x00000000,0x00000000,0x00600000,
	0x00000000,0x0001ffc0,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00003e00,
	0x00000000,0x00000000,0x00000000,0x00000000,0x000e001c,
	0x03ff8000,0x00000000,0x00000606,0x00002600,0x00000000,
	0x00000600,0x03000000,0x00000000,0x00000160,0x000308c0,
	0x00000000,0x03ff8000,0x00000000,0x00000000,0x0000000d,
	0x80000000,0x20000000,0x00000000,0x00000000,0x00000000,
	0x00400000,0x00000000,0x00003e00,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000fc0,0x00000000,0x00000000,0x00000000,0x00000000,
	0x000f807c,0x00000000,0x00000000,0x000007fc,0x00003400,
	0x00000000,0x00000e00,0x03000000,0x00000000,0x000003e0,
	0x0003cbc0,0x00000000,0x00fc0000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x000001c0,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x000003f8,
	0x00003c00,0x00000000,0x00001f00,0x0fc00000,0x00000000,
	0x000003c0,0x0000cb00,0x03df0000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00001f00,0x00000000,
	0x00000000,0x00000100,0x00000800,0x06f00000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
	0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,
};

static XImage		image = {
	1125,				/* width */
	25,				/* height */
	0,				/* xoffset */
	XYBitmap,			/* format */
	(char *)bits,			/* data */
	LSBFirst,			/* byte_order */
	WORDSIZE,			/* bitmap_unit */
	MSBFirst,			/* bitmap_bit_order */
	WORDSIZE,			/* bitmap_pad */
	1,				/* depth */
	ROUNDUP(1125, WORDSIZE) / 8,	/* bytes_per_line */
	1,				/* bits_per_pixel */
};

struct {
	short			n;
	char			height,
				ascent;
	XImage			*image;
	_Fontchar		info[129];
}			R_16 = {
	127,
	 25,
	 18,
	&image,
	{
		{   0,  18,  18,   0,   0 },
		{   0,  18,  18,   0,   0 },
		{   0,  18,  18,   0,   0 },
		{   0,  18,  18,   0,   0 },
		{   0,  18,  18,   0,   0 },
		{   0,  18,  18,   0,   0 },
		{   0,  18,  18,   0,   0 },
		{   0,   0,  23,   0,  24 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,   0,   0,   0 },
		{  24,   0,  25,   0,  12 },
		{  36,   3,  19,   0,   5 },
		{  41,   3,  18,   0,   8 },
		{  49,   3,  19,   0,  17 },
		{  66,   2,  20,   0,  11 },
		{  77,   3,  19,   0,  18 },
		{  95,   3,  19,   0,  18 },
		{ 113,   3,  18,   0,   6 },
		{ 119,   3,  22,   0,   8 },
		{ 127,   3,  22,   0,   7 },
		{ 134,   3,  18,   0,  10 },
		{ 144,   4,  19,   0,  16 },
		{ 160,  16,  22,   0,   6 },
		{ 166,  13,  18,   0,   7 },
		{ 173,  16,  19,   0,   5 },
		{ 178,   4,  19,   0,   7 },
		{ 185,   3,  19,   0,  11 },
		{ 196,   3,  19,   0,   9 },
		{ 205,   3,  19,   0,  11 },
		{ 216,   3,  19,   0,  11 },
		{ 227,   3,  19,   0,  11 },
		{ 238,   3,  19,   0,  10 },
		{ 248,   3,  19,   0,  11 },
		{ 259,   3,  19,   0,  11 },
		{ 270,   3,  19,   0,  11 },
		{ 281,   3,  19,   0,  11 },
		{ 292,   8,  19,   0,   4 },
		{ 296,   8,  22,   0,   5 },
		{ 301,   5,  19,   0,  17 },
		{ 318,   9,  18,   0,  16 },
		{ 334,   5,  18,   0,  17 },
		{ 351,   3,  19,   0,  10 },
		{ 361,   2,  22,   0,  22 },
		{ 383,   3,  19,   0,  17 },
		{ 400,   3,  19,   0,  14 },
		{ 414,   3,  19,   0,  15 },
		{ 429,   3,  19,   0,  16 },
		{ 445,   3,  19,   0,  14 },
		{ 459,   3,  19,   0,  13 },
		{ 472,   3,  19,   0,  16 },
		{ 488,   3,  19,   0,  17 },
		{ 505,   3,  19,   0,   8 },
		{ 513,   3,  19,   0,   9 },
		{ 522,   3,  19,   0,  17 },
		{ 539,   3,  19,   0,  14 },
		{ 553,   3,  19,   0,  20 },
		{ 573,   3,  19,   0,  17 },
		{ 590,   3,  19,   0,  16 },
		{ 606,   3,  19,   0,  13 },
		{ 619,   3,  23,   0,  16 },
		{ 635,   3,  19,   0,  16 },
		{ 651,   3,  19,   0,  11 },
		{ 662,   4,  19,   0,  14 },
		{ 676,   3,  19,   0,  17 },
		{ 693,   3,  19,   0,  16 },
		{ 709,   3,  19,   0,  22 },
		{ 731,   3,  19,   0,  17 },
		{ 748,   3,  19,   0,  16 },
		{ 764,   3,  19,   0,  14 },
		{ 778,   3,  22,   0,   8 },
		{ 786,   4,  19,   0,   7 },
		{ 793,   3,  22,   0,   6 },
		{ 799,   3,  18,   0,   7 },
		{ 806,  18,  21,   0,  12 },
		{ 818,   3,  18,   0,   6 },
		{ 824,   8,  19,   0,  10 },
		{ 834,   3,  19,   0,  11 },
		{ 845,   8,  19,   0,  10 },
		{ 855,   3,  19,   0,  11 },
		{ 866,   8,  19,   0,  10 },
		{ 876,   3,  19,   0,   9 },
		{ 885,   8,  23,   0,  11 },
		{ 896,   3,  19,   0,  12 },
		{ 908,   3,  19,   0,   6 },
		{ 914,   3,  23, 255,   5 },
		{ 920,   3,  19,   0,  12 },
		{ 932,   3,  19,   0,   6 },
		{ 938,   8,  19,   0,  18 },
		{ 956,   8,  19,   0,  12 },
		{ 968,   8,  19,   0,  11 },
		{ 979,   8,  24,   0,  12 },
		{ 991,   8,  23,   0,  12 },
		{ 1003,   8,  19,   0,   9 },
		{ 1012,   8,  19,   0,   9 },
		{ 1021,   6,  19,   0,   7 },
		{ 1028,   8,  19,   0,  12 },
		{ 1040,   8,  19,   0,  11 },
		{ 1051,   8,  19,   0,  16 },
		{ 1067,   8,  19,   0,  11 },
		{ 1078,   8,  24,   0,  11 },
		{ 1089,   8,  19,   0,  10 },
		{ 1099,   1,  23,   0,   8 },
		{ 1107,   0,  24,   0,   3 },
		{ 1110,   1,  23,   0,   8 },
		{ 1118,   3,  18,   0,   7 },
		{ 1125,   0,   0,   0,   0 },
		{ 1125,   0,   0,   0,   0 },
	}
};
