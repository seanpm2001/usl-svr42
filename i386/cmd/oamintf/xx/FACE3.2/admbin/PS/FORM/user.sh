#!/sbin/sh
#	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:i386/cmd/oamintf/xx/FACE3.2/admbin/PS/FORM/user.sh	1.1.1.2"
#ident	"$Header: $"
( /usr/bin/awk -F: '{ if ( $3 == 0 || $3 >=100 ) print $1 }' /etc/passwd ; echo none\\nall ) > /usr/tmp/user.$VPID
