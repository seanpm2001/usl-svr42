/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)r5extensions:server/xinput/xsetbmap.c	1.1"
/* $XConsortium: xsetbmap.c,v 1.6 89/12/02 15:21:33 rws Exp $ */

/************************************************************
Copyright (c) 1989 by Hewlett-Packard Company, Palo Alto, California, and the 
Massachusetts Institute of Technology, Cambridge, Massachusetts.

			All Rights Reserved



********************************************************/

/***********************************************************************
 *
 * Request to change the button mapping of an extension device.
 *
 */

#define	 NEED_EVENTS
#define	 NEED_REPLIES
#define IsOn(ptr, bit) \
	(((BYTE *) (ptr))[(bit)>>3] & (1 << ((bit) & 7)))

#include "X.h"				/* for inputstr.h    */
#include "Xproto.h"			/* Request macro     */
#include "inputstr.h"			/* DeviceIntPtr	     */
#include "XI.h"
#include "XIproto.h"

extern	int 		IReqCode;
extern	int 		BadDevice;
extern	int 		DeviceMappingNotify;
extern	void		(* ReplySwapVector[256]) ();
DeviceIntPtr		LookupDeviceIntRec();

/***********************************************************************
 *
 * This procedure changes the button mapping.
 *
 */

int
SProcXSetDeviceButtonMapping(client)
    register ClientPtr client;
    {
    register char n;

    REQUEST(xSetDeviceButtonMappingReq);
    swaps(&stuff->length, n);
    return(ProcXSetDeviceButtonMapping(client));
    }

/***********************************************************************
 *
 * This procedure lists the input devices available to the server.
 *
 */

ProcXSetDeviceButtonMapping (client)
    register ClientPtr client;
    {
    int					ret;
    xSetDeviceButtonMappingReply	rep;
    DeviceIntPtr dev;

    REQUEST(xSetDeviceButtonMappingReq);
    REQUEST_AT_LEAST_SIZE(xSetDeviceButtonMappingReq);

    if (stuff->length != (sizeof(xSetDeviceButtonMappingReq) + 
	stuff->map_length + 3)>>2)
	{
	SendErrorToClient(client, IReqCode, X_SetDeviceButtonMapping, 0, 
		BadLength);
	return Success;
	}

    rep.repType = X_Reply;
    rep.RepType = X_SetDeviceButtonMapping;
    rep.length = 0;
    rep.sequenceNumber = client->sequence;
    rep.status = MappingSuccess;

    dev = LookupDeviceIntRec (stuff->deviceid);
    if (dev == NULL)
	{
	SendErrorToClient(client, IReqCode, X_SetDeviceButtonMapping, 0, 
		BadDevice);
	return Success;
	}

    ret = SetButtonMapping (client, dev, stuff->map_length, &stuff[1]);

    if (ret == BadValue || ret == BadMatch)
	{
	SendErrorToClient(client, IReqCode, X_SetDeviceButtonMapping, 0, 
		ret);
	return Success;
	}
    else
	{
	rep.status = ret;
	WriteReplyToClient(client, sizeof(xSetDeviceButtonMappingReply), &rep);
	}

    if (ret != MappingBusy)
        SendDeviceMappingNotify(MappingPointer, 0, 0, dev);
    return Success;
    }

/***********************************************************************
 *
 * This procedure writes the reply for the XSetDeviceButtonMapping function,
 * if the client and server have a different byte ordering.
 *
 */

SRepXSetDeviceButtonMapping (client, size, rep)
    ClientPtr	client;
    int		size;
    xSetDeviceButtonMappingReply	*rep;
    {
    register char n;

    swaps(&rep->sequenceNumber, n);
    swapl(&rep->length, n);
    WriteToClient(client, size, rep);
    }