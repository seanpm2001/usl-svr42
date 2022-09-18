#	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)egrep:egrep.mk	1.2.8.2"
#ident "$Header: egrep.mk 1.2 91/04/08 $"

#	Makefile for egrep

include $(CMDRULES)

INSDIR = $(USRBIN)
OWN = bin
GRP = bin

LDLIBS = -lw -ly

#top#
# Generated by makefile 1.47

MAKEFILE = egrep.mk


MAINS = egrep

OBJECTS =  egrep.o

SOURCES =  egrep.y

all:		$(MAINS)

egrep:		egrep.o	
	$(CC) -o $@ egrep.o $(LDFLAGS) $(LDLIBS) $(SHLIBS)

egrep.o:	 $(INC)/stdio.h $(INC)/ctype.h $(INC)/wctype.h \
		 $(INC)/pfmt.h $(INC)/errno.h $(INC)/string.h

GLOBALINCS = $(INC)/ctype.h $(INC)/stdio.h $(INC)/wctype.h \
		$(INC)/pfmt.h $(INC)/errno.h $(INC)/string.h

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(MAINS)

newmakefile:
	makefile -m -f $(MAKEFILE)  -s INC $(INC)
#bottom#

install: all
	$(INS) -f $(INSDIR) -m 0555 -u $(OWN) -g $(GRP) $(MAINS)

size: all
	$(SIZE) $(MAINS)

strip: all
	$(STRIP) $(MAINS)

#	These targets are useful but optional

partslist:
	@echo $(MAKEFILE) $(SOURCES) $(LOCALINCS)  |  tr ' ' '\012'  |  sort

productdir:
	@echo $(INSDIR) | tr ' ' '\012' | sort

product:
	@echo $(MAINS)  |  tr ' ' '\012'  | \
	sed 's;^;$(INSDIR)/;'

srcaudit:
	@fileaudit $(MAKEFILE) $(LOCALINCS) $(SOURCES) -o $(OBJECTS) $(MAINS)
