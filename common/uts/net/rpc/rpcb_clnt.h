/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-comm:net/rpc/rpcb_clnt.h	1.3.3.6"
#ident	"$Header: $"

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 * rpcb_clnt.h
 * Supplies C routines to get to rpcbid services.
 *
 */

/*
 * Usage:
 *	success = rpcb_set(program, version, nconf, address);
 *	success = rpcb_unset(program, version, nconf);
 *	success = rpcb_getaddr(program, version, nconf, host);
 *	head = rpcb_getmaps(nconf, host);
 *	clnt_stat = rpcb_rmtcall(nconf, host, program, version, procedure,
 *		xdrargs, argsp, xdrres, resp, tout, addr_ptr)
 * 	clnt_stat = rpc_broadcast(program, version, procedure,
 *		xdrargs, argsp,	xdrres, resp, eachresult, nettype)
 *		(like rpcb_rmtcall, except the call is broadcasted to all
 *		locally connected nets.  For each valid response received,
 *		the procedure eachresult is called.  Its form is:
 *		done = eachresult(resp, raddr, netconf)
 *			bool_t done;
 *			caddr_t resp;
 *			struct netbuf *raddr;
 *			struct netconfig *netconf;
 *		where resp points to the results of the call and raddr is the
 *		address if the responder to the broadcast. netconf is the
 *		on which the response came.
 *	success = rpcb_gettime(host, timep)
 *	uaddr = rpcb_taddr2uaddr(nconf, taddr);
 *	taddr = rpcb_uaddr2uaddr(nconf, uaddr);
 */

#ifndef _NET_RPC_RPCB_CLNT_H	/* wrapper symbol for kernel use */
#define _NET_RPC_RPCB_CLNT_H	/* subject to change without notice */

#ifdef _KERNEL_HEADERS

#ifndef _NET_RPC_TYPES_H
#include <net/rpc/types.h>	/* REQUIRED */
#endif

#ifndef _NET_RPC_RPCB_PROT_H
#include <net/rpc/rpcb_prot.h>	/* REQUIRED */
#endif

#elif defined(_KERNEL)

#include <rpc/types.h>		/* REQUIRED */
#include <rpc/rpcb_prot.h>	/* REQUIRED */

#else

#include <rpc/types.h>		/* SVR4.0COMPAT */
#include <rpc/rpcb_prot.h>	/* SVR4.0COMPAT */

#endif /* _KERNEL_HEADERS */

extern bool_t		rpcb_set();
extern bool_t		rpcb_unset();
extern RPCBLIST		*rpcb_getmaps();
extern enum clnt_stat	rpcb_rmtcall();
extern enum clnt_stat	rpc_broadcast();
extern bool_t		rpcb_getaddr();
extern bool_t		rpcb_gettime();
extern char		*rpcb_taddr2uaddr();
extern struct netbuf	*rpcb_uaddr2taddr();

#endif /* _NET_RPC_RPCB_CLNT_H */
