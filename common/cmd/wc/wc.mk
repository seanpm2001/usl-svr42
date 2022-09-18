#	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T
#	 All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)wc:wc.mk	1.6.4.1"
#ident "$Header: wc.mk 1.2 91/03/13 $"

include $(CMDRULES)

#	Makefile for wc 

OWN = bin
GRP = bin

all: wc

wc: wc.o 
	$(CC) -o wc wc.o  $(LDFLAGS) $(LDLIBS) $(PERFLIBS)

wc.o:	$(INC)/stdio.h \
	$(INC)/ctype.h \
	$(INC)/locale.h \
	$(INC)/pfmt.h \
	$(INC)/errno.h \
	$(INC)/string.h

install: all
	 $(INS) -f $(USRBIN) -m 00555 -u $(OWN) -g $(GRP) wc

clean:
	rm -f wc.o

clobber: clean
	rm -f wc

lintit:
	$(LINT) $(LINTFLAGS) wc.c

#	These targets are useful but optional

partslist:
	@echo wc.mk wc.c $(LOCALINCS) | tr ' ' '\012' | sort

productdir:
	@echo $(USRBIN) | tr ' ' '\012' | sort

product:
	@echo wc | tr ' ' '\012' | \
	sed 's;^;$(USRBIN)/;'

srcaudit:
	@fileaudit wc.mk $(LOCALINCS) wc.c -o wc.o wc
