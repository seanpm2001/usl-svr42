/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_RFS_RF_VFS_H	/* wrapper symbol for kernel use */
#define _FS_RFS_RF_VFS_H	/* subject to change without notice */

#ident	"@(#)uts-x86:fs/rfs/rf_vfs.h	1.6"
#ident	"$Header: $"

#ifdef _KERNEL_HEADERS

#ifndef _UTIL_TYPES_H
#include <util/types.h>		/* REQUIRED */
#endif

#ifndef _UTIL_LIST_H
#include <util/list.h>		/* REQUIRED */
#endif

#ifndef _FS_VNODE_H
#include <fs/vnode.h>		/* REQUIRED */
#endif

#ifndef _FS_VFS_H
#include <fs/vfs.h>		/* REQUIRED */
#endif

#ifndef _FS_RFS_NSERVE_H
#include <fs/rfs/nserve.h>	/* REQUIRED */
#endif

#ifndef _FS_RFS_RF_CIRMGR_H
#include <fs/rfs/rf_cirmgr.h>	/* REQUIRED */
#endif

#elif defined(_KERNEL)

#include <sys/types.h>		/* REQUIRED */
#include <sys/list.h>		/* REQUIRED */
#include <sys/vnode.h>		/* REQUIRED */
#include <sys/vfs.h>		/* REQUIRED */
#include <sys/nserve.h>		/* REQUIRED */
#include <sys/rf_cirmgr.h>	/* REQUIRED */

#else

#include <sys/list.h>		/* SVR4.0COMPAT */
#include <sys/types.h>		/* SVR4.0COMPAT */
#include <sys/vnode.h>		/* SVR4.0COMPAT */
#include <sys/vfs.h>		/* SVR4.0COMPAT */

#endif /* _KERNEL_HEADERS */

/*
 * RFS-specific part of VFS.
 */
typedef struct rf_vfs {
	struct rf_vfs	*rfvfs_prev;
	struct rf_vfs	*rfvfs_next;
	vfs_t		*rfvfs_vfsp;	/* to vfs for resource */
	vnode_t		*rfvfs_rootvp;	/* root vnode when mntproc NULL */
	struct proc	*rfvfs_mntproc;	/* process mounting when rootvp NULL */
	int		rfvfs_refcnt;	/* n sndds active in resource */
	ushort		rfvfs_flags;	/* flags, below */
	long		rfvfs_mntid;	/* "index" from static table days */
	ls_elt_t	rfvfs_sdhash;	/* to hash chains for free sndds */
	char		rfvfs_name[MAXDNAME + 1];
					/* resource name */
} rf_vfs_t;
/*
 * flags in rf_vfs
 */
#define MFREE           0x0
#define MFUMOUNT        0x40
#define MLINKDOWN       0x80
#define MCACHE          0x100

/*
 * list header for RFS dependent VFSs
 */
typedef struct rf_head {
	rf_vfs_t  *rfh_prev;
	rf_vfs_t  *rfh_next;
} rf_head_t;
extern rf_head_t       rf_head;

/*
 * for rfs mounts, mount system call gets a pointer to one
 * of these as its "dataptr" arg
 */
typedef struct rf_mountdata {
	ushort  rfm_flags;       /* RFS-specific flags */
	rf_token_t rfm_token;
} rf_mountdata_t;
/*
 * flags in rf_mountdata_t
 */
#define MS_CACHE	0x8

/*
 * generic mount flags of interest
 */
#define MS_FLAGS_TO_RFS(flags)  ((flags) & MS_RDONLY)

/*
 * Give a pointer to an rf_vfs structure, yield ptr to corresponding VFS
 */
#define RFTOVF(rf_vfsp)   ((vfs_t *)((rf_vfsp)->rfvfs_vfsp))
/*
 * Given a pointer to a VFS, yield ptr to corresponding rf_vfs structure
 */
#define VFTORF(vfsp)    ((rf_vfs_t *)((vfsp)->vfs_data))
/* Is this an rf_vfs vnode pointer? */
#define ISRFSVP(vp)      ((vp)->v_op == &rf_vnodeops)
/* Is this a rf_vfst vfs pointer? */
#define ISRFSVFSP(vfsp)   ((vfsp)->vfs_op == &rf_vfsops)

#if defined(_KERNEL)

extern struct vfsops rf_vfsops;
extern struct vnodeops rf_vnodeops;

extern rf_vfs_t *findrfvfs();

#endif

#endif /* _FS_RFS_RF_VFS_H */
