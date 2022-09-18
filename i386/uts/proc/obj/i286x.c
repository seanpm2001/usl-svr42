/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-x86:proc/obj/i286x.c	1.3"
#ident	"$Header: $"

#include <util/types.h>
#include <util/param.h>
#include <util/sysmacros.h>
#include <proc/user.h>
#include <svc/errno.h>
#include <proc/proc.h>
#include <util/cmn_err.h>
#include <util/debug.h>
#include <proc/exec.h>

#define I286S5EMUL	"/usr/bin/i286emul"
#define I286SMAGIC	0512
#define I286LMAGIC	0522

short	i286x_magic[2] = {
	I286SMAGIC,
	I286LMAGIC
};


/* ARGSUSED */
int
i286x_exec(vp, args, level, execsz, ehdp, setid)
struct vnode *vp;
struct uarg *args;
int level;
long	 *execsz;
exhda_t *ehdp;
int setid;
{
	int	error;

	if ((error = setemulate(I286S5EMUL, vp, args, execsz)) != 0)
		return error;

	/*
	 * For a 286 emulator, we set UE_EMUL to allow the emulator to
	 * read or map a 286 binary for which it doesn't have read access.
	 * The emulator should clear this bit (by making the appropriate
	 * sysi86() call) as soon as it has opened or mapped the binary.
	 */
	u.u_renv |= UE_EMUL;

	return -1;	/* Special emulator return */
}


int
i286x_core()
{
	return ENOSYS;
}
