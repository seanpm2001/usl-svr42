/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-x86at:io/target/sw01.c	1.16"
#ident	"$Header: $"

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*      INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied under the terms of a license agreement   */
/*	or nondisclosure agreement with Intel Corporation and may not be   */
/*	copied or disclosed except in accordance with the terms of that    */
/*	agreement.							   */

/*	Copyright (c) 1989 TOSHIBA CORPORATION		*/
/*	All Rights Reserved	*/

/*	Copyright (c) 1989 SORD COMPUTER CORPORATION	*/
/*	All Rights Reserved	*/


/*
**	SCSI WORM Target Driver.
*/

#include <svc/errno.h>
#include <util/types.h>
#include <util/param.h>
#include <util/sysmacros.h>
#include <proc/signal.h>
#include <proc/user.h>
#include <util/cmn_err.h>
#include <fs/buf.h>
#include <mem/kmem.h>
#include <svc/systm.h>
#include <io/open.h>
#include <io/ioctl.h>
#include <util/debug.h>
#include <io/conf.h>
#include <io/uio.h>
#include <io/ddi.h>
#include <io/target/scsi.h>
#include <io/target/sdi_edt.h>
#include <io/target/dynstructs.h>
#include <io/target/sdi.h>
#include <io/target/worm.h>
#include <io/target/sw01.h>
#include <util/mod/moddefs.h>

#define SLEEP           !NOSLEEP
#define DRVNAME         "sw01 - WORM target driver"

STATIC  int     sw01_load(), sw01_unload();
void	sw01rinit(); 
MOD_DRV_WRAPPER(sw01, sw01_load, sw01_unload, NULL, DRVNAME);

/* The following symbols should be defined in scsi.h */
#define	SS_VERIFY	0x002f
#define	SS_READBUF	0x003c
#define	SS_STNCHECK	0x00ce
#define	SS_LOADCA	0x00cf
#define	SS_READCB	0x00d2
#define	SS_CHECK	0x00e4

/* Allocated in space.c */
extern 	struct tc_data	Sw01_data[];	/* Array of TC dev info	    */
extern	unsigned 	Sw01_datasz;	/* Number of supported TCs  */
extern	long	 	Sw01_bmajor;	/* Block major number	    */
extern	long	 	Sw01_cmajor;	/* Character major number   */
extern	int	 	Sw01_jobs;	/* Allocation per LU        */
extern	struct head	lg_poolhead;	/* head struct for dyn. allocs */

static	int 		sw01_wormcnt;	/* Num of worms configured  */
static	struct tc_edt  *sw01_edt;	/* Array of EDT structures  */
static	struct worm    *sw01_worm;	/* Array of worm structures */
static	unsigned char	sw01_tmp[1028];	/* Temporary buffer	    */
static	int		sw01_tmp_flag;	/* sw01_tmp control flag    */
static  struct owner   *ownerlist;	/* list of owner structs    */
int	sw01devflag	= D_NEW | D_DMA;	/* 4.0 style driver flag */

#define	ARG	a0,a1,a2,a3,a4,a5

static  int     mod_dynamic = 0;
static  size_t  mod_memsz = 0;
static  int     rinit_flag = 0;

STATIC  int
sw01_load()
{
        mod_dynamic = 1;
        sw01init();
        return(0);
}

STATIC  int
sw01_unload()
{
	sdi_clrconfig(ownerlist, SDI_DISCLAIM|SDI_REMOVE, sw01rinit);

        if(mod_memsz > 0)       {
		ownerlist = NULL;
                kmem_free((caddr_t)sw01_worm, mod_memsz);
		sw01_worm = NULL;
        }
        return(0);
}

int	sw01sendt();
void	sw01intn();
void	sw01intf();
void	sw01intrq();
void	sw01strategy();

#ifdef DEBUG
#define SIZE    10
daddr_t	sw01_Offset = 0;
static char     sw01_Debug[SIZE] = {1,1,1,1,0,0,0,0,0,0};
#define DPR(l)          if (sw01_Debug[l]) cmn_err
#endif


/*
** Function name: sw01init()
** Description:
**	Called by kernel to perform driver initialization.
**	This function does not access any devices.
*/

sw01init()
{
	register struct worm  *wmp;	/* worm pointer	 */
	register struct job    *jp;	/* Job struct pointer	 */
	struct	owner	*op;
	struct drv_majors drv_maj;
	caddr_t	 base;			/* Base memory pointer	 */
	int  wormsz,			/* worm size (in bytes) */
	     jobsz,			/* Job size (in bytes)	 */
	     i,				/* loop index - level 1	 */
	     j,				/* loop index - level 2	 */
	     lu,			/* LU number		 */
	     tc;			/* TC number		 */


#ifdef	DEBUG
	DPR (1)(CE_CONT, "SW01 DEBUG DRIVER INSTALLED\n");
#endif
	/* Check if SDI has been started */
	sw01_wormcnt = 0;

	drv_maj.b_maj = Sw01_bmajor;
	drv_maj.c_maj = Sw01_cmajor;
	ownerlist = sdi_doconfig(SW01_dev_cfg, SW01_dev_cfg_size,
			"SW01 Worm Driver", &drv_maj, sw01rinit);

	sw01_wormcnt = 0;
	for (op = ownerlist; op; op = (struct owner *)op->res1) {
		sw01_wormcnt++;
	}

#ifdef DEBUG
	printf("%d worms claimed\n", sw01_wormcnt);
#endif

#ifdef	NEEDS_INTEL_BUGFIX
	/*
	 * The SCSI controller of the WM-D070 or WM-C050 is able to connect
	 * more 7 logical units.
	 * But, when you send 'INQUIRY' command to unconnected logical unit,
	 * it will not return 'Check Condition' but 'Good'.
	 * We don't apply this spec to sdi_config()
	 * Therefore, we only support 1 logical unit per 1 controller.
	 * Normally, it is no problem because logical unit is 1.
	 * Some future day, this bug is going to be fixed.
	 */
	{
		register int	l;	/* Logical Unit */

		for (tc = 0, edt_p = sw01_edt; tc < tc_cnt; tc++, edt_p++) 
		{
			for (l = 0; l < MAX_LUS; l++)
			{
				if ((edt_p->lu_id[l]) && (l >= 1))
				{
					edt_p->lu_id[l] = 0;
					edt_p->n_lus--;
				}
			}
		}
	}
#endif

	/* Check if there are devices configured */
	if (sw01_wormcnt == 0) {
		sw01_wormcnt = WMNOTCS;
		return;
	}

	/*
	 * Allocate the worm and job structures
	 */
	wormsz = sw01_wormcnt * sizeof(struct worm);
	mod_memsz = wormsz;
        if ((base = (caddr_t)kmem_alloc(mod_memsz,
			 (mod_dynamic ? KM_SLEEP : KM_NOSLEEP))) == 0)
	{
		cmn_err(CE_WARN,
			"WORM Error: Insufficient memory to configure driver");
		cmn_err(CE_CONT, "!Unable to allocate 0x%x bytes of memory\n");
		sw01_wormcnt = WMNOTCS;
		mod_memsz = 0;
		return;
	}

	sw01_worm = (struct worm *) base;

	/*
	 * Initialize the worm structures
	 */
	wmp = sw01_worm;
	for(tc = 0, op = ownerlist; tc < sw01_wormcnt;
			tc++, op=(struct owner *)op->res1, wmp++) {

		/* Initialize the queue ptrs */
		wmp->wm_first = (struct job *) wmp;
		wmp->wm_last  = (struct job *) wmp;
		wmp->wm_next  = (struct job *) wmp;
		wmp->wm_batch = (struct job *) wmp;

		/* Initialize state & counters */
		wmp->wm_state  = 0;
		wmp->wm_count  = 0;
		wmp->wm_npend  = 0;
		wmp->wm_fltcnt = 0;

		/* Initialize the SCSI address */
		wmp->wm_addr.sa_exlun = 0;
#ifdef DEBUG
	printf("op 0x%x ", op);
	printf("edt 0x%x ", op->edtp);
	printf("hba %d scsi id %d lun %d\n",op->edtp->hba_no,op->edtp->scsi_id,op->edtp->lun);
#endif
		wmp->wm_addr.sa_lun    = op->edtp->lun;
	    	wmp->wm_addr.sa_fill   = (op->edtp->hba_no << 3) |
						(op->edtp->scsi_id);
		wmp->wm_spec	    = sdi_findspec(op->edtp, sw01_dev_spec);
	}
}

/*
** Function name: sw01rinit()
** Description:
**	Called by sdi to perform driver initialization of additional
**	devices found after the dynamic load of HBA drivers. This 
**	routine is called only when sw01 is a static driver.
**	This function does not access any devices.
*/
void
sw01rinit()
{
	struct worm  *wmp, *owmp;	/* worm pointer	 */
	struct	owner	*ohp, *op;
	struct drv_majors drv_maj;
	caddr_t	 base;			/* Base memory pointer	 */
	int  wormsz,			/* worm size (in bytes) */
	     new_wormcnt,		/* number of additional devs found*/
	     owormcnt,			/* number of devs previously found*/
	     tmpcnt,			/* temp count of devs */
	     found,			/* search flag */
	     prevpl,			/* prev process level for splx */
	     wormcnt;			/* worm instance	*/

	/* set rinit_flag to prevent access to sw01_worm while sw01rinit */
	/* is running to prevent interim access				 */
	rinit_flag = 1;
	new_wormcnt= 0;
	drv_maj.b_maj = Sw01_bmajor;
	drv_maj.c_maj = Sw01_cmajor;
	/* call sdi_doconfig with NULL func so we don't get called again */
	ohp = sdi_doconfig(SW01_dev_cfg, SW01_dev_cfg_size,
				"SW01 WORM Driver", &drv_maj, NULL);

	for (op = ohp; op; op = (struct owner *)op->res1) {
		new_wormcnt++;
	}
#ifdef DEBUG
	printf("sw01rinit %d worms claimed\n", new_wormcnt);
#endif
	/* Check if there are additional devices configured */
	if ((new_wormcnt == sw01_wormcnt) || (new_wormcnt == 0)) {
		rinit_flag = 0;
		wakeup((caddr_t)&rinit_flag);
		return;
	}
	/*
	 * Allocate the worm and job structures
	 */
	wormsz = new_wormcnt * sizeof(struct worm);
        if ((base = kmem_alloc(wormsz, KM_NOSLEEP)) == NULL) {
		cmn_err(CE_WARN,
			"WORM Error: Insufficient memory to configure driver");
		cmn_err(CE_CONT,
			"!Could not allocate 0x%x bytes of memory\n",wormsz);
		rinit_flag = 0;
		wakeup((caddr_t)&rinit_flag);
		return;
	}
	/*
	 * Initialize the worm structures
	 */
	if(sw01_wormcnt == WMNOTCS)
		owormcnt = 0;
	else
		owormcnt = sw01_wormcnt;
	found = 0;
	prevpl = spl5();
	for(wmp = (struct worm *)base, wormcnt = 0, op = ohp; 
	    wormcnt < new_wormcnt;
	    wormcnt++, op=(struct owner *)op->res1, wmp++) {

		/* Initialize new worm structs by copying existing worm */
		/* structs into new struct and initializing new instances */
		if (owormcnt) {
			for (owmp = sw01_worm, tmpcnt = 0; 
			     tmpcnt < sw01_wormcnt; owmp++,tmpcnt++) {
				if ((owmp->wm_addr.sa_lun == op->edtp->lun) &&
	    			   (owmp->wm_addr.sa_fill == 
				   ((op->edtp->hba_no << 3)|(op->edtp->scsi_id)))){
					found = 1;
					break;
				}
			}
			if (found) { /* copy owmp to wmp */
				if (owmp->wm_next == (struct job *)owmp) {
					wmp->wm_first = (struct job *)wmp;
					wmp->wm_last  = (struct job *)wmp;
					wmp->wm_next  = (struct job *)wmp;
					wmp->wm_batch = (struct job *)wmp;
				}
				else {
					wmp->wm_first = owmp->wm_first;
					wmp->wm_last  = owmp->wm_last;
					wmp->wm_last->j_next = (struct job *)wmp;
					wmp->wm_next  = owmp->wm_next;
					wmp->wm_next->j_prev = (struct job *)wmp;
					if (owmp->wm_batch == (struct job *)owmp) {
						wmp->wm_batch = (struct job *)wmp;
				 	}
					else
						wmp->wm_batch = owmp->wm_batch;
				}
				wmp->wm_addr.sa_exlun = owmp->wm_addr.sa_exlun;
				wmp->wm_addr.sa_lun = owmp->wm_addr.sa_lun;
	    			wmp->wm_addr.sa_fill = owmp->wm_addr.sa_fill;
				wmp->wm_sendid = owmp->wm_sendid;
				wmp->wm_state  = owmp->wm_state;
				wmp->wm_count  = owmp->wm_count;
				wmp->wm_npend  = owmp->wm_npend;
				wmp->wm_fltcnt = owmp->wm_fltcnt;
				wmp->wm_fltjob = owmp->wm_fltjob;
				wmp->wm_fltreq = owmp->wm_fltreq;
				wmp->wm_fltres = owmp->wm_fltres;
				wmp->wm_fltcmd = owmp->wm_fltcmd;
				wmp->wm_sense  = owmp->wm_sense;
				wmp->wm_capacity = owmp->wm_capacity;
				wmp->wm_spec = owmp->wm_spec;
				wmp->wm_blkshft = owmp->wm_blkshft;
				found = 0;
				owormcnt--;
				continue;
			}
		}
		/* Its a new worm device so init worm struct */
		wmp->wm_first = (struct job *) wmp;
		wmp->wm_last  = (struct job *) wmp;
		wmp->wm_next  = (struct job *) wmp;
		wmp->wm_batch = (struct job *) wmp;
		wmp->wm_state  = 0;
		wmp->wm_count  = 0;
		wmp->wm_npend  = 0;
		wmp->wm_fltcnt = 0;
		wmp->wm_addr.sa_exlun = 0;
#ifdef DEBUG
	printf("worm: op 0x%x edt 0x%x hba %d ",op,op->edtp,op->edtp->hba_no);
	printf("scsi id %d lun %d\n",op->edtp->scsi_id,op->edtp->lun);
#endif
		wmp->wm_addr.sa_lun = op->edtp->lun;
	    	wmp->wm_addr.sa_fill = (op->edtp->hba_no << 3) | (op->edtp->scsi_id);
		wmp->wm_spec = sdi_findspec(op->edtp, sw01_dev_spec);
	}
	if (sw01_wormcnt > 0)
		kmem_free(sw01_worm, mod_memsz);
	sw01_wormcnt = new_wormcnt;
	sw01_worm = (struct worm *)base;
	mod_memsz = wormsz;
	ownerlist = ohp;
	splx(prevpl);
	rinit_flag = 0;
	wakeup((caddr_t)&rinit_flag);
}

/*
** Function name: sw01getjob()
** Description:
**	This function will allocate a worm job structure from the free
**	list.  The function will sleep if there are no jobs available.
**	It will then get a SCSI block from SDI.
*/

struct job *
sw01getjob()
{
	register struct job *jp;

#ifdef	DEBUG
	DPR (4)(CE_CONT, "sw01: getjob\n");
#endif
	jp = (struct job *)sdi_get(&lg_poolhead, 0);

	/* Get an SB for this job */
	jp->j_sb = sdi_getblk();
	return(jp);
}
	

/*
** Function name: sw01freejob()
** Description:
**	This function returns a job structure to the free list. The
**	SCSI block associated with the job is returned to SDI.
*/

sw01freejob(jp)
register struct job *jp;
{
	register int s;

#ifdef	DEBUG
	DPR (4)(CE_CONT, "sw01: freejob\n");
#endif
	sdi_freeblk(jp->j_sb);
	sdi_free(&lg_poolhead, (jpool_t *)jp);
}


/*
** Function name: sw01send()
** Description:
**	This function sends jobs from the work queue to the host adapter.
**	It will send as many jobs as available or the number required to
**	keep the logical unit busy.  If the job cannot be accepted by SDI 
**	the function will reschedule itself via the timeout mechanizism.
*/

sw01send(wmp)
register struct worm *wmp;
{
	register struct job *jp;
	register int rval;

#ifdef	DEBUG
	DPR (1)(CE_CONT, "sw01: sw01send\n");
#endif
	while (wmp->wm_npend < MAXPEND &&  wmp->wm_next != (struct job *)wmp)
	{
		jp = wmp->wm_next;
		wmp->wm_next = jp->j_next;

		if (jp == wmp->wm_batch) {
			/* Start new batch */
			wmp->wm_batch = (struct job *)wmp;
			wmp->wm_state ^= WM_DIR;
		}

		if ((rval = sdi_send(jp->j_sb)) != SDI_RET_OK)
		{
			if (rval == SDI_RET_RETRY) {
#ifdef DEBUG
				cmn_err(CE_NOTE,
					"WORM Error: SDI currently busy - Will retry later");
#endif
				/* Reset next position */
				wmp->wm_next = jp;

				if (wmp->wm_state & WM_SEND)
					return;

				/* Call back later */
				wmp->wm_state |= WM_SEND;
				wmp->wm_sendid = timeout((void(*)())sw01sendt, (caddr_t)wmp, LATER);
				return;
			} else {
				cmn_err(CE_WARN,
					"WORM Error: Invalid SDI request - Return value = 0x%x", rval);
				wmp->wm_npend++;
				sw01comp(jp);
				continue;
			}
		}

		wmp->wm_npend++;
	}

	if (wmp->wm_state & WM_SEND) {
		wmp->wm_state &= ~WM_SEND;
		untimeout(wmp->wm_sendid);
	}
}


/*
** Function name: sw01sendt()
** Description:
**	This function calls sw01send() after the record of the pending
**	timeout is erased.
*/

sw01sendt(wmp)
register struct worm *wmp;
{
	wmp->wm_state &= ~WM_SEND;
	sw01send(wmp);
}


/*
** Function name: sw01open()
** Description:
** 	Driver open() entry point.  Determines the type of open being
**	being requested.  On the first open to a device, the PD and
**	VTOC information is read. 
*/

sw01open(devp, flag, otyp, cred_p)
dev_t	*devp;
int	flag;
int	otyp;
struct cred	*cred_p;
{
	register struct worm	*wmp;
	unsigned		unit;
	dev_t			dev = *devp;

	/* sleep while sw01rinit is running so sw01_worm is valid */
	while (rinit_flag) {
		sleep((caddr_t)rinit_flag, PRIBIO+1);
	}

#ifdef	DEBUG
	if (getminor(dev) & 0x08) {		/* Debug off */
		int	i;
		for (i = 0; i < SIZE; i++)
			sw01_Debug[i] = 0;
	} else if (getminor(dev) & 0x10) {		/* Debug on */
		int	i;
		/* Debug on */
		for (i = 0; i < SIZE; i++)
			sw01_Debug[i] = 1;
	}
	DPR (1)(CE_CONT, "sw01open: (dev=0x%x flags=0x%x otype=0x%x)\n",
			dev, flag, otyp);
#endif

	unit = UNIT(dev);

	/* Check for non-existent device */
	if ((int)unit >= sw01_wormcnt) {
		return(ENXIO);
	}

	wmp = &sw01_worm[unit];

	/* Sleep if someone else already opening */
	while (wmp->wm_state & WM_WOPEN)
		sleep((caddr_t)&wmp->wm_state, PRIBIO);

	/* Lock out other attempts */
	wmp->wm_state |= WM_WOPEN;

	if (!(wmp->wm_state & WM_INIT))
	{
		wmp->wm_fltreq = sdi_getblk();  /* Request sense */
		wmp->wm_fltres = sdi_getblk();  /* Resume */

		wmp->wm_fltres->SFB.sf_dev    = wmp->wm_addr;

		wmp->wm_fltreq->sb_type = ISCB_TYPE;
		wmp->wm_fltreq->SCB.sc_datapt = SENSE_AD(&wmp->wm_sense);
		wmp->wm_fltreq->SCB.sc_datasz = SENSE_SZ;
		wmp->wm_fltreq->SCB.sc_mode   = SCB_READ;
		wmp->wm_fltreq->SCB.sc_cmdpt  = SCS_AD(&wmp->wm_fltcmd);
		wmp->wm_fltreq->SCB.sc_dev    = wmp->wm_addr;
		sdi_translate(wmp->wm_fltreq, B_READ, 0);
		wmp->wm_addr.sa_major = getmajor(dev);
		wmp->wm_addr.sa_minor = getminor(dev);
		wmp->wm_state |= WM_INIT;
	}

	if (wmp->wm_spec && wmp->wm_spec->first_open) {
		(*wmp->wm_spec->first_open)(wmp);
	}

	if(sw01ioctl(dev,W_TESTUNIT,0,0))
		sw01ioctl(dev,W_STRTUNIT,0,0);

	wmp->wm_state &= ~WM_WOPEN;
	wakeup((caddr_t)&wmp->wm_state);

	return(0);
}


/*
** Function name: sw01close()
** Description:
** 	Driver close() entry point.  Determine the type of close
**	being requested.
*/

sw01close(dev, flag, otyp, cred_p)
register dev_t	dev;
int		flag;
int		 otyp;
struct cred	*cred_p;
{
	register struct worm	*wmp;
	unsigned		unit;

	/* sleep while sw01rinit is running so sw01_worm is valid */
	while (rinit_flag) {
		sleep((caddr_t)rinit_flag, PRIBIO+1);
	}

#ifndef	BUG
	/*
	 * Normally, if prefixopen() is failed, prefixclose() isn't called.
	 * But prefixclose() is called from inner kernel in UNIX R4.0 B9.
	 */

	unit = UNIT(dev);
	/* Check for non-existent device */
	if ((int)unit >= sw01_wormcnt) {
#ifdef	DEBUG
		DPR (3)(CE_CONT, "sw01close: close error\n");
#endif
		return(ENXIO);
	}
	wmp = &sw01_worm[unit];
#else
	wmp = &sw01_worm[UNIT(dev)];
#endif
	if(!sw01ioctl(dev,W_TESTUNIT,0,0))
		sw01ioctl(dev,W_STOPUNIT,0,0);

	if (wmp->wm_spec && wmp->wm_spec->last_close) {
		(*wmp->wm_spec->last_close)(wmp);
	}

	wmp->wm_state &= ~WM_PARMS; 

	return(0);
}


/*
** Function name: sw01strategy()
** Description:
** 	Driver strategy() entry point.  Initiate I/O to the device.
**	The buffer pointer passed from the kernel contains all the
**	necessary information to perform the job.  This function only
**	checks the validity of the request.  Most of the work is done
**	by sw01io().
*/
void
sw01strategy(bp)
register struct buf	*bp;
{
	register struct worm	*wmp;
	register int		lastsect;	/* Last sector in device */
	register int		sectlen;	/* Sector Length */
	unsigned 		sectcnt;	/* Sector count of request */
	uint_t	 		blksects;	/* Blocks per Sector */

	/* sleep while sw01rinit is running so sw01_worm is valid */
	while (rinit_flag) {
		sleep((caddr_t)rinit_flag, PRIBIO+1);
	}

	wmp = &sw01_worm[UNIT(bp->b_dev)];

#ifdef	DEBUG
	DPR (1)(CE_CONT, "sw01: sw01strategy\n");
#endif

	if (((wmp->wm_state & WM_PARMS) == 0) && sw01config(wmp)) {
		bp->b_flags |= B_ERROR;
		bp->b_error = ENXIO;
		biodone(bp);
		return;
	}

	lastsect = wmp->wm_capacity.wm_addr;
	sectlen = wmp->wm_capacity.wm_len;
	sectcnt = (bp->b_bcount + sectlen - 1) / sectlen;

	/* 
	 * If this is a partial-sector transfer, then special
	 * handling is required.
	 */
	blksects = (1 << wmp->wm_blkshft);
	if (((bp->b_blkno & (blksects-1)) != 0) ||
	    ((bp->b_bcount & (sectlen-1)) != 0)) {
		sdi_blkio(bp, wmp->wm_blkshft, sw01strategy);
		return;
	}

	bp->b_sector = bp->b_blkno / (sectlen / BLKSIZE);
	bp->b_resid = 0;

#ifdef	DEBUG
	DPR (3)(CE_CONT, "Block 0x%x -> Sector 0x%x\n",
		bp->b_blkno, bp->b_sector);

	bp->b_sector += sw01_Offset;
	DPR (3)(CE_CONT, "Offset 0x%x -> Sector 0x%x\n",
		sw01_Offset, bp->b_sector);
#endif

	/*
	 * Check if request fits in partition
	 */
	if (bp->b_sector + sectcnt > lastsect) {
		if (bp->b_sector > lastsect) {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
			biodone(bp);
			return;
		}
		bp->b_resid = bp->b_bcount -
			((lastsect - bp->b_sector) * sectlen);
		if (bp->b_bcount == bp->b_resid) {
			biodone(bp);
			return;
		}
		bp->b_bcount -= bp->b_resid;
	}

	sw01io(wmp, bp);
}


/*
** Function name: sw01io()
** Description:
**	This function creates a SCSI I/O request from the information in
**	the worm structure and the buffer header.  The request is queued
**	according to an elevator algorithm.
*/

sw01io(wmp, bp)
register struct worm	*wmp;
register buf_t		*bp;
{
	register struct job	*jp;
	register struct scb	*scb;
	register struct scm	*cmd;
	register int		sectlen;
	unsigned		sectcnt;
	int			s;

#ifdef	DEBUG
	DPR (1)(CE_CONT, "sw01: sw01io\n");
#endif
	jp = sw01getjob();
	jp->j_bp = bp;
	jp->j_wmp = wmp;
	jp->j_errcnt = 0;

	jp->j_sb->sb_type = SCB_TYPE;

	/*
	 * Fill in the scb for this job.
	 */
	scb = &jp->j_sb->SCB;
	scb->sc_cmdpt = SCM_AD(&jp->j_cmd);
	scb->sc_cmdsz = SCM_SZ;
	scb->sc_datapt = bp->b_un.b_addr;
	scb->sc_datasz = bp->b_bcount;
	scb->sc_link = NULL;
	scb->sc_mode = (bp->b_flags & B_READ) ? SCB_READ : SCB_WRITE;
	scb->sc_dev = wmp->wm_addr;

	sdi_translate(jp->j_sb, bp->b_flags, bp->b_proc);

	scb->sc_int = sw01intn;
	scb->sc_time = JTIME;
	scb->sc_wd = (long) jp;

	/*
	 * Fill in the command for this job.
	 */
	cmd = &jp->j_cmd.sm;
	cmd->sm_op = (bp->b_flags & B_READ) ? SM_READ : SM_WRITE;
	cmd->sm_lun = wmp->wm_addr.sa_lun;
	cmd->sm_res1 = 0;
	cmd->sm_res2 = 0;
	cmd->sm_cont = 0;

	jp->j_addr = bp->b_sector;

	sectlen = wmp->wm_capacity.wm_len;
	sectcnt = (bp->b_bcount + sectlen - 1) / sectlen;

	cmd->sm_len  = sdi_swap16(sectcnt);
	cmd->sm_addr = sdi_swap32(jp->j_addr);

#ifdef	DEBUG
	DPR (3)(CE_CONT, "Sector = 0x%x,   Count = 0x%x\n",
		bp->b_sector, sectcnt);
#endif
	/* Is this a partial block? */
	if ((scb->sc_resid = (cmd->sm_len * sectlen) - bp->b_bcount) != 0)
		scb->sc_mode |= SCB_PARTBLK;

	drv_getparm(LBOLT, (ulong *)&bp->b_start);

	/*
 	 * Put the job onto the drive worklist using
 	 * an elevator algorithm.
 	 */
	s = spl5();
	wmp->wm_count++;
        if (wmp->wm_next == (struct job *) wmp) {
                jp->j_next = (struct job *) wmp;
                jp->j_prev = wmp->wm_last;
                wmp->wm_last->j_next = jp;
		wmp->wm_last = jp;
		wmp->wm_next = jp;
	} else {
		register struct job *jp1 = wmp->wm_batch;

                if (wmp->wm_state & WM_DIR) { 
                        while (jp1 != (struct job *) wmp) {
				if (jp1->j_addr < jp->j_addr)
					break;
                                jp1 = jp1->j_next;
			}
		} else {
                        while (jp1 != (struct job *) wmp) {
				if (jp1->j_addr > jp->j_addr)
					break;
                                jp1 = jp1->j_next;
			}
		}

                jp->j_next = jp1;
                jp->j_prev = jp1->j_prev;
                jp1->j_prev->j_next = jp;
                jp1->j_prev = jp;

                if (jp1 == wmp->wm_batch)
                        wmp->wm_batch = jp;
                if (jp1 == wmp->wm_next)
                        wmp->wm_next = jp;
	}

	sw01send(wmp);
	splx(s);
}


/*
** Function name: sw01breakup()
** Description:
** 	Driver raw I/O entry point.  Performs a raw transfer using the
**	user's locked down buffer.  The function calls sw01strategy()
**	with contiguous DMA-able pieces of the request.
*/
sw01breakup(bp)
struct buf *bp;
{
dma_pageio(sw01strategy, bp);
}


/*
** Function name: sw01read()
** Description:
** 	Driver read() entry point.  Performs a "raw" read.
*/

sw01read(dev, uio_p, cred_p)
dev_t dev;
struct uio	*uio_p;
struct cred	*cred_p;
{
	register struct worm	*wmp;

	/* sleep while sw01rinit is running so sw01_worm is valid */
	while (rinit_flag) {
		sleep((caddr_t)rinit_flag, PRIBIO+1);
	}

	wmp = &sw01_worm[UNIT(dev)];

	if (((wmp->wm_state & WM_PARMS) == 0) && sw01config(wmp)) {
		return(ENXIO);
	}

	return(uiophysio(sw01breakup, NULL, dev, B_READ, uio_p));
}


/*
** Function name: sw01write()
** Description:
** 	Driver write() entry point.  Performs a "raw" write.
*/

sw01write(dev, uio_p, cred_p)
dev_t	dev;
struct uio	*uio_p;
struct cred	*cred_p;
{
	register struct worm	*wmp;

	/* sleep while sw01rinit is running so sw01_worm is valid */
	while (rinit_flag) {
		sleep((caddr_t)rinit_flag, PRIBIO+1);
	}

	wmp = &sw01_worm[UNIT(dev)];

	if (((wmp->wm_state & WM_PARMS) == 0) && sw01config(wmp)) {
		return(ENXIO);
	}

	return(uiophysio(sw01breakup, NULL, dev, B_WRITE, uio_p));
}

/*
** Function name: sw01print()
** Description:
** 	Driver print() entry point.  Prints an error message on
**	the system console.
*/

sw01print(dev, str)
dev_t	dev;
char	*str;
{
	register struct worm	*wmp;
	char	name[NAMESZ];

	wmp = &sw01_worm[UNIT(dev)];

	sdi_name(&wmp->wm_addr, name);
	cmn_err(CE_WARN, "WORM Error: %s: Logical Unit %d - %s",
		name, wmp->wm_addr.sa_lun, str);
}

/*
** Function name: sw01size()
** Description:
**	Driver size() entry point.  Return the device size.
*/
sw01size(dev)
dev_t	dev;
{
	register struct worm	*wmp;
	int			unit = UNIT(dev);
	
	/* sleep while sw01rinit is running so sw01_worm is valid */
	while (rinit_flag) {
		sleep((caddr_t)rinit_flag, PRIBIO+1);
	}

	if ((int)unit >= sw01_wormcnt)
		return -1;

	wmp = &sw01_worm[unit];

	if ((wmp->wm_state & WM_PARMS) == 0) {
		if (sw01open(&dev, 0, OTYP_LYR, (struct cred *)0) ||
			sw01config(wmp))
		{
			return(-1);
		}
	}
	return(wmp->wm_capacity.wm_addr * wmp->wm_capacity.wm_len / BLKSIZE);
}

/*
** Function name: sw01ioctl()
** Description:
**	Driver ioctl() entry point.  Used to implement the following 
**	special functions:
**
**    B_GETTYPE		-  Get bus type and driver name
**    B_GETDEV		-  Get pass-thru major/minor numbers 
**    C_ERRMSGON	-  System error message ON	
**    C_ERRMSGOFF	-  System error message OFF
**    
**  Group 0 commands
**    W_TESTUNIT	-  Test unit ready
**    W_REZERO		-  Rezero unit
**    W_SEEK		-  Seek
**    W_INQUIR		-  Inquiry
**    W_STRTUNIT	-  Start unit
**    W_STOPUNIT	-  Stop unit
**    W_PREVMV		-  Prevent medium removal
**    W_ALLOMV		-  Allow medium removal
**
**  Group 1 commands
**    W_READCAPA	-  Read capacity
**    W_VERIFY		-  Verify
**
**  Group 6 commands
**    W_STNCHECK	-  Standby check
**    W_LOADCART	-  Load cartridge
**    W_UNLOADCA	-  Unload cartridge
**    W_READCB		-  Read control block
**
**  Group 7 commands
**    W_CHECK		-  Check
**    W_CCHECK		-  Contrary Check
*/

sw01ioctl(dev, cmd, arg, mode, cred_p, rval_p)
dev_t	dev;
int	cmd;
int	mode;
caddr_t	arg;
struct cred	*cred_p;
int		*rval_p;
{
	register struct worm *wmp;
	unsigned unit;
	int uerror;

	/* sleep while sw01rinit is running so sw01_worm is valid */
	while (rinit_flag) {
		sleep((caddr_t)rinit_flag, PRIBIO+1);
	}

	unit = UNIT(dev);
	wmp = &sw01_worm[unit];

	switch(cmd) {
	case	B_GETTYPE:
		/*
		 * Tell user bus and driver name
		 */
		if (copyout("scsi", ((struct bus_type *)arg)->bus_name, 5))
		{
			uerror = EFAULT;
			break;
		}
		if (copyout("sw01", ((struct bus_type *)arg)->drv_name, 5))
		{
			uerror = EFAULT;
			break;
		}
		break;

	case	B_GETDEV:
		/*
		 * Return pass-thru device major/minor 
		 * to user in arg.
		 */
		{
			dev_t	pdev;

			sdi_getdev(&wmp->wm_addr, &pdev);
			if (copyout((caddr_t)&pdev, arg, sizeof(pdev))) {
				uerror = EFAULT;
				break;
			}
			break;
		}

	case	SDI_RELEASE:	/* XXX */
		/*
		 * allow another processor on the same SCSI bus to accsess a 
		 * reserved drive.
		 */
		if (sw01cmd(wmp, SS_RELES, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	case	SDI_RESERVE:	/* XXX */
		/*
		 * reserve a drive to a processor.
		 */
		if (sw01cmd(wmp, SS_RESERV, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	/*
	 * The following ioctls are group 0 commamds
	 */
	case	W_TESTUNIT:	/* XXX */
		/*
		 * Test Unit Ready
		 */
		if (sw01cmd(wmp, SS_TEST, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	case	W_REZERO:	/* XXX */
		/*
		 * Rezero Unit 
		 */
		if (sw01cmd(wmp, SS_REZERO, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;
	
	case	W_SEEK:	/* XXX */
		/*
		 * Seek 
		 */
		if (sw01cmd(wmp, SS_SEEK, (int)arg, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	case	W_INQUIR:
		/*
		 * Inquire
		 */
	{
		struct worm_inq *inqp;
		int		inqlen;

		while (sw01_tmp_flag & B_BUSY) {
			sw01_tmp_flag |= B_WANTED;
			sleep((caddr_t)sw01_tmp, PRIBIO+1);
		}

		inqp = (struct worm_inq *)&sw01_tmp[200];
		if(copyin((char *)arg, (char *)inqp,
			sizeof(struct worm_inq)) < 0){
			uerror = EFAULT;
			goto INQUIR_EXIT;
		}
		if((inqp->length > INQ_SZ) || (inqp->length == 0)) 
			inqlen = INQ_SZ;
		else
			inqlen = inqp->length;
#ifdef DEBUG
		cmn_err(CE_NOTE,
			"WORM: W_INQUIR length=%x addr=%x inqlen %x",
			inqp->length, inqp->addr, inqlen);
#endif
		if (sw01cmd(wmp, SS_INQUIR, 0, (caddr_t)sw01_tmp, inqlen, inqlen,
			 SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
			goto INQUIR_EXIT;
		}
		if(copyout((char *)sw01_tmp,inqp->addr,inqlen))
			uerror = EFAULT;

INQUIR_EXIT:
		sw01_tmp_flag &= ~B_BUSY;
		if (sw01_tmp_flag & B_WANTED)
			wakeup((caddr_t)sw01_tmp);
		break;
	}

	case	W_STRTUNIT:	/* XXX */
		/*
		 * Start unit
		 */
		if (sw01cmd(wmp, SS_ST_SP, 0, NULL, 0, 1, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;
	case	W_STOPUNIT:	/* XXX */
		/*
		 * Stop unit
		 */
		if (sw01cmd(wmp, SS_ST_SP, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	case	W_PREVMV:	/* XXX */
		/*
		 * Prevent media removal
		 */
		if (sw01cmd(wmp, SS_LOCK, 0, NULL, 0, 1, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	case	W_ALLOMV:	/* XXX */
		/*
		 * Allow media removal
		 */
		if (sw01cmd(wmp, SS_LOCK, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	/*
	 * The following ioctls are group 1 commamds
	 */
	case	W_READCAPA:
		/*
		 * Read capacity
		 */
	{
		register struct capacity *cp;
		uint_t 	i;

		cp = &wmp->wm_capacity;

		if (sw01cmd(wmp, SM_RDCAP, 0, (char *)cp, RDCAP_SZ, 0, SCB_READ,
			 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
			wmp->wm_state &= ~WM_PARMS;
			break;
		}

		cp->wm_addr = sdi_swap32(cp->wm_addr);
		cp->wm_len  = sdi_swap32(cp->wm_len);

		/*
		 * Initialize the Block <-> Logical Sector shift amount.
		 */
		for (i=0; i < 32; i++) {
			if ((BLKSIZE << i) == cp->wm_len) {
				break;
			}
		}

		if ((BLKSIZE << i) != cp->wm_len) {
			cmn_err (CE_WARN, "WORM: Logical Unit %d - Invalid media format", wmp->wm_addr.sa_lun);
			cmn_err(CE_CONT, "!Logical block length (0x%x) not a power-of-two multiple of 0x%x", cp->wm_len, BLKSIZE);
			wmp->wm_state &= ~WM_PARMS;
		}
		wmp->wm_blkshft = i;
		
		if(copyout((char *)cp,arg,8))
			uerror = EFAULT;
	}
		break;

	case	W_VERIFY:	
		/*
		 * Verify
		 */
	{
		struct worm_verify *vrp;

		while (sw01_tmp_flag & B_BUSY) {
			sw01_tmp_flag |= B_WANTED;
			sleep((caddr_t)sw01_tmp, PRIBIO+1);
		}

		vrp = (struct worm_verify *)&sw01_tmp[200];
		if(copyin((char *)arg, (char *)vrp,
			sizeof(struct worm_verify)) < 0){
			uerror = EFAULT;
			goto VERIFY_EXIT;
		}
#ifdef	DEBUG
		cmn_err(CE_NOTE, "WORM: W_VERIFY start=%x num=%x",
			vrp->start, vrp->num);
#endif
		if (sw01cmd(wmp, SS_VERIFY, vrp->start, NULL, 0, vrp->num,
			SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}

VERIFY_EXIT:
		sw01_tmp_flag &= ~B_BUSY;
		if (sw01_tmp_flag & B_WANTED)
			wakeup((caddr_t)sw01_tmp);
		break;
	}

	/*
	 * The following ioctls are group 6 commamds
	 */
	case	W_STNCHECK:
		/*
		 * Standby check
		 */

		while (sw01_tmp_flag & B_BUSY) {
			sw01_tmp_flag |= B_WANTED;
			sleep((caddr_t)sw01_tmp, PRIBIO+1);
		}

		bzero((char *)sw01_tmp, 4);
		if (sw01cmd(wmp, SS_STNCHECK, 0, (caddr_t)&sw01_tmp[3], 1, 0,
			SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
			goto STNCHECK_EXIT;
		}
#ifdef DEBUG
		cmn_err(CE_NOTE, "WORM: W_STNCHECK data=%x",
			sw01_tmp[3]);
#endif
		if(copyout((char *)sw01_tmp,arg,4))
			uerror = EFAULT;

STNCHECK_EXIT:
		sw01_tmp_flag &= ~B_BUSY;
		if (sw01_tmp_flag & B_WANTED)
			wakeup((caddr_t)sw01_tmp);
		break;

	case	W_LOADCART:
		/* 
		 * Load cartridge
		 */
		if (sw01cmd(wmp, SS_LOADCA, 0, NULL, 0, 1, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	case	W_UNLOADCA:
		/* 
		 * Unload cartridge
		 */
		if (sw01cmd(wmp, SS_LOADCA, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
		}
		break;

	case	W_READCB:
		/*
		 * Read control block
		 */
		if (sw01cmd(wmp, SS_READCB, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
			break;
		}
		/*
		 * Read buffer #1
		 */

		while (sw01_tmp_flag & B_BUSY) {
			sw01_tmp_flag |= B_WANTED;
			sleep((caddr_t)sw01_tmp, PRIBIO+1);
		}
		if (sw01cmd(wmp, SS_READBUF, 0x1000000, (caddr_t)sw01_tmp,
			CB_SZ+4, CB_SZ+4, SCB_READ, 1, 0)) {
			if (uerror == 0) {
				uerror = ENXIO;
			}
			goto READCB_EXIT;
		}
#ifdef DEBUG
		cmn_err(CE_NOTE,"sw01: W_READCB 0 0x%x 1 0x%x 2 0x%x 3 0x%x",
			sw01_tmp[0], sw01_tmp[1], sw01_tmp[2], sw01_tmp[3]);
#endif
		if(copyout((char *)&sw01_tmp[4],(char *)arg,CB_SZ))
			uerror = EFAULT;

READCB_EXIT:
		sw01_tmp_flag &= ~B_BUSY;
		if (sw01_tmp_flag & B_WANTED)
			wakeup((caddr_t)sw01_tmp);
		break;

	/*
	 * The following ioctls are group 7 commamds
	 */
	case	W_CHECK:
		/*
		 * Check
		 */
	case	W_CCHECK:
		/*
		 * Contrary Check
		 */
	{
		struct worm_check *ckp;
		int	block,result;

		while (sw01_tmp_flag & B_BUSY) {
			sw01_tmp_flag |= B_WANTED;
			sleep((caddr_t)sw01_tmp, PRIBIO+1);
		}

		ckp = (struct worm_check *)&sw01_tmp[200];
		if(copyin((char *)arg, (char *)ckp,
			sizeof(struct worm_check)) < 0){
			uerror = EFAULT;
			goto CCHECK_EXIT;
		}
#ifdef	DEBUG
		cmn_err(CE_NOTE, "WORM: W_[C]CHECK start=%x num=%x",
			ckp->start,ckp->num);
#endif
		if(cmd == W_CHECK){
			if (sw01cmd(wmp, SS_CHECK, ckp->start, (caddr_t)sw01_tmp,
				4, ckp->num, SCB_READ, 1, 0)) {
				if (uerror == 0) {
					uerror = ENXIO;
				}
				goto CCHECK_EXIT;
			}
		} else {
			if (sw01cmd(wmp, SS_CHECK, ckp->start, (caddr_t)sw01_tmp,
				4, ckp->num, SCB_READ, 0, 0)) {
				if (uerror == 0) {
					uerror = ENXIO;
				}
				goto CCHECK_EXIT;
			}
		}
#ifdef DEBUG
		cmn_err(CE_NOTE,"WORM: W_[C]CHECK 0 0x%x 1 0x%x 2 0x%x 3 0x%x",
			sw01_tmp[0], sw01_tmp[1], sw01_tmp[2], sw01_tmp[3]);
#endif
		ckp->block = (sw01_tmp[0] << 16) | (sw01_tmp[1] << 8) |
			 sw01_tmp[2];
		ckp->result = (long)sw01_tmp[3];
		if(copyout((char *)ckp, (char *)arg,
			sizeof(struct worm_check)) < 0){
			uerror = EFAULT;
#ifdef	DEBUG
			cmn_err(CE_NOTE, "WORM: W_[C]CHECK copyout failed");
#endif
		}

CCHECK_EXIT:
		sw01_tmp_flag &= ~B_BUSY;
		if (sw01_tmp_flag & B_WANTED)
			wakeup((caddr_t)sw01_tmp);
	}
		break;

	default:
		uerror = EINVAL;
	}
	return(uerror);
}


/*
** Function name: sw01cmd()
** Description:
**	This function performs a SCSI command such as Mode Sense on
**	the addressed worm.  The op code indicates the type of job
**	but is not decoded by this function.  The data area is
**	supplied by the caller and assumed to be in kernel space. 
**	This function will sleep.
*/

sw01cmd(wmp, op_code, addr, buffer, size, length, mode, param, control)
register struct worm	*wmp;
unsigned char	op_code;		/* Command opcode		*/
unsigned int	addr;			/* Address field of command 	*/
char		*buffer;		/* Buffer for command data 	*/
unsigned int	size;			/* Size of the data buffer 	*/
unsigned int	length;			/* Block length in the CDB	*/
unsigned short	mode;			/* Direction of the transfer 	*/
unsigned int	param;
unsigned int	control;
{
	register struct job *jp;
	register struct scb *scb;
	register buf_t *bp;
	int error, s;

	bp = getrbuf(KM_SLEEP);
	
	jp = sw01getjob();
	scb = &jp->j_sb->SCB;
	
	bp->b_iodone = NULL;
	bp->b_sector = addr;
	bp->b_blkno = addr * (wmp->wm_capacity.wm_len / BLKSIZE);
	bp->b_flags |= mode & SCB_READ ? B_READ : B_WRITE;
	bp->b_error = 0;
	
	jp->j_bp = bp;
	jp->j_wmp = wmp;
	jp->j_errcnt = 0;
	jp->j_sb->sb_type = SCB_TYPE;

	switch(op_code >> 5){
	case	GROUP7:
	{
		register struct scm *cmd;

		cmd = (struct scm *)&jp->j_cmd.sm;
		cmd->sm_op   = op_code;
		cmd->sm_lun  = wmp->wm_addr.sa_lun;
		cmd->sm_res1 = 0;
		cmd->sm_addr = sdi_swap32(addr);
		cmd->sm_res2 = 0;
		cmd->sm_len  = sdi_swap16(length);
		cmd->sm_res1 = param;
		cmd->sm_cont = control;

		scb->sc_cmdpt = SCM_AD(cmd);
		scb->sc_cmdsz = SCM_SZ;
	}
		break;
	case	GROUP6:
	{
		register struct scs *cmd;

		cmd = (struct scs *)&jp->j_cmd.ss;
		cmd->ss_op    = op_code;
		cmd->ss_lun   = wmp->wm_addr.sa_lun;
		cmd->ss_addr1 = (addr & 0x1F0000);
		cmd->ss_addr = sdi_swap16(addr & 0xFFFF);
		cmd->ss_len   = length;
		cmd->ss_cont  = control;

		scb->sc_cmdpt = SCS_AD(cmd);
		scb->sc_cmdsz = SCS_SZ;
	}
		break;
	case	GROUP1:
	{
		register struct scm *cmd;

		cmd = (struct scm *)&jp->j_cmd.sm;
		cmd->sm_op   = op_code;
		cmd->sm_lun  = wmp->wm_addr.sa_lun;
		cmd->sm_res1 = param;
		cmd->sm_addr = sdi_swap32(addr);
		cmd->sm_res2 = 0;
		cmd->sm_len  = sdi_swap16(length);
		cmd->sm_cont = 0;

		scb->sc_cmdpt = SCM_AD(cmd);
		scb->sc_cmdsz = SCM_SZ;
	}
		break;
	case	GROUP0:
	{
		register struct scs *cmd;

		cmd = (struct scs *)&jp->j_cmd.ss;
		cmd->ss_op    = op_code;
		cmd->ss_lun   = wmp->wm_addr.sa_lun;
		cmd->ss_addr1 = (addr & 0x1F0000);
		cmd->ss_addr = sdi_swap16(addr & 0xFFFF);
		cmd->ss_len   = length;
		cmd->ss_cont  = 0;

		scb->sc_cmdpt = SCS_AD(cmd);
		scb->sc_cmdsz = SCS_SZ;
	}
		break;
	default:
		cmn_err(CE_WARN,
			"WORM Error: Unknown SCSI command byte = %x\n",
			op_code);
	}
	
	/* Fill in the SCB */
	scb->sc_int = sw01intn;
	scb->sc_dev = wmp->wm_addr;
	scb->sc_datapt = buffer;
	scb->sc_datasz = size;
	scb->sc_mode = mode;
	scb->sc_resid = 0;
	scb->sc_time = JTIME;
	scb->sc_wd = (long) jp;
	sdi_translate(jp->j_sb, bp->b_flags, (caddr_t)0);

	/* Add job to the queue at the end and batch the queue */
	s = spl5();
	wmp->wm_count++;
	jp->j_next = (struct job *) wmp;
	jp->j_prev = wmp->wm_last;
	wmp->wm_last->j_next = jp;
	wmp->wm_last = jp;
	if (wmp->wm_next == (struct job *) wmp)
		wmp->wm_next = jp;
	wmp->wm_batch = (struct job *) wmp;
	
	sw01send(wmp);
	splx(s);

	biowait(bp);
		
	error = bp->b_flags & B_ERROR;
	freerbuf(bp);
	return(error);
}


/*
** Function name: sw01config()
** Description:
**	Initializes the worm driver's worm paramerter structure.  If the
**	Read Capacity is not supported, u_error is cleared so not to fail
**	the open routine.  In this case, the V_CONFIG ioctl can be used
**	to set drive parameters.
*/

sw01config(wmp)
register struct worm *wmp;
{
	register struct capacity *cp;
	int	i;

	cp = &wmp->wm_capacity;

	if (sw01cmd(wmp, SS_TEST, 0, NULL, 0, 0, SCB_READ, 0, 0)) {
		cmn_err(CE_WARN,
			"WORM Error: Logical Unit %d - Device not ready",
			wmp->wm_addr.sa_lun);
		return(-1);
	}
	/* Send READ CAPACITY to obtain last sector address */
	if (sw01cmd(wmp, SM_RDCAP, 0, (char *)cp, RDCAP_SZ, 0, SCB_READ, 0, 0))
	{
#ifdef DEBUG
		cmn_err(CE_WARN, "WORM Error: Logical Unit %d - Unable to send Read Capacity command", wmp->wm_addr.sa_lun);
#endif
		return(EIO);
	}

	cp->wm_addr = sdi_swap32(cp->wm_addr);
	cp->wm_len  = sdi_swap32(cp->wm_len);

	/*
	 * Check if the parameters are vaild
	 */
	if (cp->wm_addr <= 0)
	{
		cmn_err(CE_WARN,
			"WORM Error: Logical Unit %d - Invalid media format",
			wmp->wm_addr.sa_lun);
		cmn_err(CE_CONT, "!Invalid capacity value = 0x%x.\n", 
			cp->wm_addr);
		wmp->wm_state &= ~WM_PARMS;
		return(-1);
	}
	if (cp->wm_len <= 0)
	{
		cmn_err(CE_WARN,
			"WORM Error: Logical Unit %d - Invalid media format",
			wmp->wm_addr.sa_lun);
		cmn_err(CE_CONT, "!Invalid logical block length = 0x%x\n", 
			cp->wm_len);
		wmp->wm_state &= ~WM_PARMS;
		return(-1);
	}

	/*
	 * Initialize the Block <-> Logical Sector shift amount.
	 */
	for (i=0; i < 32; i++) {
		if ((BLKSIZE << i) == cp->wm_len) {
			break;
		}
	}

	if ((BLKSIZE << i) != cp->wm_len) {
		cmn_err(CE_WARN,
			"WORM Error: Logical Unit %d - Invalid media format",
			wmp->wm_addr.sa_lun);
		cmn_err(CE_CONT, "!Logical block length (0x%x) not a power-of-two multiple of 0x%x", 
			cp->wm_len, BLKSIZE);
		wmp->wm_state &= ~WM_PARMS;
		return(-1);
	}
	
	wmp->wm_blkshft = i;
	
	/*
	 * Indicate parameters are set and valid
	 */
	wmp->wm_state |= WM_PARMS; 
	return(0);
}
	

/*
** Function name: sw01comp()
** Description:
**	On completion of a job, both successful and failed, this function
**	removes the job from the work queue, de-allocates the job structure
**	used, and calls biodone().  The function will restart the logical
**	unit queue if necessary.
*/

sw01comp(jp)
register struct job *jp;
{
        register struct worm *wmp;
	register struct buf *bp;
        
        wmp = jp->j_wmp;
        bp = jp->j_bp;

	/* Remove job from the queue */
	jp->j_next->j_prev = jp->j_prev;
	jp->j_prev->j_next = jp->j_next;

	wmp->wm_count--;
	wmp->wm_npend--;

	/* Check if job completed successfully */
	if (jp->j_sb->SCB.sc_comp_code != SDI_ASW)
	{
		bp->b_flags |= B_ERROR;
		if (jp->j_sb->SCB.sc_comp_code == SDI_NOSELE)
			bp->b_error = ENODEV;
		else
			bp->b_error = EIO;
	}

	biodone(bp);
	sw01freejob(jp);

	/* Resume queue if suspended */
	if (wmp->wm_state & WM_SUSP)
	{
		wmp->wm_fltres->sb_type = SFB_TYPE;
		wmp->wm_fltres->SFB.sf_int  = sw01intf;
		wmp->wm_fltres->SFB.sf_dev  = wmp->wm_addr;
		wmp->wm_fltres->SFB.sf_wd = (long) wmp;
		wmp->wm_fltres->SFB.sf_func = SFB_RESUME;
		sdi_icmd(wmp->wm_fltres);

		wmp->wm_state &= ~WM_SUSP;
		wmp->wm_fltcnt = 0;
	}

        sw01send(wmp); 
}


/*
** Function name: sw01intn()
** Description:
**	This function is called by the host adapter driver when a SCB
**	job completes.  If the job completed with an error the job will
**	be retried when appropriate.  Requests which still fail or are
**	not retried are failed.
*/

void
sw01intn(sp)
register struct sb	*sp;
{
	register struct worm	*wmp;
	register struct job	*jp;

	jp = (struct job *)sp->SCB.sc_wd;
	wmp = jp->j_wmp;

	if (sp->SCB.sc_comp_code == SDI_ASW) {
		sw01comp(jp);
		return;
	}

	if (sp->SCB.sc_comp_code & SDI_SUSPEND)
		wmp->wm_state |= WM_SUSP;

	if (sp->SCB.sc_comp_code == SDI_CKSTAT && sp->SCB.sc_status == S_CKCON)
	{
		wmp->wm_fltjob = jp;

		wmp->wm_fltreq->sb_type = ISCB_TYPE;
		wmp->wm_fltreq->SCB.sc_int = sw01intrq;
		wmp->wm_fltreq->SCB.sc_cmdsz = SCS_SZ;
		wmp->wm_fltreq->SCB.sc_time = JTIME;
		wmp->wm_fltreq->SCB.sc_mode = SCB_READ;
		wmp->wm_fltreq->SCB.sc_dev = sp->SCB.sc_dev;
		wmp->wm_fltreq->SCB.sc_wd = (long) wmp;
		wmp->wm_fltcmd.ss_op = SS_REQSEN;
		wmp->wm_fltcmd.ss_lun = sp->SCB.sc_dev.sa_lun;
		wmp->wm_fltcmd.ss_addr1 = 0;
		wmp->wm_fltcmd.ss_addr = 0;
		wmp->wm_fltcmd.ss_len = SENSE_SZ;
		wmp->wm_fltcmd.ss_cont = 0;

		/* Clear old sense key */
		wmp->wm_sense.sd_key = SD_NOSENSE;

		sdi_icmd(wmp->wm_fltreq);
		return;
	}

	if (sp->SCB.sc_comp_code & SDI_RETRY && ++jp->j_errcnt <= MAXRETRY)
	{
		sp->sb_type = ISCB_TYPE;
		sp->SCB.sc_time = JTIME;
		sdi_icmd(sp);
		return;
	}

	sw01logerr(wmp, sp);
	sw01comp(jp);
}


/*
** Function name: sw01intrq()
** Description:
**	This function is called by the host adapter driver when a
**	request sense job completes.  The job will be retied if it
**	failed.  Calls sw01sense() on sucessful completions to
**	examine the request sense data.
*/

void
sw01intrq(sp)
register struct sb *sp;
{
	register struct worm *wmp;

	wmp = (struct worm *)sp->SCB.sc_wd;

	if (sp->SCB.sc_comp_code != SDI_CKSTAT  &&
	    sp->SCB.sc_comp_code &  SDI_RETRY   &&
	    ++wmp->wm_fltcnt <= MAXRETRY)
	{
		sp->SCB.sc_time = JTIME;
		sdi_icmd(sp);
		return;
	}

	if (sp->SCB.sc_comp_code != SDI_ASW) {
		sw01logerr(wmp, sp);
		sw01comp(wmp->wm_fltjob);
		return;
	}

	sw01sense(wmp);
}


/*
** Function name: sw01intf()
** Description:
**	This function is called by the host adapter driver when a host
**	adapter function request has completed.  If there was an error
**	the request is retried.  Used for resume function completions.
*/

void
sw01intf(sp)
register struct sb *sp;
{
	register struct worm *wmp;

	wmp = (struct worm *)sp->SFB.sf_wd;

	if (sp->SFB.sf_comp_code & SDI_RETRY && ++wmp->wm_fltcnt <= MAXRETRY)
	{
		sdi_icmd(sp);
		return;
	}

	if (sp->SFB.sf_comp_code != SDI_ASW) 
		sw01logerr(wmp, sp);
}


/*
** Function name: sw01sense()
** Description:
**	This function uses the Request Sense information to determine
**	what to do with the original job.
*/

sw01sense(wmp)
register struct worm *wmp;
{
	register struct job *jp;
	register struct sb *sp;

	jp = wmp->wm_fltjob;
	sp = jp->j_sb;

        switch(wmp->wm_sense.sd_key)
	{
	case SD_NOSENSE:
	case SD_ABORT:
	case SD_VENUNI:
		sw01logerr(wmp, sp);

	case SD_UNATTEN:
		if (++jp->j_errcnt > MAXRETRY)
			sw01comp(jp);
		else {
			sp->sb_type = ISCB_TYPE;
			sp->SCB.sc_time = JTIME;
			sdi_icmd(sp);
		}
		break;

	case SD_RECOVER:
		sw01logerr(wmp, sp);
		sp->SCB.sc_comp_code = SDI_ASW;
		sw01comp(jp);
		break;

	default:
		sw01logerr(wmp, sp);
		sw01comp(jp);
        }
}


/*
** Function name: sw01logerr()
** Description:
**	This function will print the error messages for errors detected
**	by the host adapter driver.  No message will be printed for
**	thoses errors that the host adapter driver has already reported.
*/

sw01logerr(wmp, sp)
register struct worm *wmp;
register struct sb *sp;
{
	if (sp->sb_type == SFB_TYPE)
	{
		sdi_errmsg("WORM",&wmp->wm_addr,sp,&wmp->wm_sense,SDI_SFB_ERR,0);
		return;
	}

	if (sp->SCB.sc_comp_code == SDI_CKSTAT && sp->SCB.sc_status == S_CKCON)
	{
		sdi_errmsg("WORM",&wmp->wm_addr,sp,&wmp->wm_sense,SDI_CKCON_ERR,0);
		return;
	}

	if (sp->SCB.sc_comp_code == SDI_CKSTAT)
	{
		sdi_errmsg("WORM",&wmp->wm_addr,sp,&wmp->wm_sense,SDI_CKSTAT_ERR,0);
	}
}


extern sdi_send(), sdi_icmd();

SW01SEND(jobp)
struct job *jobp;
{
	return sw01docmd(sdi_send, jobp->j_wmp, jobp->j_sb);
}

SW01ICMD(wormp, sbp)
struct worm *wormp;
struct sb *sbp;
{
	return sw01docmd(sdi_icmd, wormp, sbp);
}

int
sw01docmd(fcn, wormp, sbp)
int (*fcn)();
struct worm *wormp;
struct sb *sbp;
{
	struct dev_spec *dsp = wormp->wm_spec;
	int cmd;

	if (dsp && sbp->sb_type != SFB_TYPE) {
		cmd = ((struct scs *)sbp->SCB.sc_cmdpt)->ss_op;
		if (!CMD_SUP(cmd, dsp)) {
			return SDI_RET_ERR;
		} else if (dsp->command && CMD_CHK(cmd, dsp)) {
			(*dsp->command)(wormp, sbp);
		}
	}

	return (*fcn)(sbp);
}
