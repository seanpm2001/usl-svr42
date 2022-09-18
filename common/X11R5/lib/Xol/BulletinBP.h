/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	NOIDENT
#ident	"@(#)bboard:BulletinBP.h	2.1"
#endif

#ifndef _BULLETINBOARDP_H
#define _BULLETINBOARDP_H

#include "Xol/ManagerP.h"
#include "Xol/BulletinBo.h"
#include "Xol/LayoutExtP.h"

/*
 * Class structure:
 */

typedef struct _BulletinBoardClassPart {
	XtPointer		extension;
}			BulletinBoardClassPart;

typedef struct _BulletinBoardClassRec {
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	ManagerClassPart	manager_class;
	BulletinBoardClassPart	bulletin_class;
}			BulletinBoardClassRec;

extern BulletinBoardClassRec	bulletinBoardClassRec;

#define BULLETIN_C(WC) ((BulletinBoardWidgetClass)(WC))->bulletin_class

/*
 * Instance structure:
 */

typedef struct _BulletinBoardPart {
	/*
	 * Public:
	 */
	OlLayoutResources	layout;
	OlDefine		old_layout;	/* obsolete */

	/*
	 * Private:
	 */
}			BulletinBoardPart;

typedef struct _BulletinBoardRec {
	CorePart		core;
	CompositePart		composite;
	ConstraintPart		constraint;
	ManagerPart		manager;
	BulletinBoardPart	bulletin;
}			BulletinBoardRec;

#define BULLETIN_P(W) ((BulletinBoardWidget)(W))->bulletin

#endif
