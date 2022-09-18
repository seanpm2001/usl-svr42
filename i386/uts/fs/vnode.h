/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ifndef _FS_VNODE_H	/* wrapper symbol for kernel use */
#define _FS_VNODE_H	/* subject to change without notice */

#ident	"@(#)uts-x86:fs/vnode.h	1.10"
#ident	"$Header: $"

#ifdef _KERNEL_HEADERS

#ifndef _UTIL_TYPES_H
#include <util/types.h>		/* REQUIRED */
#endif

#ifndef _SVC_TIME_H
#include <svc/time.h>		/* REQUIRED */
#endif

#ifndef _ACC_MAC_MAC_H
#include <acc/mac/mac.h>	/* REQUIRED */
#endif

#elif defined(_KERNEL)

#include <sys/types.h>		/* REQUIRED */
#include <sys/time.h>		/* REQUIRED */
#include <sys/mac.h>		/* REQUIRED */

#else

#include <sys/time.h>		/* SVR4.0COMPAT */

#endif /* _KERNEL_HEADERS */

/*
 * The vnode is the focus of all file activity in UNIX.
 * A vnode is allocated for each active file, each current
 * directory, each mounted-on file, and the root.
 */

/*
 * vnode types.  VNON means no type.  These values are unrelated to
 * values in on-disk inodes.
 */
typedef enum vtype {
	VNON	= 0,
	VREG	= 1,
	VDIR	= 2,
	VBLK	= 3,
	VCHR	= 4,
	VLNK	= 5,
	VFIFO	= 6,
	VXNAM	= 7,
	VBAD	= 8
} vtype_t;

typedef struct vnode {
	u_short		v_flag;			/* vnode flags (see below) */
	u_short		v_count;		/* reference count */
	struct vfs	*v_vfsmountedhere;	/* ptr to vfs mounted here */
	struct vnodeops	*v_op;			/* vnode operations */
	struct vfs	*v_vfsp;		/* ptr to containing VFS */
	struct stdata	*v_stream;		/* associated stream */
	struct page	*v_pages;		/* vnode pages list */
	enum vtype	v_type;			/* vnode type */
	dev_t		v_rdev;			/* device (VCHR, VBLK) */
	caddr_t		v_data;			/* private data for fs */
	struct filock	*v_filocks;		/* ptr to filock list */
	lid_t		v_lid;			/* Level IDentifier (MAC) */
	lid_t		v_cmwlid; 		/* Level IDentifier (MAC) CMW */
	u_long		v_macflag;		/* vnode MAC flags */
	long		v_filler[3];		/* padding */
} vnode_t;

/*
 * Vnode flags.
 */
#define	VROOT		0x01	/* root of its file system */
#define VNOMAP		0x04	/* file cannot be mapped/faulted */
#define	VDUP		0x08	/* file should be dup'ed rather then opened */
#define VNOSWAP		0x10	/* file cannot be used as virtual swap device */
#define	VNOMOUNT 	0x20	/* file cannot be covered by mount */
#define VISSWAP		0x40	/* vnode is part of virtual swap device */
/* XENIX Support */
#define VXLOCKED 0x8000	/* Xenix frlock */
/* End XENIX Support */

/*
 * Vnode MAC flags.
 */
#define	VMAC_OPEN	0x01	/* open in progress */
#define	VMAC_OPENWANT	0x02
#define	VMAC_CHANGE	0x04	/* MAC change in progress */
#define	VMAC_CHANGEWANT	0x08
#define VMAC_DOPEN	0x10	/* delay MAC checks in vn_open  */
#define VMAC_DIOCTL	0x20	/* delay MAC checks for ioctls  */
#define VMAC_SUPPORT	0x40	/* filesystem supports levels */
#define VMAC_ISMLD	0x80	/* denotes an multi level directory */

/*
 * Operations on vnodes.
 */
typedef struct vnodeops {
	int	(*vop_open)();
	int	(*vop_close)();
	int	(*vop_read)();
	int	(*vop_write)();
	int	(*vop_ioctl)();
	int	(*vop_setfl)();
	int	(*vop_getattr)();
	int	(*vop_setattr)();
	int	(*vop_access)();
	int	(*vop_lookup)();
	int	(*vop_create)();
	int	(*vop_remove)();
	int	(*vop_link)();
	int	(*vop_rename)();
	int	(*vop_mkdir)();
	int	(*vop_rmdir)();
	int	(*vop_readdir)();
	int	(*vop_symlink)();
	int	(*vop_readlink)();
	int	(*vop_fsync)();
	void	(*vop_inactive)();
	int	(*vop_fid)();
	void	(*vop_rwlock)();
	void	(*vop_rwunlock)();
	int	(*vop_seek)();
	int	(*vop_cmp)();
	int	(*vop_frlock)();
	int	(*vop_space)();
	int	(*vop_realvp)();
	int	(*vop_getpage)();
	int	(*vop_putpage)();
	int	(*vop_map)();
	int	(*vop_addmap)();
	int	(*vop_delmap)();
	int	(*vop_poll)();
	int	(*vop_dump)();	/* not used */
	int	(*vop_pathconf)();
	int	(*vop_allocstore)();
	int	(*vop_getacl)();
	int	(*vop_getaclcnt)();
	int	(*vop_setacl)();
	int	(*vop_setlevel)();
	int	(*vop_getdvstat)();
	int	(*vop_setdvstat)();
	int	(*vop_makemld)();
	int	(*vop_filler[23])();
} vnodeops_t;

#define	VOP_OPEN(vpp, mode, cr) (*(*(vpp))->v_op->vop_open)(vpp, mode, cr)
#define	VOP_CLOSE(vp, f, c, o, cr) (*(vp)->v_op->vop_close)(vp, f, c, o, cr)
#define	VOP_READ(vp,uiop,iof,cr) (*(vp)->v_op->vop_read)(vp,uiop,iof,cr)
#define	VOP_WRITE(vp,uiop,iof,cr) (*(vp)->v_op->vop_write)(vp,uiop,iof,cr)
#define	VOP_IOCTL(vp,cmd,a,f,cr,rvp) (*(vp)->v_op->vop_ioctl)(vp,cmd,a,f,cr,rvp)
#define	VOP_SETFL(vp, f, a, cr) (*(vp)->v_op->vop_setfl)(vp, f, a, cr)
#define	VOP_GETATTR(vp, vap, f, cr) (*(vp)->v_op->vop_getattr)(vp, vap, f, cr)
#define	VOP_SETATTR(vp, vap, f, cr) (*(vp)->v_op->vop_setattr)(vp, vap, f, cr)
#define	VOP_ACCESS(vp, mode, f, cr) (*(vp)->v_op->vop_access)(vp, mode, f, cr)
#define	VOP_LOOKUP(vp,cp,vpp,pnp,f,rdir,cr) \
		(*(vp)->v_op->vop_lookup)(vp,cp,vpp,pnp,f,rdir,cr)
#define	VOP_CREATE(dvp,p,vap,ex,mode,vpp,cr) \
		(*(dvp)->v_op->vop_create)(dvp,p,vap,ex,mode,vpp,cr)
#define	VOP_REMOVE(dvp,p,cr) (*(dvp)->v_op->vop_remove)(dvp,p,cr)
#define	VOP_LINK(tdvp,fvp,p,cr) (*(tdvp)->v_op->vop_link)(tdvp,fvp,p,cr)
#define	VOP_RENAME(fvp,fnm,tdvp,tnm,cr) \
		(*(fvp)->v_op->vop_rename)(fvp,fnm,tdvp,tnm,cr)
#define	VOP_MKDIR(dp,p,vap,vpp,cr) (*(dp)->v_op->vop_mkdir)(dp,p,vap,vpp,cr)
#define	VOP_RMDIR(dp,p,cdir,cr) (*(dp)->v_op->vop_rmdir)(dp,p,cdir,cr)
#define	VOP_READDIR(vp,uiop,cr,eofp) (*(vp)->v_op->vop_readdir)(vp,uiop,cr,eofp)
#define	VOP_SYMLINK(dvp,lnm,vap,tnm,cr) \
		(*(dvp)->v_op->vop_symlink) (dvp,lnm,vap,tnm,cr)
#define	VOP_READLINK(vp, uiop, cr) (*(vp)->v_op->vop_readlink)(vp, uiop, cr)
#define	VOP_FSYNC(vp, cr) (*(vp)->v_op->vop_fsync)(vp, cr)
#define	VOP_INACTIVE(vp, cr) (*(vp)->v_op->vop_inactive)(vp, cr)
#define	VOP_FID(vp, fidpp) (*(vp)->v_op->vop_fid)(vp, fidpp)
#define	VOP_RWLOCK(vp) (*(vp)->v_op->vop_rwlock)(vp)
#define	VOP_RWUNLOCK(vp) (*(vp)->v_op->vop_rwunlock)(vp)
#define	VOP_SEEK(vp, ooff, noffp) (*(vp)->v_op->vop_seek)(vp, ooff, noffp)
#define	VOP_CMP(vp1, vp2) (*(vp1)->v_op->vop_cmp)(vp1, vp2)
#define	VOP_FRLOCK(vp,cmd,a,f,o,cr) (*(vp)->v_op->vop_frlock)(vp,cmd,a,f,o,cr)
#define	VOP_SPACE(vp,cmd,a,f,o,cr) (*(vp)->v_op->vop_space)(vp,cmd,a,f,o,cr)
#define	VOP_REALVP(vp1, vp2) (*(vp1)->v_op->vop_realvp)(vp1, vp2)
#define	VOP_GETPAGE(vp,of,sz,pr,pl,ps,sg,a,rw,cr)\
		(*(vp)->v_op->vop_getpage) (vp,of,sz,pr,pl,ps,sg,a,rw,cr)
#define	VOP_PUTPAGE(vp,of,sz,fl,cr) (*(vp)->v_op->vop_putpage)(vp,of,sz,fl,cr)
#define VOP_ALLOCSTORE(vp,off,len,cr) \
		(*(vp)->v_op->vop_allocstore) (vp,off,len,cr)
#define	VOP_MAP(vp,of,as,a,sz,p,mp,fl,cr) \
		(*(vp)->v_op->vop_map) (vp,of,as,a,sz,p,mp,fl,cr)
#define	VOP_ADDMAP(vp,of,as,a,sz,p,mp,fl,cr) \
		(*(vp)->v_op->vop_addmap) (vp,of,as,a,sz,p,mp,fl,cr)
#define	VOP_DELMAP(vp,of,as,a,sz,p,mp,fl,cr) \
		(*(vp)->v_op->vop_delmap) (vp,of,as,a,sz,p,mp,fl,cr)
#define	VOP_POLL(vp, events, anyyet, reventsp, phpp) \
		(*(vp)->v_op->vop_poll)(vp, events, anyyet, reventsp, phpp)
#define	VOP_PATHCONF(vp, cmd, valp, cr) \
		(*(vp)->v_op->vop_pathconf)(vp, cmd, valp, cr)
#define	VOP_GETACL(vp,count,dcount,abuf,cr,rvp) \
		(*(vp)->v_op->vop_getacl)(vp,count,dcount,abuf,cr,rvp)
#define	VOP_GETACLCNT(vp,cr,rvp) \
		(*(vp)->v_op->vop_getaclcnt)(vp,cr,rvp)
#define	VOP_SETACL(vp,count,dcount,abuf,cr) \
		(*(vp)->v_op->vop_setacl)(vp,count,dcount,abuf,cr)
#define	VOP_SETLEVEL(vp,level,credp) \
		(*(vp)->v_op->vop_setlevel)(vp,level,credp)
#define	VOP_GETDVSTAT(vp,bufp,cr) \
		(*(vp)->v_op->vop_getdvstat)(vp,bufp,cr)
#define	VOP_SETDVSTAT(vp,bufp,cr) \
		(*(vp)->v_op->vop_setdvstat)(vp,bufp,cr)
#define	VOP_MAKEMLD(dvp,dirname,vap,vpp,credp) \
		(*(dvp)->v_op->vop_makemld)(dvp,dirname,vap,vpp,credp)

/*
 * I/O flags for VOP_READ and VOP_WRITE.
 */
#define IO_APPEND	0x01	/* append write (VOP_WRITE) */
#define IO_SYNC		0x02	/* sync I/O (VOP_WRITE) */

/*
 * Flags for VOP_LOOKUP.
 */
#define LOOKUP_DIR	0x01	/* want parent dir vp */
#define FOLLOW_SYMLINK	0x02	/* follow symbolic links (for rfs) */

/*
 * Vnode attributes.  A bit-mask is supplied as part of the
 * structure to indicate the attributes the caller wants to
 * set (setattr) or extract (getattr).
 */
typedef struct vattr {
	long		va_mask;	/* bit-mask of attributes */
	vtype_t		va_type;	/* vnode type (for create) */
	mode_t		va_mode;	/* file access mode */
	uid_t		va_uid;		/* owner user id */
	gid_t		va_gid;		/* owner group id */
	dev_t		va_fsid;	/* file system id (dev for now) */
	ino_t		va_nodeid;	/* node id */
	nlink_t		va_nlink;	/* number of references to file */
	u_long		va_size0;	/* file size pad (for future use) */
	u_long		va_size;	/* file size in bytes */
	timestruc_t	va_atime;	/* time of last access */
	timestruc_t	va_mtime;	/* time of last modification */
	timestruc_t	va_ctime;	/* time file ``created'' */
	dev_t		va_rdev;	/* device the file represents */
	u_long		va_blksize;	/* fundamental block size */
	u_long		va_nblocks;	/* # of blocks allocated */
	u_long		va_vcode;	/* version code */
	long		va_filler[8];	/* padding */
} vattr_t;

/*
 * Attributes of interest to the caller of setattr or getattr.
 */
#define	AT_TYPE		0x0001
#define	AT_MODE		0x0002
#define	AT_UID		0x0004
#define	AT_GID		0x0008
#define	AT_FSID		0x0010
#define	AT_NODEID	0x0020
#define	AT_NLINK	0x0040
#define	AT_SIZE		0x0080
#define	AT_ATIME	0x0100
#define	AT_MTIME	0x0200
#define	AT_CTIME	0x0400
#define	AT_RDEV		0x0800
#define AT_BLKSIZE	0x1000
#define AT_NBLOCKS	0x2000
#define AT_VCODE	0x4000

#define	AT_ALL	(AT_TYPE|AT_MODE|AT_UID|AT_GID|AT_FSID|AT_NODEID|\
		AT_NLINK|AT_SIZE|AT_ATIME|AT_MTIME|AT_CTIME|\
		AT_RDEV|AT_BLKSIZE|AT_NBLOCKS|AT_VCODE)

#define	AT_STAT	(AT_MODE|AT_UID|AT_GID|AT_FSID|AT_NODEID|AT_NLINK|\
		AT_SIZE|AT_ATIME|AT_MTIME|AT_CTIME|AT_RDEV)

#define	AT_TIMES (AT_ATIME|AT_MTIME|AT_CTIME)

#define	AT_NOSET (AT_NLINK|AT_RDEV|AT_FSID|AT_NODEID|AT_TYPE|\
		 AT_BLKSIZE|AT_NBLOCKS|AT_VCODE)
	
/*
 *  Modes.  Some values same as S_xxx entries from stat.h for convenience.
 */
#define	VSUID		04000		/* set user id on execution */
#define	VSGID		02000		/* set group id on execution */
#define VSVTX		01000		/* save swapped text even after use */

/*
 * Permissions.
 */
#define	VREAD		00400
#define	VWRITE		00200
#define	VEXEC		00100

#define	MODEMASK	07777		/* mode bits plus permission bits */
#define	PERMMASK	00777		/* permission bits */

/*
 * Check whether mandatory file locking is enabled.
 */

/* XENIX Support */
#define MANDLOCK(vp, mode)	\
	((vp)->v_type == VREG && ((((mode) & (VSGID|(VEXEC>>3))) == VSGID) \
	  || ((vp)->v_flag & VXLOCKED) == VXLOCKED))
#if 0 /* XENIX Support else case */
#define MANDLOCK(type, mode)	\
	((type) == VREG && ((mode) & (VSGID|(VEXEC>>3))) == VSGID)
#endif
/* End XENIX Support */

/*
 * Public vnode manipulation functions.
 */
extern int	vn_open();
extern int	vn_create();
extern int	vn_rdwr();
extern int	vn_close();
extern void	vn_rele();
extern int	vn_link();
extern int	vn_rename();
extern int	vn_remove();

#define VN_HOLD(vp) {		\
	(vp)->v_count++;	\
}

#define VN_RELE(vp) {		\
	vn_rele(vp);		\
}

#define VN_INIT(vp, vfsp, type, dev) {	\
	(vp)->v_flag = 0;		\
	(vp)->v_count = 1;		\
	(vp)->v_vfsp = (vfsp);		\
	(vp)->v_type = (type);		\
	(vp)->v_rdev = (dev);		\
	(vp)->v_pages = NULL;		\
	(vp)->v_stream = NULL;		\
}

/*
 * Compare two vnodes for equality.  In general this macro should be used
 * in preference to calling VOP_CMP directly.
 */
#define VN_CMP(VP1,VP2)	((VP1) == (VP2) ? 1 : 	\
	((VP1) && (VP2) && ((VP1)->v_op == (VP2)->v_op) ? VOP_CMP(VP1,VP2) : 0))


/*
 * Macros to synchronize opens and changing of MAC attributes
 */
#define	VN_MACCHGDENY(vp) {					\
	while ((vp)->v_macflag & VMAC_CHANGE) {			\
		(vp)->v_macflag |= VMAC_CHANGEWANT;		\
		(void)sleep((caddr_t)(&(vp)->v_lid), PINOD);	\
	}							\
	(vp)->v_macflag |= VMAC_OPEN;				\
}

#define VN_MACCHGOK(vp) {					\
	(vp)->v_macflag &= ~VMAC_OPEN;				\
}

/*
 * VN_MACCHGON will return EBUSY if the VMAC_OPEN flag is set
 * this will prevent the process that called either lvlfile or
 * devstat from sleeping indefinitely. The VMAC_OPEN is set
 * because a another process is sleeping on open.
 */
#define	VN_MACCHGON(vp) {					\
	while ((vp)->v_macflag & VMAC_CHANGE) {			\
		(vp)->v_macflag |= VMAC_CHANGEWANT;		\
		(void)sleep((caddr_t)(&(vp)->v_lid), PINOD);	\
	}							\
	(vp)->v_macflag |= VMAC_CHANGE;				\
	if ((vp)->v_macflag & VMAC_OPEN) {			\
		(vp)->v_macflag &= ~VMAC_CHANGE;		\
		return EBUSY;					\
	}							\
}

#define VN_MACCHGDONE(vp) {					\
	(vp)->v_macflag &= ~VMAC_CHANGE;			\
	if ((vp)->v_macflag & VMAC_CHANGEWANT) {		\
		(vp)->v_macflag &= ~VMAC_CHANGEWANT;		\
		wakeprocs((caddr_t)(&(vp)->v_lid), PRMPT);	\
	}							\
}

/*
 * This macro determines if a write is actually allowed
 * on the node.  This macro is used to check if a file's
 * access time can be modified.
 */
#define	WRITEALLOWED(vp, cr) \
(((vp)->v_vfsp->vfs_flag & (VFS_RDONLY|VFS_REMOUNT)) == 0 && MAC_VACCESS(vp, VWRITE, cr) == 0)

/*
 * For the benefit of stateless (network) file systems,
 * the credentials passed in to read/write types of VOP
 * operations are those of the process which opened the
 * file.  Until the VOP interface can be revised to
 * support open-time and current credentials (anticipated
 * in some future release), local file systems will
 * bypass the supplied open-time credentials in favor of
 * the current credentials (which are essential for MAC
 * and privilege checks).  An exception is made when the
 * VOP operation is called on behalf of the kernel, in
 * which case the supplied credentials (sys_cred) are used.
 *
 * The following macro is used by local fs dependent code
 * to get the appropriate credentials.
 */
#define	VCURRENTCRED(cr)	((cr) == sys_cred ? (cr) : u.u_cred)

/*
 * Flags for vnode operations.
 */
enum rm		{ RMFILE, RMDIRECTORY };	/* rm or rmdir (remove) */
enum symfollow	{ NO_FOLLOW, FOLLOW };		/* follow symlinks (or not) */
enum vcexcl	{ NONEXCL, EXCL };		/* (non)excl create */
enum create	{ CRCREAT, CRMKNOD, CRMKDIR, CRCORE, CRMKMLD };
						/* reason for create */

typedef enum rm		rm_t;
typedef enum symfollow	symfollow_t;
typedef enum vcexcl	vcexcl_t;
typedef enum create	create_t;

/*
 * Flags to VOP_SETATTR/VOP_GETATTR.
 */
#define	ATTR_UTIME	0x01	/* non-default utime(2) request */
#define	ATTR_EXEC	0x02	/* invocation from exec(2) */
#define	ATTR_COMM	0x04	/* yield common vp attributes */


/*
 * Flags to pass to VOP_ACCESS for security.
 */

#define MAC_ACC		0x01	/* MAC checks */
#define DAC_ACC		0x02	/* DAC checks */
#define MAC_RW		0x04	/* MAC checks for read/write */
#define MAC_IO		0x08	/* MAC checks for ioctl with "read" type cmds */

/*
 * Generally useful macros.
 */
#define	VBSIZE(vp)	((vp)->v_vfsp->vfs_bsize)
#define	NULLVP		((struct vnode *)0)
#define	NULLVPP		((struct vnode **)0)

#ifdef _KERNEL

extern int lookupname();
extern int mld_deflect();

extern struct vnode *rootdir;	/* pointer to vnode of root directory */

#endif

#endif	/* _FS_VNODE_H */
