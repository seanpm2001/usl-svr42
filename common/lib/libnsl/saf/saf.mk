#	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)libnsl:common/lib/libnsl/saf/saf.mk	1.16.8.4"
#ident "$Header: saf.mk 1.3 91/03/14 $"

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#	PROPRIETARY NOTICE (Combined)
#
# This source code is unpublished proprietary information
# constituting, or derived under license from AT&T's UNIX(r) System V.
# In addition, portions of such source code were derived from Berkeley
# 4.3 BSD under license from the Regents of the University of
# California.
#
#
#
#	Copyright Notice 
#
# Notice of copyright on this source code product does not indicate 
#  publication.
#
#	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
#	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
#          All rights reserved.
# 

include $(LIBRULES)

LOCALDEF = $(PICFLAG)
OBJECTS=	doconfig.o checkver.o
LIBOBJECTS=	../doconfig.o ../checkver.o
SRCS=		$(OBJECTS:%.o=%.c)

all:		$(OBJECTS)
		cp $(OBJECTS) ../


doconfig.o:	doconfig.c \
		$(INC)/stdio.h \
		$(INC)/fcntl.h \
		$(INC)/ctype.h \
		$(INC)/sys/types.h \
		$(INC)/stropts.h \
		$(INC)/sys/conf.h \
		$(INC)/sys/errno.h \
		$(INC)/signal.h \
		$(INC)/sac.h \
		$(INC)/signal.h

checkver.o:	checkver.c \
		$(INC)/stdio.h

lint:
		$(LINT) $(CFLAGS) $(SRCS)

clean:
		rm -f $(OBJECTS)

clobber:	clean
		rm -f $(LIBOBJECTS)

size:		all
		$(SIZE) $(LIBOBJECTS)

strip:		all
		$(STRIP) $(LIBOBJECTS)
