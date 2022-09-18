#!/sbin/sh
#	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)filemgmt:common/cmd/oamintf/files/bin/dirchk.sh	1.1.7.2"
#ident  "$Header: dirchk.sh 2.1 91/09/12 $"

file=${1}
if test -d ${file}
then
	exit 0
else
	echo "   $file is not the full path name of a valid directory."
	exit 1
fi
