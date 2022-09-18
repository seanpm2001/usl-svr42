/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:common/lib/nametoaddr/resolv/libsocket/socket/_conn_util.c	1.1.5.2"
#ident  "$Header: _conn_util.c 1.2 91/06/26 $"

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
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989,1990  AT&T.
 *	(c) 1990,1991  UNIX System Laboratories, Inc.
 * 	          All rights reserved.
 *  
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stream.h>
#include <sys/ioctl.h>
#include <sys/stropts.h>
#include <sys/tihdr.h>
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <sys/sockmod.h>
#include <sys/signal.h>

extern int	errno;

/*
 * Snd_conn_req - send connect request message to
 * transport provider
 */
int
_rs__s_snd_conn_req(siptr, call)
	register struct _si_user	*siptr;
	register struct t_call		*call;
{
	register struct T_conn_req	*creq;
	register char			*buf;
	register int			size;
	struct strbuf			ctlbuf;

	buf = siptr->ctlbuf;
	creq = (struct T_conn_req *)buf;
	creq->PRIM_type = T_CONN_REQ;
	creq->DEST_length = call->addr.len;
	creq->DEST_offset = 0;
	creq->OPT_length = call->opt.len;
	creq->OPT_offset = 0;
	size = sizeof (struct T_conn_req);

	if (call->addr.len) {
		_rs__s_aligned_copy(buf, call->addr.len, size,
			call->addr.buf, &creq->DEST_offset);
		size = creq->DEST_offset + creq->DEST_length;
	}
	if (call->opt.len && call->opt.len != -1) {
		_rs__s_aligned_copy(buf, call->opt.len, size,
			call->opt.buf, &creq->OPT_offset);
		size = creq->OPT_offset + creq->OPT_length;
	}

	ctlbuf.maxlen = siptr->ctlsize;
	ctlbuf.len = size;
	ctlbuf.buf = buf;

	if (putmsg(siptr->fd, &ctlbuf, (call->udata.len? &call->udata: NULL),
					0) < 0)
		return (-1);

	if (!_rs__s_is_ok(siptr, T_CONN_REQ))
		return (-1);

	return (0);
}

/*
 * Rcv_conn_con - get connection confirmation off
 * of read queue
 */
int
_rs__s_rcv_conn_con(siptr)
	register struct _si_user	*siptr;
{
	struct strbuf			ctlbuf;
	struct strbuf			databuf;
	register union T_primitives	*pptr;
	register int			retval;
	int				flg;
	char				dbuf[128];

	flg = 0;
	if (siptr->udata.servtype == T_CLTS) {
		errno = EOPNOTSUPP;
		return (-1);
	}
again:
	ctlbuf.maxlen = siptr->ctlsize;
	ctlbuf.len = 0;
	ctlbuf.buf = siptr->ctlbuf;

	databuf.maxlen = sizeof (dbuf);
	databuf.len = 0;
	databuf.buf = dbuf;

	/*
	 * No data expected, but we play safe.
	 */
	if ((retval = getmsg(siptr->fd, &ctlbuf, &databuf, &flg)) < 0) {
		if (errno == ENXIO)
			errno = ECONNREFUSED;
		return (-1);
	}

	/*
	 * did we get entire message
	 */
	if (retval) {
		errno = EIO;
		return (-1);
	}

	/*
	 * is cntl part large enough to determine message type?
	 */
	if (ctlbuf.len < sizeof (long)) {
		errno = EPROTO;
		return (-1);
	}

	pptr = (union T_primitives *)ctlbuf.buf;
	switch (pptr->type) {
		case T_CONN_CON:
			return (0);

		case T_DISCON_IND:
			if (ctlbuf.len < sizeof (struct T_discon_ind))
				errno = ECONNREFUSED;
			else	errno = pptr->discon_ind.DISCON_reason;
			return (-1);

		default:
			break;
	}

	errno = EPROTO;
	return (-1);
}
