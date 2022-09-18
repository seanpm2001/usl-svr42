#	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


#ident	"@(#)expr:common/cmd/expr/expr.mk	1.9.6.5"
#ident "$Header: expr.mk 1.3 91/08/13 $"

#	Makefile for expr

include $(CMDRULES)

INSDIR = $(USRBIN)
OWN = bin
GRP = bin

LDLIBS = -lgen

#top#
# Generated by makefile 1.47

MAKEFILE = expr.mk

MAINS = expr expr.dy

OBJECTS =  expr.o expres.o

SOURCES =  expr.c expres.c

all:		$(MAINS)

expr:		$(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS) $(NOSHLIBS)

expr.dy:		$(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) $(LDLIBS) -dy

expr.o: expr.c \
	$(INC)/stdlib.h \
	$(INC)/regexpr.h \
	$(INC)/locale.h \
	$(INC)/pfmt.h \
	expr.h

expres.o: expres.c \
	expr.h

install: expr expr.dy
	$(INS) -f $(INSDIR) -m 0555 -u $(OWN) -g $(GRP) expr
	$(INS) -f $(INSDIR) -m 0555 -u $(OWN) -g $(GRP) expr.dy

clean:
	rm -f $(OBJECTS)

clobber: clean
	rm -f $(MAINS)

lintit:
	$(LINT) $(LINTFLAGS) $(SOURCES)

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
