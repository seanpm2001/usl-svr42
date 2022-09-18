

/*****************************************************
 *
 * SVR 4.2 STREAMS YHTP - Release 1.0 
 *
 * Copyright 1994 NET612 Computer Department of NUDT 
 * All Rights Reserved. 
 *
 ****************************************************/

#ifndef	_NET_YHTPIP_YHIN_H	/* wrapper symbol for kernel use */
#define	_NET_YHTPIP_YHIN_H	/* subject to change without notice */



/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981.
 */
#ifdef _KERNEL_HEADERS

#ifndef _UTIL_TYPES_H
#include <util/types.h>			/* REQUIRED */
#endif

#ifndef _IO_STREAM_H
#include <io/stream.h>			/* REQUIRED */
#endif

/* for network <--> host byteorder conversion */
/* macros: ntohl, ntohs, htonl, htons	      */
#ifndef _NET_YHTPIP_BYTEORDER_H
#include <net/yhtpip/byteorder.h>	/* REQUIRED */
#endif

#ifndef _NET_TRANSPORT_SOCKET_H
#include <net/transport/socket.h>	/* REQUIRED */
#endif

#elif defined(_KERNEL)

#include <sys/types.h>			/* REQUIRED */
#include <sys/stream.h>			/* REQUIRED */
#include <sys/byteorder.h>		/* REQUIRED */
#include <sys/socket.h>			/* REQUIRED */

#else

#include <sys/stream.h>			/* SVR4.0COMPAT */
#include <sys/byteorder.h>		/* SVR4.0COMPAT */

#endif /* _KERNEL_HEADERS */

#define IPM_ID		200	/* Module ID for IP stream */
#define ICMPM_ID	201	/* Module ID for ICMP stream */
#define TCPM_ID		202	/* Module ID for TCP stream */
#define UDPM_ID		203	/* Module ID for UDP stream */
#define ARPM_ID		204	/* Module ID for ARP stream */
#define APPM_ID		205	/* Module ID for ProcARP stream */
#define RIPM_ID         206     /* Module ID for RIP stream */
#define YHTPM_ID 	207     /* Module ID for YHTP stream */
#define YHIPM_ID	208	/* Module ID for YHIP stream */
#define YHARPM_ID	209	/* Module ID for YHARP stream */
#define YHAPPM_ID	210 	/* Module ID for YHAPP stream */

/*
 * Protocols
 */
#define	IPPROTO_IP		0		/* dummy for IP */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define IPPROTO_IGMP		2		/* group control protocol */
#define	IPPROTO_GGP		3		/* gateway^2 (deprecated) */
#define IPPROTO_YHIP		4		/* yhip */
#define	IPPROTO_TCP		6		/* tcp */
#define IPPROTO_YHTP		7	 	/* yhtp */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_IDP		22		/* xns idp */
#define IPPROTO_HELLO		63		/* "hello" routing protocol */
#define	IPPROTO_ND		77		/* UNOFFICIAL net disk proto */

#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256

/*
 * Port/socket numbers: network standard functions
 */
#define	IPPORT_ECHO		7
#define	IPPORT_DISCARD		9
#define	IPPORT_SYSTAT		11
#define	IPPORT_DAYTIME		13
#define	IPPORT_NETSTAT		15
#define	IPPORT_FTP		21
#define	IPPORT_TELNET		23
#define	IPPORT_SMTP		25
#define	IPPORT_TIMESERVER	37
#define	IPPORT_NAMESERVER	42
#define	IPPORT_WHOIS		43
#define	IPPORT_MTP		57

/*
 * Port/socket numbers: host specific functions
 */
#define	IPPORT_TFTP		69
#define	IPPORT_RJE		77
#define	IPPORT_FINGER		79
#define	IPPORT_TTYLINK		87
#define	IPPORT_SUPDUP		95

/*
 * UNIX TCP sockets
 */
#define	IPPORT_EXECSERVER	512
#define	IPPORT_LOGINSERVER	513
#define	IPPORT_CMDSERVER	514
#define	IPPORT_EFSSERVER	520

/*
 * UNIX UDP sockets
 */
#define	IPPORT_BIFFUDP		512
#define	IPPORT_WHOSERVER	513
#define	IPPORT_ROUTESERVER	520	/* 520+1 also used */

/*
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.
 */
#define	IPPORT_RESERVED		1024
#define	IPPORT_USERRESERVED	5000

/*
 * Link numbers
 */
#define	IMPLINK_IP		155
#define	IMPLINK_LOWEXPER	156
#define	IMPLINK_HIGHEXPER	158

/*
 * Internet address
 *	This definition contains obsolete fields for compatibility
 *	with SunOS 3.x and 4.2bsd.  The presence of subnets renders
 *	divisions into fixed fields misleading at best.  New code
 *	should use only the s_addr field.
 */
struct in_addr {
	union {
		struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
		struct { u_short s_w1,s_w2; } S_un_w;
		u_long S_addr;
	} S_un;
#define	s_addr	S_un.S_addr		/* should be used for all code */
#define	s_host	S_un.S_un_b.s_b2	/* OBSOLETE: host on imp */
#define	s_net	S_un.S_un_b.s_b1	/* OBSOLETE: network */
#define	s_imp	S_un.S_un_w.s_w2	/* OBSOLETE: imp */
#define	s_impno	S_un.S_un_b.s_b4	/* OBSOLETE: imp # */
#define	s_lh	S_un.S_un_b.s_b3	/* OBSOLETE: logical host */
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define	IN_CLASSA(i)		(((long)(i) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		0x00ffffff
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((long)(i) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		0x0000ffff
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((long)(i) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ff

#define	IN_CLASSD(i)		(((long)(i) & 0xf0000000) == 0xe0000000)
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	IN_EXPERIMENTAL(i)	(((long)(i) & 0xe0000000) == 0xe0000000)
#define	IN_BADCLASS(i)		(((long)(i) & 0xf0000000) == 0xf0000000)

#define	INADDR_ANY		(u_long)0x00000000
#define	INADDR_LOOPBACK		(u_long)0x7F000001
#define	INADDR_BROADCAST	(u_long)0xffffffff	/* must be masked */

#define	IN_LOOPBACKNET		127			/* official! */

/*
 * Define a macro to stuff the loopback address into an Internet address
 */
#define IN_SET_LOOPBACK_ADDR(a)	{(a)->sin_addr.s_addr  = htonl(INADDR_LOOPBACK); \
	(a)->sin_family = AF_OSI;}

/*
 * Socket address, internet style.
 */
struct sockaddr_in {
	short	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};

/*
 * The transport providers allow any address length between
 * IN_MINADDRLEN and IN_MAXADDRLEN.  The minimum length corresponds to
 * a sockaddr_in without the sin_zero field.  The maximum length is
 * the size of the sockaddr_in structure.
 *
 * in_chkaddrlen returns true if the given length is valid.
 */

#define IN_MINADDRLEN	(sizeof(struct sockaddr_in) - 8)
#define IN_MAXADDRLEN	(sizeof(struct sockaddr_in))
#define in_chkaddrlen(x) ((x) >= IN_MINADDRLEN && (x) <= IN_MAXADDRLEN)

/*
 * Options for use with [gs]etsockopt at the IP level.
 */
#define	IP_OPTIONS	1		/* set/get IP per-packet options */


#ifdef	_KERNEL
extern	struct domain yhinetdomain;
extern	struct protosw yhinetsw[];
struct	in_addr yhin_makeaddr();
u_long	yhin_netof(), yhin_lnaof();

struct iocblk_in {
	struct iocblk   iocblk;
	queue_t        *ioc_transport_client;
	queue_t        *ioc_network_client;
	int             ioc_ifflags;
};

/*
 * Useful defines, should be in stream.h 
 */
#define msgbavail(bp) ((bp)->b_datap->db_lim - (bp)->b_wptr)
#define msgblen(bp)	((bp)->b_wptr - (bp)->b_rptr)
#endif	/* _KERNEL */


#endif	/* _NET_YHTPIP_YHIN_H */
