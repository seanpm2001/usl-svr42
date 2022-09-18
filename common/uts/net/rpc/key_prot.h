/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _NET_RPC_KEY_PROT_H	/* wrapper symbol for kernel use */
#define _NET_RPC_KEY_PROT_H	/* subject to change without notice */

#ident	"@(#)uts-comm:net/rpc/key_prot.h	1.3.2.4"
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
 * Compiled from key_prot.x using rpcgen.
 * DO NOT EDIT THIS FILE!
 * This is NOT source code!
 */

#ifdef _KERNEL_HEADERS

#ifndef _UTIL_TYPES_H
#include <util/types.h>		/* REQUIRED */
#endif

#ifndef _NET_RPC_AUTH_H
#include <net/rpc/auth.h>	/* REQUIRED */
#endif

#elif defined(_KERNEL)

#include <sys/types.h>		/* REQUIRED */
#include <rpc/auth.h>		/* REQUIRED */

#endif /* _KERNEL_HEADERS */

#define KEY_PROG 100029
#define KEY_VERS 1
#define KEY_SET 1
#define KEY_ENCRYPT 2
#define KEY_DECRYPT 3
#define KEY_GEN 4
#define KEY_GETCRED 5

#define PROOT 3
#define HEXMODULUS "d4a0ba0250b6fd2ec626e7efd637df76c716e22d0944b88b"
#define HEXKEYBYTES 48
#define KEYSIZE 192
#define KEYBYTES 24
#define KEYCHECKSUMSIZE 16

enum keystatus {
	KEY_SUCCESS = 0,
	KEY_NOSECRET = 1,
	KEY_UNKNOWN = 2,
	KEY_SYSTEMERR = 3
};
typedef enum keystatus keystatus;
bool_t xdr_keystatus();

#ifndef _KERNEL

typedef char keybuf[HEXKEYBYTES];
bool_t xdr_keybuf();

#endif

typedef char *netnamestr;
bool_t xdr_netnamestr();


struct cryptkeyarg {
	netnamestr remotename;
	des_block deskey;
};
typedef struct cryptkeyarg cryptkeyarg;
bool_t xdr_cryptkeyarg();


struct cryptkeyres {
	keystatus status;
	union {
		des_block deskey;
	} cryptkeyres_u;
};
typedef struct cryptkeyres cryptkeyres;
bool_t xdr_cryptkeyres();

#define MAXGIDS 16

struct unixcred {
	uid_t uid;
	uid_t gid;
	struct {
		u_int gids_len;
		u_int *gids_val;
	} gids;
};
typedef struct unixcred unixcred;
bool_t xdr_unixcred();


struct getcredres {
	keystatus status;
	union {
		unixcred cred;
	} getcredres_u;
};
typedef struct getcredres getcredres;
bool_t xdr_getcredres();

#endif /* _NET_RPC_KEY_PROT_H */
