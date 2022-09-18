/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_Message_h
#define	_Message_h
#ident	"@(#)debugger:inc/common/Message.h	1.1"

#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "Msgtypes.h"
#include "Severity.h"
#include "Signature.h"
#include "Transport.h"

enum Msg_state
{
	MSTATE_empty = 0,
	MSTATE_ready_to_send,
	MSTATE_sent,
	MSTATE_received
};

// bundle sets msg_*; transport_type, sequence_num, dbcontext, and
// uicontext are set by transport when the message is sent
//
// msg_data should always be the last data member; if pipes are being
// used for the transport mechanism, everything up to msg_data is
// written to the pipe, and the data it points to is also written,
// but the pointer itself is meaningless on the other side and is
// not written to the pipe

class Message
{
	Transport_type	transport_type;
	int		sequence_num;
	DBcontext	dbcontext;
	UIcontext	uicontext;
	Msg_id		msg_id;
	Msg_state	msg_state;
	Severity	msg_severity;
	size_t		msg_length;
	void		*msg_data;
	
			// functions to get the individual pieces of msg_data
			// strings (char *'s) and Words are the only kinds
			// of data that can be sent in a message
	void		pick_string(char *&in, char *&out);
	void		pick_word(char *&in, Word &out);

	friend class	Transport;
public:
			Message()	{ memset(this, 0, sizeof(Message)); }
			~Message()		{ delete msg_data; }
	Message&	operator=(Message&);

	void		clear();	// return to empty state

			// access functions
#ifndef NOCHECKS
	DBcontext	get_dbcontext();
	UIcontext	get_uicontext();
	Msg_id		get_msg_id();
	Severity	get_severity();
	Transport_type	get_transport_type();
	size_t		get_msg_length();
#else
	DBcontext	get_dbcontext()		{ return dbcontext; }
	UIcontext	get_uicontext()		{ return uicontext; }
	Msg_id		get_msg_id()		{ return msg_id; }
	Severity	get_severity()		{ return severity; }
	Transport_type	get_transport_type()	{ return transport_type; }
	size_t		get_msg_length()	{ return msg_length; }
#endif // NOCHECKS

			// function to create messages
	void		bundle(Msg_id, Severity, va_list);
	
			// output formatting
	char		*format();

	// the message decomposition functions are declared in Unbundle.h
	// that file is created by an awk script from Signature.h
	// The functions look like:
	// void unbundle(char *&); void unbundle(Word &, char *&); etc.
	// unbundle provides the space for the unpacked strings,
	// but the space may be overwritten, so the calling function
	// should make a copy if it needs to save the string

	#include "Unbundle.h"
};

#endif	// _Message_h
