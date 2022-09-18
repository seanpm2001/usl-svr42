/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)lp:include/xdrMsgs.h	1.4.1.3"
#ident	"$Header: $"
/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#include <rpc/types.h>

#define	MSGS_VERSION_MAJOR	1
#define	MSGS_VERSION_MINOR	2

enum networkMsgType {
	JobControlMsg = 0,
	SystemIdMsg = 1,
	DataPacketMsg = 2,
	FileFragmentMsg = 3,
	PacketBundleMsg = 4,
};
typedef enum networkMsgType networkMsgType;
bool_t xdr_networkMsgType();

enum jobControlCode {
	NormalJobMsg = 0,
	RequestToSendJob = 1,
	ClearToSendJob = 2,
	AbortJob = 3,
	JobAborted = 4,
	RequestDenied = 5,
};
typedef enum jobControlCode jobControlCode;
bool_t xdr_jobControlCode();

struct routingControl {
	u_int sysId;
	u_int msgId;
};
typedef struct routingControl routingControl;
bool_t xdr_routingControl();

struct jobControl_1_1 {
	u_char controlCode;
	u_char priority;
	u_char endOfJob;
	u_int jobId;
	long timeStamp;
};
typedef struct jobControl_1_1 jobControl_1_1;
bool_t xdr_jobControl_1_1();

struct jobControlMsg {
	u_char controlCode;
	u_char priority;
	u_char endOfJob;
	u_int jobId;
	long timeStamp;
	u_int uid;
	u_int gid;
	u_int lid;
	u_int mode;
	char *ownerp;
};
typedef struct jobControlMsg jobControlMsg;
bool_t xdr_jobControlMsg();

struct networkMsgTag_1_1 {
	u_char versionMajor;
	u_char versionMinor;
	struct routingControl routeControl;
	networkMsgType msgType;
	struct jobControl_1_1 *jobControlp;
};
typedef struct networkMsgTag_1_1 networkMsgTag_1_1;
bool_t xdr_networkMsgTag_1_1();

struct networkMsgTag_1_2 {
	u_char versionMajor;
	u_char versionMinor;
	networkMsgType msgType;
	struct routingControl routeControl;
};
typedef struct networkMsgTag_1_2 networkMsgTag_1_2;
bool_t xdr_networkMsgTag_1_2();

struct systemIdMsg {
	char *systemNamep;
	struct {
		u_int data_len;
		char *data_val;
	} data;
};
typedef struct systemIdMsg systemIdMsg;
bool_t xdr_systemIdMsg();

struct dataPacketMsg {
	int endOfPacket;
	struct {
		u_int data_len;
		char *data_val;
	} data;
};
typedef struct dataPacketMsg dataPacketMsg;
bool_t xdr_dataPacketMsg();

struct packetBundleMsg {
	struct {
		u_int packets_len;
		struct dataPacketMsg *packets_val;
	} packets;
};
typedef struct packetBundleMsg packetBundleMsg;
bool_t xdr_packetBundleMsg();

struct fileFragmentMsg_1_1 {
	int endOfFile;
	long sizeOfFile;
	char *destPathp;
	struct {
		u_int fragment_len;
		char *fragment_val;
	} fragment;
};
typedef struct fileFragmentMsg_1_1 fileFragmentMsg_1_1;
bool_t xdr_fileFragmentMsg_1_1();

struct fileFragmentMsg_1_2 {
	int endOfFile;
	u_long uid;
	u_long gid;
	u_long lid;
	u_long mode;
	u_long sizeOfFile;
	char *ownerp;
	char *destPathp;
	struct {
		u_int fragment_len;
		char *fragment_val;
	} fragment;
};
typedef struct fileFragmentMsg_1_2 fileFragmentMsg_1_2;
bool_t xdr_fileFragmentMsg_1_2();