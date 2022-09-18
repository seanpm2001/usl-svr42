/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _FS_SFS_SFS_QUOTA_H	/* wrapper symbol for kernel use */
#define _FS_SFS_SFS_QUOTA_H	/* subject to change without notice */

#ident	"@(#)uts-comm:fs/sfs/sfs_quota.h	1.3.3.3"
#ident	"$Header: $"

#ifdef _KERNEL_HEADERS

#ifndef _UTIL_TYPES_H
#include <util/types.h>	/* REQUIRED */
#endif

#elif defined(_KERNEL)

#include <sys/types.h>	/* REQUIRED */

#endif /* _KERNEL_HEADERS */

/*
 * Various junk to do with various quotas (etc) imposed upon
 * the average user (big brother finally hits UNIX).
 */

/*
 * The following constants define the default amount of time given a user
 * before the soft limits are treated as hard limits (usually resulting
 * in an allocation failure). These may be  modified by the quotactl
 * system call with the Q_SETQLIM or Q_SETQUOTA commands.
 */

#define DQ_FTIMELIMIT   (7 * 24*60*60)          /* 1 week */
#define DQ_BTIMELIMIT   (7 * 24*60*60)          /* 1 week */

/*
 * The dqblk structure defines the format of the disk quota file
 * (as it appears on disk) - the file is an array of these structures
 * indexed by user number.  The setquota sys call establishes the inode
 * for each quota file (a pointer is retained in the mount structure).
 */

struct  dqblk {
        u_long  dqb_bhardlimit; /* absolute limit on disk blks alloc */
        u_long  dqb_bsoftlimit; /* preferred limit on disk blks */
        u_long  dqb_curblocks;  /* current block count */
        u_long  dqb_fhardlimit; /* maximum # allocated files + 1 */
        u_long  dqb_fsoftlimit; /* preferred file limit */
        u_long  dqb_curfiles;   /* current # allocated files */
        u_long  dqb_btimelimit; /* time limit for excessive disk use */
        u_long  dqb_ftimelimit; /* time limit for excessive files */
};

#define dqoff(UID)      ((off_t)((UID) * sizeof(struct dqblk)))

/*
 * The dquot structure records disk usage for a user on a filesystem.
 * There is one allocated for each quota that exists on any filesystem
 * for the current user. A cache is kept of recently used entries.
 * Active inodes have a pointer to the dquot associated with them.
 */
struct  dquot {
        struct  dquot *dq_forw, *dq_back;/* hash list, MUST be first entry */
        struct  dquot *dq_freef, *dq_freeb; /* free list */
        short   dq_flags;
#define DQ_LOCKED       0x01            /* locked for I/O */
#define DQ_WANT         0x02            /* wanted */
#define DQ_MOD          0x04            /* this quota modified since read */
#define DQ_BLKS         0x10            /* has been warned about blk limit */
#define DQ_FILES        0x20            /* has been warned about file limit */
        short   dq_cnt;                 /* count of active references */
        uid_t   dq_uid;                 /* user this applies to */
        struct sfs_vfs *dq_sfs_vfsp;     /* filesystem this relates to */
        struct dqblk dq_dqb;            /* actual usage & quotas */
};

#define dq_bhardlimit   dq_dqb.dqb_bhardlimit
#define dq_bsoftlimit   dq_dqb.dqb_bsoftlimit
#define dq_curblocks    dq_dqb.dqb_curblocks
#define dq_fhardlimit   dq_dqb.dqb_fhardlimit
#define dq_fsoftlimit   dq_dqb.dqb_fsoftlimit
#define dq_curfiles     dq_dqb.dqb_curfiles
#define dq_btimelimit   dq_dqb.dqb_btimelimit
#define dq_ftimelimit   dq_dqb.dqb_ftimelimit

/*
 * Flags for m_qflags in mount struct.
 */
#define MQ_ENABLED      0x01            /* quotas are enabled */

#if defined(_KERNEL) && defined(QUOTA)
extern struct dquot *sfs_dquot;		/* start of disk quota table */
extern struct dquot *sfs_dquotNDQUOT;	/* end of disk quota table */
extern int           sfs_ndquot;	/* size of quota table, from sfs.cf */

extern void sfs_qtinit();               /* initialize quota system */
extern struct dquot *sfs_getinoquota(); /* establish quota for an inode */
extern int sfs_chkdq();                 /* check disk block usage */
extern int sfs_chkiq();                 /* check inode usage */
extern void sfs_dqrele();               /* release dquot */
extern int sfs_closedq();               /* close quotas */

extern int sfs_getdiskquota();          /* get dquot for uid on filesystem */
extern void sfs_dqput();                /* release locked dquot */
extern void sfs_dqupdate();             /* update dquot on disk */

#define DQLOCK(dqp) {					\
        while ((dqp)->dq_flags & DQ_LOCKED) {		\
                (dqp)->dq_flags |= DQ_WANT;		\
                (void) sleep((caddr_t)(dqp), PINOD+1);	\
        }						\
        (dqp)->dq_flags |= DQ_LOCKED;			\
}

#define DQUNLOCK(dqp) {					\
        (dqp)->dq_flags &= ~DQ_LOCKED;			\
        if ((dqp)->dq_flags & DQ_WANT) {		\
                (dqp)->dq_flags &= ~DQ_WANT;		\
                wakeprocs((caddr_t)(dqp), PRMPT);	\
        }						\
}
#endif

/*
 * Definitions for the 'quotactl' system call.
 */
#define Q_QUOTAON       1       /* turn quotas on */
#define Q_QUOTAOFF      2       /* turn quotas off */
#define Q_SETQUOTA      3       /* set disk limits & usage */
#define Q_GETQUOTA      4       /* get disk limits & usage */
#define Q_SETQLIM       5       /* set disk limits only */
#define Q_SYNC          6       /* update disk copy of quota usages */
#define Q_ALLSYNC       7       /* update disk copy of quota usages for all fs*/

struct quotctl {
        int     op;
        uid_t   uid;
        caddr_t addr;
};
#define Q_QUOTACTL	0x00030189	/* ioctl command for quotactl */
#endif /* _FS_SFS_SFS_QUOTA_H */