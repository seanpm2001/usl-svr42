%{
/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
%}
%{
/*	Copyright (c) 1988, 1989, 1990 AT&T	*/
%}
%{
/*	  All Rights Reserved  	*/
%}

%{
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
%}
%{
/*	UNIX System Laboratories, Inc.                     	*/
%}
%{
/*	The copyright notice above does not evidence any   	*/
%}
%{
/*	actual or intended publication of such source code.	*/
%}

#ident	"@(#)r5twm:lex.l	1.1"
%{
/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/*****************************************************************************/

/***********************************************************************
 *
 * $XConsortium: lex.l,v 1.62 89/12/10 17:46:33 jim Exp $
 *
 * .twmrc lex file
 *
 * 12-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

/* #include <stdio.h> */		/* lex already includes stdio.h */
#include "gram.h"
#include "parse.h"
extern char *ProgramName;

extern int ParseError;

%}

string				\"([^"]|\\.)*\"
number				[0-9]+
%%
"{"				{ return (LB); }
"}"				{ return (RB); }
"("				{ return (LP); }
")"				{ return (RP); }
"="				{ return (EQUALS); }
":"				{ return (COLON); }
"+"				{ return PLUS; }
"-"				{ return MINUS; }
"|"				{ return OR; }

[a-zA-Z\.]+			{ int token = parse_keyword (yytext, 
							     &yylval.num);
				  if (token == ERRORTOKEN) {
				      twmrc_error_prefix();
				      fprintf (stderr,
				       "ignoring unknown keyword:  %s\n", 
					       yytext);
				      ParseError = 1;
				  } else 
				    return token;
				}

"!"				{ yylval.num = F_EXEC; return FSKEYWORD; }
"^"				{ yylval.num = F_CUT; return FSKEYWORD; }

{string}			{ yylval.ptr = (char *)yytext; return STRING; }
{number}			{ (void)sscanf(yytext, "%d", &yylval.num);
				  return (NUMBER);
				}
\#[^\n]*\n			{;}
[\n\t ]				{;}
.				{
				  twmrc_error_prefix();
				  fprintf (stderr, 
					   "ignoring character \"%s\"\n",
					   yytext);
				  ParseError = 1;
				}
%%
yywrap() { return(1);}

#undef unput
#undef input
#undef output
#undef feof
#define unput(c)	twmUnput(c)
#define input()		(*twmInputFunc)()
#define output(c)	TwmOutput(c)
#define feof()		(1)