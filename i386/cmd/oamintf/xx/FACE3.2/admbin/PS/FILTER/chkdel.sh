#!/sbin/sh
#	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.



#ident	"@(#)oamintf:i386/cmd/oamintf/xx/FACE3.2/admbin/PS/FILTER/chkdel.sh	1.1.1.2"
#ident	"$Header: $"
if [ "$1" = "" ]
then	echo 1
else
	if echo "$1" | /usr/bin/grep all > /dev/null
	then	
		echo 2
	else
		echo 3
	fi
fi
