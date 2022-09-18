/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uts-comm:net/lockmgr/klm_kprot.c	1.2.2.1"
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

#include <net/rpc/rpc.h>
#include <net/lockmgr/klm_prot.h>

bool_t
xdr_klm_stats(xdrs, objp)
	XDR *xdrs;
	klm_stats *objp;
{
	if (!xdr_enum(xdrs, (enum_t *)objp)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_klm_lock(xdrs, objp)
	XDR *xdrs;
	klm_lock *objp;
{
	if (!xdr_string(xdrs, &objp->server_name, LM_MAXSTRLEN)) {
		return (FALSE);
	}
	if (!xdr_netobj(xdrs, &objp->fh)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->base)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->length)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->type)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->granted)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->color)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->LockID)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->pid)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->class)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->rsys)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->rpid)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_klm_holder(xdrs, objp)
	XDR *xdrs;
	klm_holder *objp;
{
	if (!xdr_bool(xdrs, &objp->exclusive)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->base)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->length)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->type)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->granted)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->color)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->LockID)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->pid)) {
		return (FALSE);
	}
	if (!xdr_int(xdrs, &objp->class)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->rsys)) {
		return (FALSE);
	}
	if (!xdr_long(xdrs, &objp->rpid)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_klm_stat(xdrs, objp)
	XDR *xdrs;
	klm_stat *objp;
{
	if (!xdr_klm_stats(xdrs, &objp->stat)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_klm_testrply(xdrs, objp)
	XDR *xdrs;
	klm_testrply *objp;
{
	if (!xdr_klm_stats(xdrs, &objp->stat)) {
		return (FALSE);
	}
	switch (objp->stat) {
	case klm_denied:
		if (!xdr_klm_holder(xdrs, &objp->klm_testrply_u.holder)) {
			return (FALSE);
		}
		break;
	}
	return (TRUE);
}




bool_t
xdr_klm_lockargs(xdrs, objp)
	XDR *xdrs;
	klm_lockargs *objp;
{
	if (!xdr_bool(xdrs, &objp->block)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &objp->exclusive)) {
		return (FALSE);
	}
	if (!xdr_klm_lock(xdrs, &objp->alock)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_klm_testargs(xdrs, objp)
	XDR *xdrs;
	klm_testargs *objp;
{
	if (!xdr_bool(xdrs, &objp->exclusive)) {
		return (FALSE);
	}
	if (!xdr_klm_lock(xdrs, &objp->alock)) {
		return (FALSE);
	}
	return (TRUE);
}




bool_t
xdr_klm_unlockargs(xdrs, objp)
	XDR *xdrs;
	klm_unlockargs *objp;
{
	if (!xdr_klm_lock(xdrs, &objp->alock)) {
		return (FALSE);
	}
	return (TRUE);
}


