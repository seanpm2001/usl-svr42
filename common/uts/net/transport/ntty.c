/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-comm:net/transport/ntty.c	1.3"

#include <io/conf.h>
#include <io/stream.h>
#include <io/stropts.h>
#include <io/termio.h>
#include <io/ttold.h>
#include <net/transport/tihdr.h>
#include <proc/signal.h>
#include <proc/user.h>
#include <svc/errno.h>
#include <svc/systm.h>
#include <util/param.h>
#include <util/types.h>

#ifdef DBUG
#define DDEBUG(a)	 if (debug) printf(a) 
#else
#define DDEBUG(a)
#endif 

#define INUSE 		0x1
#define WACK 		0x2
#define W_INFO_ACK	0x4
#define	EX_DATA_SUP	0x8
#define	EX_BREAK_HI	0x00
#define EX_BREAK_LO	0x01
#define	EX_BREAK_SIZE	2

struct Ntty
{
	tcflag_t cflags;/* copy of cflags to keep setty happy */
	int state;	/* state of ntty device */
	mblk_t *mp;	/* pointer to preallocated message block */
	mblk_t *ioc_mp; /* saved reply for stty 0 - disconnect request */
	tcflag_t brk_flag; /* break flags in iflags */
};

extern struct Ntty ntty[];
extern int nt_cnt;

static int debug = 0;

static char *slanversion = "aeh 08/17/89";

extern nulldev();

#define NTTY_ID	4444


/* stream data structure definintons */
int ntopen(), ntclose(), ntrput(), ntwput();
static struct module_info nt_info = {NTTY_ID, "ntty", 0, INFPSZ, 4096, 1024};
static struct qinit ntrinit = { ntrput, NULL, ntopen, ntclose, nulldev, &nt_info, NULL};
static struct qinit ntwinit = { ntwput, NULL, ntopen, ntclose, nulldev, &nt_info, NULL};
struct streamtab ntinfo = { &ntrinit, &ntwinit, NULL, NULL };


int ntdevflag = D_OLD;

/*
 * ntopen - open rountne gets called when the
 *	       module gets pushed onto the stream.
 */

ntopen(q, dev, sflag, oflag)
register queue_t *q;
int dev, sflag, oflag;
{
	register struct Ntty *ntp;
	register queue_t *nq;
	register mblk_t *mop;
	register union T_primitives *t_prim;

	DDEBUG(("nt: open()\t"));

	if (q->q_ptr != NULL)			/* already attached */
	{
		return(1);
	}



	for (ntp = ntty; ntp->state&INUSE; ntp++)
		if (ntp >= &ntty[nt_cnt-1])
		{
			DDEBUG(("!No ntty structures.\t"));
			u.u_error = ENOSPC;
			return(OPENFAIL);
		}


	/* allocate a message block, used to generate disconnect for "stty 0"  */
	if ((ntp->mp = (mblk_t *)allocb(sizeof(struct T_discon_req), BPRI_HI)) == NULL) {
		u.u_error = ENOMEM;
		return(OPENFAIL);
	}

/************************************************************************/
/*
 * Fix proposed by Streams Group for changing URP hi/lo water marks
 * for network tty service.

 NOTE: This is a temporary solution and is to be removed when a long term
 solution is in place. 

 */
	/*
	 * set up hi/lo water marks on stream head read queue
	 */
	if (mop = allocb(sizeof(struct stroptions), BPRI_HI)) {
		register struct stroptions *sop;
		
		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof(struct stroptions);
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
		sop->so_hiwat = 512;
		sop->so_lowat = 256;
		putnext(q, mop);
	} else {
		freemsg(ntp->mp);
		ntp->mp = NULL;
		u.u_error = ENOMEM;
		return(OPENFAIL);
	}
	/*
	 * find the next write queue downstream that has a service
	 * procedure (or the driver, whichever comes first) and
	 * reset its hi/lo water marks.  This will not work for
	 * all situations but definitely works for URP either with
	 * or without timod in between.
	 */
	nq = WR(q)->q_next;
	while (!nq->q_qinfo->qi_srvp && nq->q_next) nq = nq->q_next;
	nq->q_hiwat = 512;
	nq->q_lowat = 256;
		
/************************************************************************/



	q->q_ptr = (caddr_t)ntp;
	WR(q)->q_ptr = (caddr_t)ntp;
	ntp->cflags = (tcflag_t) (B300|CS8|CREAD|HUPCL);
	ntp->state = INUSE;

	/* Find out if expedited data is used by transport */

	if((mop = allocb(sizeof(union T_primitives), BPRI_MED)) == NULL)
	{
		freemsg(ntp->mp);
		ntp->mp = NULL;
		u.u_error = ENOMEM;
		return(OPENFAIL);
	}

	/* Send a T_info_req to the transport to find out if expedited
	   data is supported */

	t_prim = (union T_primitives *) mop->b_rptr;

	mop->b_wptr = mop->b_rptr + sizeof(union T_primitives);

	t_prim->type = T_INFO_REQ;

	mop->b_datap->db_type = M_PCPROTO;

	ntp->state |= W_INFO_ACK;

	putnext(WR(q), mop);

	if(ntp->state & W_INFO_ACK)
	{
		if(sleep((caddr_t)ntp, (PSLEP|PCATCH)) > 0)
		{
			u.u_error = EINTR;
			return(OPENFAIL);
		}
	}

	return(1);

}


/*
 * ntclose - This rountne gets called when the module
 *              gets popped off of the stream.
 */

ntclose(q)
register queue_t *q;
{
	register struct Ntty *ntp = (struct Ntty *)q->q_ptr;

	DDEBUG(("nt: ntclose()\t"));

	putctl1(q->q_next, M_PCSIG, SIGHUP);

	ntp->state = 0;
	q->q_ptr = (caddr_t)NULL;
	WR(q)->q_ptr = (caddr_t)NULL;
	if (ntp->mp)
	{
		freemsg(ntp->mp);
		ntp->mp = NULL;
	}
	if (ntp->ioc_mp)
	{
		freemsg(ntp->ioc_mp);
		ntp->ioc_mp = NULL;
	}
	return;
}


/*
 * ntrput - Module read queue put procedure.
 *             This is called from the module or
 *	       driver downstream.
 */

ntrput(q, mp)
register queue_t *q;
register mblk_t *mp;
{
	register union T_primitives *pptr;
	register mblk_t *ctlmp;
	struct Ntty *ntp;

	DDEBUG(("nt: ntrput()\t"));

	ntp = (struct Ntty *)q->q_ptr;

	switch(mp->b_datap->db_type)
	{
	case M_DATA:
		if(!(ntp->cflags & CREAD))
		{
			freemsg(mp);
			return;
		}
		break;
	case M_DELAY:
		freemsg(mp);
		return;
		
	case M_PCPROTO:
	case M_PROTO:
		while(mp->b_wptr == mp->b_rptr)
		{
			if(mp->b_cont == NULL)
			{
				if(mp->b_datap)
                                	freeb(mp);

				return;
			}

			ctlmp = mp->b_cont;
			mp->b_cont = NULL;
			freeb(mp);
			mp = ctlmp;
			ctlmp = NULL;
		}


		pptr = (union T_primitives *)mp->b_rptr;
		switch(pptr->type) 
		{
                 	case T_DATA_IND:
                                if(!(ntp->cflags & CREAD))
                                {
                                 	freemsg(mp);
                                        return;
                                }

                                break;

			case T_INFO_ACK:
				if(ntp->state & W_INFO_ACK)
				{
					ntp->state &= ~W_INFO_ACK;

					if(pptr->info_ack.ETSDU_size <= 0)
						ntp->state &= ~EX_DATA_SUP;
					else
						ntp->state |= EX_DATA_SUP;
					freemsg(mp);
					wakeup((caddr_t)ntp);
					return;
				}

				break;

			case T_ERROR_ACK:
			case T_OK_ACK:
				if (ntp->state& WACK)
				{	/* waiting for disconnect? */
					DDEBUG(("nt: disconnect acknowledged\t"));
					
					ntp->state&= ~WACK;
					freemsg(mp);
					if (ntp->ioc_mp) 
					{
						putnext(q, ntp->ioc_mp);
						ntp->ioc_mp = NULL;
					}
					putctl(q->q_next, M_HANGUP);
					return;
				}
				break;
			case T_EXDATA_IND:
				ctlmp = mp->b_cont;
				if(*ctlmp->b_rptr == EX_BREAK_HI &&
*(ctlmp->b_rptr + 1) == EX_BREAK_LO)
				{
					if((!(ntp->brk_flag & IGNBRK)) &&
(ntp->brk_flag & BRKINT))
					{
						mp->b_cont = NULL;
						ctlmp->b_wptr = ctlmp->b_rptr = ctlmp->b_datap->db_base;
						*ctlmp->b_wptr++ = (char)SIGINT;
						ctlmp->b_datap->db_type = M_PCSIG;
						putnext(q, ctlmp);
					}

					freemsg(mp);
					return;
				}
				else
					break;
		}
		break;
				
	}
	putnext(q,mp);
}


/*
 * ntwput - Module write queue put procedure.
 *             This is called from the module or
 *	       stream head upstream.
 */

ntwput(q, mp)
register queue_t *q;
register mblk_t *mp;
{
	struct iocblk *iocp;
	struct termio *cb;
	struct termios *termiosp;
	struct sgttyb *gb;
	struct Ntty *ntp;
	register union T_primitives *pptr;
	mblk_t *ctlmp;

	DDEBUG(("nt: ntwput()\t"));

	iocp = (struct iocblk *)mp->b_rptr;
	ntp = (struct Ntty *)q->q_ptr;

	/* Acknowlege all M_IOCTL messages of a tty nature here; queue everything else.	*/
	switch (mp->b_datap->db_type)
	{
		
		case M_DATA:
			putnext(q, mp);
			break;
		
#ifdef FLOWCTL  	/* remove this ifdef when flow control 
			   is properly tested */
		case M_STOP:
			freemsg(mp);
			if (ctlmp = allocb(4, BPRI_MED));
			{
				/* send ASCII DC3 - Stop character */
				*ctlmp->b_wptr++ = 023;	
				putnext(q, ctlmp);
			}
			break;
		
		case M_START:
			freemsg(mp);
			if (ctlmp = allocb(4, BPRI_MED));
			{
				/* send ASCII DC1 - Start character */
				*ctlmp->b_wptr++ = 021;	
				putnext(q, ctlmp);
			}
			break;
#endif

		case M_IOCTL:
		switch (iocp->ioc_cmd)
		{

			case TCSETA:
			case TCSETAF:
			case TCSETAW:
				cb = (struct termio *)mp->b_cont->b_rptr;
				ntp->cflags = cb->c_cflag;
				ntp->brk_flag = cb->c_iflag&(BRKINT|IGNBRK);
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_count = 0;

#ifdef DBUG
				if ((cb->c_cflag&CBAUD) == B50)
					debug = 1;
				if ((cb->c_cflag&CBAUD) == B75)
					debug = 0;
#endif
	
				if ((cb->c_cflag&CBAUD) == B0)
				{
				/* hang-up: generate a disconnect request */

					DDEBUG(("ntwput(): generate disconnect request\t"));
					ntp->ioc_mp = mp;
					mp = ntp->mp;

					if (mp)
					{
						ntp->mp = NULL;	/* use only once */
						pptr = (union T_primitives *)mp->b_rptr;

						mp->b_wptr = mp->b_rptr + sizeof(struct T_discon_req);
						pptr->type = T_DISCON_REQ;
						pptr->discon_req.SEQ_number = -1;
						mp->b_datap->db_type = M_PROTO;
						ntp->state |= WACK;
						putnext(q, mp);
					}
					return;
				}

				qreply(q, mp);
				return;

			case TCSETS:
			case TCSETSF:
			case TCSETSW:
				termiosp = (struct termios *)mp->b_cont->b_rptr;
				ntp->cflags = termiosp->c_cflag;
				ntp->brk_flag = termiosp->c_iflag&(BRKINT|IGNBRK);
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_count = 0;

#ifdef DBUG
				if ((termiosp->c_cflag&CBAUD) == B50)
					debug = 1;
				if ((termiosp->c_cflag&CBAUD) == B75)
					debug = 0;
#endif

                                if ((termiosp->c_cflag&CBAUD) == B0)                  				{
				/* hang-up: generate a disconnect request */

					DDEBUG(("ntwput(): generate disconnect request\t"));
					ntp->ioc_mp = mp;
					mp = ntp->mp;
                                                                                					if (mp)
                                        {                                       						ntp->mp = NULL; /* use only once */
						pptr = (union T_primitives *)mp->b_rptr;

                                                mp->b_wptr = mp->b_rptr + sizeof(struct T_discon_req);
                                                pptr->type = T_DISCON_REQ;
                                                pptr->discon_req.SEQ_number = -1;
                                                mp->b_datap->db_type = M_PROTO;
                                                ntp->state |= WACK;
                                                putnext(q, mp);
                                        }
                                        return;
                                }

                                qreply(q, mp);
                                return;


			case TCGETA:
				if(!(mp->b_cont = 
					(allocb(sizeof (struct termio), BPRI_HI))))
				{
					mp->b_datap->db_type = M_IOCNAK;
					iocp->ioc_error = ENOSR;
					iocp->ioc_count = 0;
					qreply(q, mp);
					return;
				}

				bzero((caddr_t)mp->b_cont->b_rptr, sizeof(struct termio));
				mp->b_cont->b_wptr =  mp->b_cont->b_rptr +
					sizeof(struct termio);
				cb = (struct termio *)mp->b_cont->b_rptr;
				cb->c_cflag = ntp->cflags;
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_count = sizeof(struct termio);
				qreply(q, mp);
				return;

			case TCGETS:
				if ( mp->b_cont)
					freemsg( mp->b_cont);

				if ( !( mp->b_cont = ( allocb( sizeof (struct termios), BPRI_MED)))) {
					mp->b_datap->db_type = M_IOCNAK;
					iocp->ioc_error = EAGAIN;
					iocp->ioc_count = 0;
					qreply( q, mp);
					break;
				}

				bzero((caddr_t)mp->b_cont->b_rptr, sizeof(struct termios));
				mp->b_cont->b_wptr =  mp->b_cont->b_rptr + sizeof(struct termios);
				termiosp = (struct termios *)mp->b_cont->b_rptr;
				termiosp->c_cflag = ntp->cflags;
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_count = sizeof( struct termios);
				qreply( q, mp);
				break;

			case TCSBRK:
				if(!(*(int *)mp->b_cont->b_rptr))
				{
					if(ntp->state & EX_DATA_SUP)
					{
						if((ctlmp = allocb(sizeof(union T_primitives),
BPRI_MED)) == NULL)
						{
							mp->b_datap->db_type = M_IOCNAK;
							iocp->ioc_error = ENOSR;
							iocp->ioc_count = 0;
							qreply(q, mp);
							return;
						}

						if((ctlmp->b_cont = allocb(EX_BREAK_SIZE,
BPRI_MED)) == NULL)
						{
							mp->b_datap->db_type = M_IOCNAK;
							iocp->ioc_error = ENOSR;
							iocp->ioc_count = 0;
							qreply(q, mp);
							return;
						}

						ctlmp->b_wptr = ctlmp->b_rptr +
sizeof(union T_primitives);
						pptr = (union T_primitives *) ctlmp->b_rptr;
						pptr->exdata_req.PRIM_type = T_EXDATA_REQ;
						pptr->exdata_req.MORE_flag = 0;
						ctlmp->b_datap->db_type = M_PROTO;

						ctlmp->b_cont->b_wptr = ctlmp->b_cont->b_rptr;
						*ctlmp->b_cont->b_wptr++ = EX_BREAK_HI;
						*ctlmp->b_cont->b_wptr++ = EX_BREAK_LO;

						ctlmp->b_cont->b_datap->db_type = M_DATA;
						putnext(q, ctlmp);
					}
					else
						putctl(q->q_next, M_BREAK);
				}


				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_count = 0;
				qreply(q, mp);
				return;

			case TIOCGETP:
				if((mp->b_cont = allocb(sizeof(struct sgttyb), BPRI_HI)) == NULL)
				{
					mp->b_datap->db_type = M_IOCNAK;
					iocp->ioc_error = ENOMEM;
					qreply(q, mp);
				}

				gb = (struct sgttyb *)mp->b_cont->b_rptr;

				gb->sg_ispeed = ntp->cflags&CBAUD;
				gb->sg_ospeed = gb->sg_ispeed;

				gb->sg_flags = 0;

#ifndef O_HUPCL
#define O_HUPCL 01
#endif
				if(ntp->cflags&HUPCL)
					gb->sg_flags |= O_HUPCL;

				if(ntp->cflags&PARODD)
					gb->sg_flags |= O_ODDP;

				mp->b_cont->b_wptr += sizeof(struct sgttyb);
				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_error = 0;
				iocp->ioc_count = sizeof(struct sgttyb);
				qreply(q, mp);
				return;

			case TIOCSETP:
				gb = (struct sgttyb *)mp->b_cont->b_rptr;

				ntp->cflags = (gb->sg_ispeed&CBAUD)|CREAD;
				if((ntp->cflags&CBAUD) == B110)
					ntp->cflags |= CSTOPB;
				if(gb->sg_flags&O_HUPCL)
					ntp->cflags |= HUPCL;
				if(!(gb->sg_flags&O_RAW))
					ntp->cflags |= CS7|PARENB;
				if(gb->sg_flags&O_ODDP)
					if(!(gb->sg_flags&O_EVENP))
						ntp->cflags |= PARODD;

				mp->b_datap->db_type = M_IOCACK;
				iocp->ioc_count = 0;
				iocp->ioc_error = 0;
	
				if ((ntp->cflags&CBAUD) == B0)
				{
				/* hang-up: generate a disconnect request */

					DDEBUG(("ntwput(): generate disconnect request\t"));
					ntp->ioc_mp = mp;
					mp = ntp->mp;

					if (mp)
					{
						ntp->mp = NULL;	/* use only once */
						pptr = (union T_primitives *)mp->b_rptr;

						mp->b_wptr = mp->b_rptr + sizeof(struct T_discon_req);
						pptr->type = T_DISCON_REQ;
						pptr->discon_req.SEQ_number = -1;
						mp->b_datap->db_type = M_PROTO;
						ntp->state |= WACK;
						putnext(q, mp);
					}
					return;
				}
				qreply(q, mp);
				return;

			default:
				putnext(q, mp);
				return;
		}
		break;
		
		case M_DELAY:
			/* tty delays not supported over network at this time */
			freemsg(mp);
			break;

		default:
			putnext(q, mp);
			break;
	}
}
