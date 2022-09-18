/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)csh:common/cmd/csh/signal.c	1.8.6.5"
#ident  "$Header: signal.c 1.2 91/06/26 $"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * 4.3BSD signal compatibility functions
 *
 * the implementation interprets signal masks equal to -1 as "all of the
 * signals in the signal set", thereby allowing signals with numbers
 * above 32 to be blocked when referenced in code such as:
 *
 *	for (i = 0; i < NSIG; i++)
 *		mask |= sigmask(i)
 */

#include <sys/types.h>
#include <sys/siginfo.h>

#include <sys/tss.h>
#include <sys/user.h>
#include <sys/reg.h>

#include <sys/ucontext.h>
#include <signal.h>
#include "signal.h"
#include <errno.h>

#define set2mask(setp) ((setp)->sa_sigbits[0])
#define mask2set(mask, setp) \
	((mask) == -1 ? sigfillset(setp) : (((setp)->sa_sigbits[0]) = (mask)))

extern void (*_siguhandler[])();

/*
 * sigstack is emulated with sigaltstack by guessing an appropriate
 * value for the stack size - on machines that have stacks that grow 
 * upwards, the ss_sp arguments for both functions mean the same thing, 
 * (the initial stack pointer sigstack() is also the stack base 
 * sigaltstack()), so a "very large" value should be chosen for the 
 * stack size - on machines that have stacks that grow downwards, the
 * ss_sp arguments mean opposite things, so 0 should be used (hopefully
 * these machines don't have hardware stack bounds registers that pay
 * attention to sigaltstack()'s size argument.
 */

stackgrowth()
{
	int i, j;

	return (&i - &j);
}

/*
 * sigvechandler is the real signal handler installed for all
 * signals handled in the 4.3BSD compatibility interface - it translates
 * SVR4 signal hander arguments into 4.3BSD signal handler arguments
 * and then calls the real handler
 *
 * Note: pre-k13 version had 4 arguments. The extra argument was mainly
 * 	a place holder
 */

static void
sigvechandler(sig, sip, ucp) 
	int sig;
	siginfo_t *sip;
	ucontext_t *ucp;
{
	struct sigcontext sc;
	int code;
	char *addr;
	
	sc.sc_onstack = ((ucp->uc_stack.ss_flags & SS_ONSTACK) != 0);
	sc.sc_mask = set2mask(&ucp->uc_sigmask);

	sc.sc_esp = (int)(ucp->uc_mcontext.gregs[ESP]);
	sc.sc_eip = (int)(ucp->uc_mcontext.gregs[EIP]);
	sc.sc_ebp = (int)(ucp->uc_mcontext.gregs[EBP]);
	sc.sc_eax = (int)(ucp->uc_mcontext.gregs[EAX]);
	sc.sc_edx = (int)(ucp->uc_mcontext.gregs[EDX]);

	if ((code = sip->si_code) == BUS_OBJERR)
		code = SEGV_MAKE_ERR(sip->si_errno);

	if (sig == SIGILL || sig == SIGFPE || sig == SIGSEGV || sig == SIGBUS)
		addr = (char *)sip->si_addr;
	else
		addr = SIG_NOADDR;
	
	(*_siguhandler[sig])(sig, code, &sc, addr);

	if (sc.sc_onstack)
		ucp->uc_stack.ss_flags |= SS_ONSTACK;
	else
		ucp->uc_stack.ss_flags &= ~SS_ONSTACK;
	mask2set(sc.sc_mask, &ucp->uc_sigmask);


	setcontext(ucp);
}

sigsetmask(mask)
	int mask;
{
	sigset_t oset;
	sigset_t nset;

	(void) sigprocmask(0, (sigset_t *)0, &nset);
	mask2set(mask, &nset);
	(void) sigprocmask(SIG_SETMASK, &nset, &oset);
	return set2mask(&oset);
}

sigblock(mask)
	int mask;
{
	sigset_t oset;
	sigset_t nset;

	(void) sigprocmask(0, (sigset_t *)0, &nset);
	mask2set(mask, &nset);
	(void) sigprocmask(SIG_BLOCK, &nset, &oset);
	return set2mask(&oset);
}

#ifdef __STDC__
sigpause(mask)
#else
ucbsigpause(mask)
#endif
	int mask;
{
	sigset_t set;

	(void) sigprocmask(0, (sigset_t *)0, &set);
	mask2set(mask, &set);
	return (sigsuspend(&set));
}

sigvec(sig, nvec, ovec)
        int sig;
        struct sigvec *nvec;
	struct sigvec *ovec;
{
        struct sigaction nact;
        struct sigaction oact;
        struct sigaction *nactp;
        void (*ohandler)(), (*nhandler)();

        if (sig <= 0 || sig >= NSIG) {
                errno = EINVAL;
                return -1;
        }

        ohandler = _siguhandler[sig];

        if (nvec) {
		__sigaction(sig, (struct sigaction *)0, &nact);
                nhandler = nvec->sv_handler; 
                _siguhandler[sig] = nhandler;
                if (nhandler != SIG_DFL && nhandler != SIG_IGN)
                        nact.sa_handler = (void (*)())sigvechandler;
		else
			nact.sa_handler = nhandler;
		mask2set(nvec->sv_mask, &nact.sa_mask);
		if ( sig == SIGTSTP || sig == SIGSTOP )
			nact.sa_handler = SIG_DFL; 
		nact.sa_flags = SA_SIGINFO;
		if (!(nvec->sv_flags & SV_INTERRUPT))
			nact.sa_flags |= SA_RESTART;
		if (nvec->sv_flags & SV_RESETHAND)
			nact.sa_flags |= SA_RESETHAND;
		if (nvec->sv_flags & SV_ONSTACK)
			nact.sa_flags |= SA_ONSTACK;
		nactp = &nact;
        } else
		nactp = (struct sigaction *)0;

        if (__sigaction(sig, nactp, &oact) < 0) {
                _siguhandler[sig] = ohandler;
                return -1;
        }

        if (ovec) {
		if (oact.sa_handler == SIG_DFL || oact.sa_handler == SIG_IGN)
			ovec->sv_handler = oact.sa_handler;
		else
			ovec->sv_handler = ohandler;
		ovec->sv_mask = set2mask(&oact.sa_mask);
		ovec->sv_flags = 0;
		if (oact.sa_flags & SA_ONSTACK)
			ovec->sv_flags |= SV_ONSTACK;
		if (oact.sa_flags & SA_RESETHAND)
			ovec->sv_flags |= SV_RESETHAND;
		if (!(oact.sa_flags & SA_RESTART))
			ovec->sv_flags |= SV_INTERRUPT;
	}
			
        return 0;
}

sigstack(nss, oss)
	struct sigstack *nss;
	struct sigstack *oss;
{
	struct sigaltstack nalt;
	struct sigaltstack oalt;
	struct sigaltstack *naltp;

	if (nss) {
		if (nss->ss_onstack)
			nalt.ss_flags = SS_ONSTACK;
		else
			nalt.ss_flags = 0;
		nalt.ss_sp = (char *)nss->ss_sp;
		if (stackgrowth() > 0)
			nalt.ss_size = 0;
		else
			nalt.ss_size = 1000000;
		naltp = &nalt;
	}  else
		naltp = (struct sigaltstack *)0;

	if (sigaltstack(naltp, &oalt) < 0)
		return -1;

	if (oss) {
		oss->ss_onstack = ((oalt.ss_flags & SS_ONSTACK) != 0);
		oss->ss_sp = (char *)oalt.ss_sp;
	}

	return 0;
}


void (*
signal(s, a))()
        int s;
        void (*a)();
{
        struct sigvec osv;
	struct sigvec nsv;
        static int mask[NSIG];
        static int flags[NSIG];

	nsv.sv_handler = a;
	nsv.sv_mask = mask[s];
	nsv.sv_flags = flags[s];
        if (sigvec(s, &nsv, &osv) < 0)
                return (SIG_ERR);
        if (nsv.sv_mask != osv.sv_mask || nsv.sv_flags != osv.sv_flags) {
                mask[s] = nsv.sv_mask = osv.sv_mask;
                flags[s] = nsv.sv_flags = osv.sv_flags & ~SV_RESETHAND;
                if (sigvec(s, &nsv, (struct sigvec *)0) < 0)
                        return (SIG_ERR);
        }
        return (osv.sv_handler);
}
/*
 * Set signal state to prevent restart of system calls
 * after an instance of the indicated signal.
 */
siginterrupt(sig, flag)
        int sig, flag;
{
        struct sigvec sv;
        int ret;

        if ((ret = sigvec(sig, 0, &sv)) < 0)
                return (ret);
        if (flag)
                sv.sv_flags |= SV_INTERRUPT;
        else
                sv.sv_flags &= ~SV_INTERRUPT;
        return (sigvec(sig, &sv, 0));
}