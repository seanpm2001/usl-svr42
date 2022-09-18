/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NET_TCPIP_IP_STR_H	/* wrapper symbol for kernel use */
#define _NET_TCPIP_IP_STR_H	/* subject to change without notice */

#ident	"@(#)uts-comm:net/tcpip/ip_str.h	1.3.3.2"
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
 *  	(c) 1983,1984,1985,1986,1987,1988,1989,1990  AT&T.
 *	(c) 1990,1991 UNIX System Laboratories, Inc.
 *  	          All rights reserved.
 */

/*
 * System V STREAMS TCP - Release 3.0 
 *
 * Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI) 
 * All Rights Reserved. 
 *
 * The copyright above and this notice must be preserved in all copies of this
 * source code.  The copyright above does not evidence any actual or intended
 * publication of this source code. 
 *
 * This is unpublished proprietary trade secret source code of Lachman
 * Associates.  This source code may not be copied, disclosed, distributed,
 * demonstrated or licensed except as expressly authorized by Lachman
 * Associates. 
 *
 * System V STREAMS TCP was jointly developed by Lachman Associates and
 * Convergent Technologies. 
 */

#ifdef _KERNEL_HEADERS

#ifndef _UTIL_TYPES_H
#include <util/types.h>		/* REQUIRED */
#endif

#ifndef _IO_STREAM_H
#include <io/stream.h>		/* REQUIRED */
#endif

#ifndef _NET_TCPIP_IN_H
#include <net/tcpip/in.h>	/* REQUIRED */
#endif

#ifndef _NET_TCPIP_IN_VAR_H
#include <net/tcpip/in_var.h>	/* REQUIRED */
#endif

#ifndef _NET_TCPIP_IF_H
#include <net/tcpip/if.h>	/* REQUIRED */
#endif

#ifndef _NET_TCPIP_ROUTE_H
#include <net/tcpip/route.h>	/* REQUIRED */
#endif

#elif defined(_KERNEL)

#include <sys/types.h>		/* REQUIRED */
#include <sys/stream.h>		/* REQUIRED */
#include <netinet/in.h>		/* REQUIRED */
#include <netinet/in_var.h>	/* REQUIRED */
#include <net/if.h>		/* REQUIRED */
#include <net/route.h>		/* REQUIRED */

#endif /* _KERNEL_HEADERS */

/*
 * Definitions for stream driver control of the Internet Protocol. This
 * module defines the structures related to controlling the streams interface
 * itself, the structures related to various other protocol elements are in
 * other files. 
 */

#define NIP		8	/* Number of minor devices supported */
#define IP_PROVIDERS	16	/* Max Number of link level service providers */
#define IP_SAP		0x800	/* SAP for IP protocol - currently enet type */
#define ARP_SAP		0x806	/* SAP for ARP */

struct ip_pcb {
	queue_t        *ip_rdq;	/* Upper read queue for this client */
	ushort          ip_proto;	/* Client protocol number set with
					 * N_BIND */
	ushort          ip_state;	/* State flags for this client, see
					 * below */
};

#define IPOPEN	1		/* Minor device open when set */

struct ip_provider {		/* The description of each link service */
	char            name[IFNAMSIZ];	/* provider name (e.g., emd1) */
	queue_t        *qbot;		/* lower write queue */
	queue_t        *qtop;		/* upper write queue for link/unlink */
	mblk_t         *linkbp;		/* iocblk for linke/unlink */
	int             l_index;	/* unique ID of lower stream */
	int             if_flags;	/* up/down, broadcast, etc. */
	int             if_metric;	/* routing metric (external only) */
	int             if_maxtu;	/* maximum transmission unit */
	int             if_mintu;	/* minimum transmission unit */
	struct in_ifaddr ia;	/* address chain structure maintained by if */

#define SOCK_INADDR(sock) (&(((struct sockaddr_in *)(sock))->sin_addr))
#define PROV_INADDR(prov) SOCK_INADDR(&((prov)->ia.ia_ifa.ifa_addr))

	/* The following defines are vestiges of */
	/* the socket based ip implementation */
#define if_addr		ia.ia_ifa.ifa_addr
						/* interface address */
#define	if_broadaddr	ia.ia_broadaddr
						/* broadcast address */
#define	if_dstaddr	ia.ia_dstaddr
						/* other end of p-to-p link */

	mblk_t         *unswitch;	/* ioctl pointer for switched routes */
};

/*
 * A special version of the unitdata request to be sent down through ip -> it
 * contains various ip specific extensions to the base structure 
 */

struct ip_unitdata_req {
	ulong		dl_primitive;		/* always N_UNITDATA_REQ */
	ulong		dl_dest_addr_length;	/* dest NSAP addr length - 
						 * 4 for ip */
	ulong		dl_dest_addr_offset;	/* dest NSAP addr offset */
	ulong		dl_reserved[2];
	mblk_t		*options;		/* options for ip */
	struct route	route;			/* route for packet to follow */
	int		flags;
	struct in_addr	ip_addr;		/* the ip destination addr */
};


/*
 * ip_ctlmsg is the structure used to send control messages to the client
 * upper level protocols.  These messages are actually generated by icmp and
 * are passed down to ip to distribute among the clients. 
 */

struct ip_ctlmsg {
	int             command;
	struct in_addr  src_addr;
	struct in_addr  dst_addr;
	u_short		src_port;
	u_short		dst_port;
	int             proto;
};

#endif	/* _NET_TCPIP_IP_STR_H */
