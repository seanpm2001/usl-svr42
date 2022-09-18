/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-x86:vpix/v86subr.c	1.1"
#ident	"$Header: $"

/*
 * LIM 4.0 changes:
 * Copyright (c) 1989 Phoenix Technologies Ltd.
 * All Rights Reserved
*/


#ifdef VPIX
#include <util/sysmacros.h>
#include <util/param.h>
#include <util/types.h>
#include <mem/immu.h>
#include <util/cmn_err.h>
#include <svc/systm.h>
#include <proc/signal.h>
#include <svc/errno.h>
#include <proc/proc.h>
#include <fs/file.h>
#include <fs/vnode.h>
#include <io/uio.h>
#include <proc/tss.h>
#include <vpix/v86.h>
#include <util/fp.h>
#include <proc/user.h>
#include <svc/resource.h>
#include <util/debug.h>
#include <util/var.h>
#include <fs/rfs/rf_messg.h>
#include <io/conf.h>
#include <svc/bootinfo.h>
#include <proc/mman.h>
#include <mem/as.h>
#include <mem/seg.h>
#include <mem/seg_vpix.h>


/* Core dump of a VPIX dual more process. This dumps the XTSS and
 * all valid pages in the V86 process into the file core.vpix.
 */

int
core_vpix(pp, credp, rlimit)
	proc_t		*pp;
	struct cred	*credp;
	rlim_t		rlimit;
{
	struct vnode *vp;
	extern int	userstack[];
	off_t	offset;
	caddr_t base;
	int	count;
	struct vattr vattr;
	int error = 0;

	vattr.va_type = VREG;
	vattr.va_mode = 0666;
	vattr.va_mask = AT_TYPE|AT_MODE;
	PTOU(pp)->u_syscall = DUCOREDUMP;

	error = vn_create("core.vpix", UIO_SYSSPACE, &vattr,
	  NONEXCL, VWRITE, &vp, CRCORE);

	if (error)
		return error;

	if (VOP_ACCESS(vp, VWRITE, 0, u.u_cred) || vp->v_type != VREG)
		error = EACCES;
	else {
		/*
		 * Put the text, data and stack sizes (in pages)
		 * into the u-block for the dump.
		 */
		
		PTOU(pp)->u_tsize = 0;
		PTOU(pp)->u_dsize = V86VIRTSIZE;
		PTOU(pp)->u_ssize = btoc(pp->p_v86->vp_szxtss);

		/*
		 * Check the sizes against the current ulimit and
		 * don't write a file bigger than ulimit.  If we
		 * can't write everything, we would prefer to
		 * write the data and not the "stack" (xtss) rather than
		 * the other way around.
		*/

		if (ctob(USIZE + PTOU(pp)->u_dsize + PTOU(pp)->u_ssize) > rlimit) {
			PTOU(pp)->u_ssize = 0;
			if (ctob(USIZE + PTOU(pp)->u_dsize) > rlimit)
				PTOU(pp)->u_dsize = 0;
		}



		vattr.va_size = 0;
		vattr.va_mask = AT_SIZE;
		(void) VOP_SETATTR(vp, &vattr, 0, credp);

		error = vn_rdwr(UIO_WRITE, vp, (caddr_t)PTOU(pp), ctob(USIZE),
		  (off_t) 0, UIO_SYSSPACE, 0, rlimit, credp, (int *)NULL);
		offset = ctob(USIZE);

		/* Write the data and stack to the dump file. */
		
		if (error == 0 && PTOU(pp)->u_dsize) {
			base = (caddr_t)0;
			count = ctob (PTOU(pp)->u_dsize);
			error = core_seg(pp, vp, offset, base, count, rlimit, credp);
			offset += ctob(btoc(count));
		}

		if (error == 0 && PTOU(pp)->u_ssize) {
			base = (caddr_t) XTSSADDR;
			count = ctob (PTOU(pp)->u_ssize);
			error = core_seg(pp, vp, offset, base, count, rlimit, credp);
		}

	}

	VN_RELE(vp);

	return error == 0;
}

#endif /* VPIX */
