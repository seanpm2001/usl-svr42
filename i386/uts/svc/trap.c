/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

#ident	"@(#)uts-x86:svc/trap.c	1.19"
#ident	"$Header: $"

#include <acc/audit/audit.h>
#include <acc/mac/covert.h>
#include <fs/procfs/prsystm.h>
#include <fs/vnode.h>
#include <mem/as.h>
#include <mem/faultcatch.h>
#include <mem/faultcode.h>
#include <mem/immu.h>
#include <mem/immu.h>
#include <mem/kmem.h>
#include <mem/seg.h>
#include <mem/seg.h>
#include <mem/seg_kmem.h>
#include <mem/seg_vn.h>
#include <mem/vmparam.h>
#include <mem/vmsystm.h>
#include <proc/class.h>
#include <proc/cred.h>
#include <proc/disp.h>
#include <proc/mman.h>
#include <proc/priocntl.h>
#include <proc/proc.h>
#include <proc/reg.h>
#include <proc/siginfo.h>
#include <proc/signal.h>
#include <proc/tss.h>
#include <proc/ucontext.h>
#include <proc/user.h>
#include <svc/errno.h>
#include <svc/hrtcntl.h>
#include <svc/pic.h>
#include <svc/syscall.h>
#include <svc/systm.h>
#include <svc/trap.h>
#include <svc/utsname.h>
#include <util/cmn_err.h>
#include <util/debug.h>
#include <util/debugreg.h>
#include <util/fault.h>
#include <util/param.h>
#include <util/spl.h>
#include <util/sysinfo.h>
#include <util/sysmacros.h>
#include <util/types.h>
#include <util/var.h>

#ifdef  VPIX
#include <vpix/v86.h>
#endif
#ifdef WEITEK
#include <util/weitek.h>
#endif
#include <util/xdebug.h>
/* #ifdef XXX - MS_EMULATOR */
#include <util/fp.h>
/* #endif XXX - MS_EMULATOR */

#ifndef S_EXEC
#define S_EXEC  S_READ
#endif

#define DR_TRAPS	(DR_TRAP0|DR_TRAP1|DR_TRAP2|DR_TRAP3)
#define DR_ENABLE0	0x03	/* Local or Global enable of trap 0 */
#define DR_ENABLE1	0x0C	/* Local or Global enable of trap 1 */
#define DR_ENABLE2	0x30	/* Local or Global enable of trap 2 */
#define DR_ENABLE3	0xC0	/* Local or Global enable of trap 3 */

extern void systrap();
extern int	cpu_family;

#if __STDC__
int usrxmemflt(int, addr_t, k_siginfo_t *);
STATIC int krnxmemflt(int, addr_t);
STATIC int stop_on_fault(u_int, k_siginfo_t *);
STATIC void fault_to_info(int, k_siginfo_t *);
#else
int usrxmemflt();
STATIC int krnxmemflt();
STATIC int stop_on_fault();
STATIC void fault_to_info();
#endif

/* Addresses of system call and signal-cleanup entry points in kernel */
extern void *sys_call();
extern void *sig_clean();
extern void *common_iret();
extern void *do_ret();

extern u_int do386b1;	/* Enable 80386 B1 stepping workarounds */

/* control structure for covert channel limiter */
STATIC ccevent_t cc_spec_wakeup = { CC_SPEC_WAKEUP, CCBITS_SPEC_WAKEUP };

int *save_r0ptr;	/* pansave() uses this to find the registers */

STATIC void
trapsig(pp, ip)
	register proc_t *pp;
	register k_siginfo_t *ip;
{
	int sig;

	if ((sig = ip->si_signo) != 0) {
		if (sig != SIGTRAP || ip->si_code != TRAP_BRKPT) {

			/*
			 * Avoid a possible infinite loop
			 * if process is holding or ignoring
			 * signal generated by a trap of a
			 * restartable instruction
			 */
			sigdelset(&pp->p_ignore, sig);
			if (u.u_signal[sig - 1] == SIG_IGN)
				u.u_signal[sig - 1] = SIG_DFL;
			sigdelset(&pp->p_hold, sig);
		}
		sigaddq(pp, ip, KM_SLEEP);
	}
}

/*
 * s_trap() is called just before return from kernel mode to user mode
 * if there are pending signals or events to the current process or the
 * current process needs to be preempted.
 */
#if	defined(VPIX) || defined(MERGE386)
void
s_trap(r0ptr)
register int *r0ptr;
#else
void
s_trap()
#endif
{
	register proc_t	*pp;
	time_t	 syst;

	/*
	if (u.u_ageinterval <= 0)
		ageprocess();
	*/

	if (u.u_debugon)
		_wdr7(0);

	pp = u.u_procp;
#if	defined(VPIX) || defined(MERGE386)
	u.u_ar0 = r0ptr;                /* Point to registers on stack */
#endif
	syst = pp->p_stime;
	if (runrun != 0) 
		preempt();

	u.u_sigevpend = 0;

	if (ISSIG(pp, FORREAL)) 
		psig();

	if (u.u_prof.pr_scale & ~1) 
		addupc((void(*)())u.u_ar0[EIP], (int)(pp->p_stime - syst));
}


/*
 * Called from ttrap.s if a trap occurs while on the kernel stack.
 *
 *
 * When this function returns all registers are popped, so modifications
 * to registers on the stack will modify the registers.
 */
#ifdef KPERF
asm int 
geteip()
{
	leal 0(%esp), %eax
}
#endif /* KPERF */

void
k_trap(r0ptr)
	int    *r0ptr;         /* pointer to registers on stack */
{
	register int	vector;			/* idt vector number */
	register u_int	fcflags;
	u_int		debugstatus, dr7;
	k_siginfo_t	info;
	u_int		errcode;
	unsigned long	mceflt_addr[2], mcetype[2];

	vector = r0ptr[TRAPNO];

	/* save pointer to r0 for use by pansave() */
	save_r0ptr = r0ptr;

	/*
	 * Save current fault-catch flags;
	 * set to zero for duration of trap processing,
	 * so we don't incorrectly "catch" bad pointer dereferences.
	 */
	fcflags = u.u_fault_catch.fc_flags;
	u.u_fault_catch.fc_flags = 0;

	/*
	 * If we were moving data to or from a user's
	 * process space and we got a memory fault,
	 * it may be an invalid page.  If so, validate it.
	 * If not, return an error to the fault error handler.
	 */

	if (vector == PGFLT) {
		addr_t		faultadr;
		struct seg	*seg;
		extern struct seg	*segkmap, *segu;

		/*	
	 	 * Try to correct the fault.
	 	 */
		faultadr = (addr_t)_cr2();

		/* For 80386 B1 stepping Errata #9 - page fault code unreliable
		 * Since kernel accesses only trap if page not present, we
		 * force a page-not-present errcode.  We can't tell if it's
		 * read or write, but it's safe to assume read.
		 */
		if (do386b1) {
			if (u.u_386b1 & IN_USERWRITE)
				errcode = PF_ERR_PAGE | PF_ERR_WRITE;
			else 
				errcode = PF_ERR_PAGE;
		} else
			errcode = r0ptr[ERR];

		if (valid_usr_range(faultadr, 1)) {
		    if ((fcflags & (CATCH_UFAULT|CATCH_ALL_FAULTS)) &&
						!servicing_interrupt()) {
			struct_zero((caddr_t)&info, sizeof info);
			errcode = usrxmemflt(errcode, faultadr, &info);
			/* If a user address fault failed, see if a fault
			   handler was set; if so, jump to it. */
			if (errcode != 0) {
				u.u_fault_catch.fc_errno = EFAULT;
				r0ptr[EIP] = (int)u.u_fault_catch.fc_func;
				save_r0ptr = NULL;
#ifdef KPERF
				if (kpftraceflg) {
					Kpc = geteip();
					kperf_write(KPT_UXMEMF, Kpc, curproc);
				}
#endif /* KPERF */
				return;
			}
		    } else
			errcode = EFAULT;
		} else {
			errcode = krnxmemflt(errcode, faultadr);	
			/* If a kernel address fault failed, see if a fault
			   handler was set; if so, jump to it. */
			if (errcode != 0 && !servicing_interrupt() &&
			    ((fcflags & CATCH_ALL_FAULTS) ||
			     (fcflags & CATCH_KPAGE_FAULT) ||
			     (((seg = as_segat(&kas, faultadr)) == segkmap &&
				    (fcflags & CATCH_SEGMAP_FAULT)) ||
			      (seg == segu &&
				    (fcflags & CATCH_SEGU_FAULT))))) {
				u.u_fault_catch.fc_errno = errcode;
				r0ptr[EIP] = (int)u.u_fault_catch.fc_func;
				save_r0ptr = NULL;
#ifdef KPERF
				if (kpftraceflg) {
					Kpc = geteip();
					kperf_write(KPT_UXMEMF, Kpc, curproc);
				}
#endif /* KPERF */
				return;
			}
		}

		/*
	 	 * At this point, if *xmemflt returned a zero value,
	 	 * the fault should be corrected,
	 	 * so return to try the access again.
	 	 */

		if (errcode == 0) {
			u.u_fault_catch.fc_flags = fcflags;
			save_r0ptr = NULL;
#ifdef KPERF
			if (kpftraceflg) {
				Kpc = geteip();
				kperf_write(KPT_TRAP_RET, Kpc, curproc);
			}
#endif /* KPERF */
			return;
		}
	}

	switch (vector) {
	case DBLFLT:
		{
		extern struct tss386 *dfgetuserTSS();

		printf("Double Fault.\n");
		dispTSS(dfgetuserTSS());        /* Display user TSS */
		cmn_err(CE_PANIC, "", DBLFLT);
		break;
		}


	case SGLSTP: { /* debugging related */
		/* It appears that writing dr7 wipes out dr6, so save it first */
		debugstatus = _dr6();
		/* Mask out disabled breakpoints */
		dr7 = _dr7();
		if (!(dr7 & DR_ENABLE0))
			debugstatus &= ~DR_TRAP0;
		if (!(dr7 & DR_ENABLE1))
			debugstatus &= ~DR_TRAP1;
		if (!(dr7 & DR_ENABLE2))
			debugstatus &= ~DR_TRAP2;
		if (!(dr7 & DR_ENABLE3))
			debugstatus &= ~DR_TRAP3;
		/* Now evaluate how we got here */
		if (debugstatus & DR_SINGLESTEP) {
			/*
			 * The i386 even single-steps through lcalls which change
			 * the privilege level. So we take a trap at the first
			 * instruction in privileged mode. It also seems that it
			 * fails to clear the single-step flag in the flags word.
			 * The next instruction would also trap if the flag were
			 * not cleared.
			 *
			 * We cannot psignal() now, lest interruptable system
			 * calls be interrupted. So set a flag to indicate that
			 * upon completion of the system call, deliver the signal.
			 */
			if (
#if defined(VPIX) || defined(MERGE386)
				!(r0ptr[EFL] & PS_VM) &&
#endif
				(r0ptr[CS] & 0xFFFF) == KCSSEL &&
				(r0ptr[EIP] == (int) sys_call ||
				 r0ptr[EIP] == (int) sig_clean)
			) {
				/* turn off trace*/
				((flags_t *)&r0ptr[EFL])->fl_tf = 0;
				u.u_debugpend = 1;
				break;
			}
#ifndef NODEBUGGER
			if (cdebugger != nullsys) {
				_wdr6(debugstatus);
				(*cdebugger)(DR_STEP, r0ptr);
				_wdr6(0);
				break;
			}
#endif
		}
		else if (debugstatus & DR_TRAPS) {
#ifndef NODEBUGGER
			if (cdebugger != nullsys) {
				_wdr6(debugstatus);
				(*cdebugger)(DR_BPT1, r0ptr);
				_wdr6(0);
				break;
			}
#endif
			((flags_t *)&r0ptr[EFL])->fl_rf = 1;
		}

		cmn_err(CE_WARN, "Unexpected INT 1 in system mode");
		((flags_t *)&r0ptr[EFL])->fl_tf = 0; /*turn off trace*/
		struct_zero((caddr_t)&info, sizeof(info));
		info.si_signo = SIGTRAP;
		sigaddq(u.u_procp, &info, KM_SLEEP);
		_wdr6(0);
		break;
	}

	case BPTFLT:		/* breakpoint */
#ifndef NODEBUGGER
		if (cdebugger != nullsys) {
			extern int demon_call_type;	/* From os/sysi86.c */
			int	type = DR_BPT3;

			if (demon_call_type != -1) {
				/*
				 * Inline debugger request.
				 * Convert to specified type.
				 */
				type = demon_call_type;
				demon_call_type = -1;
			}
			(*cdebugger)(type, r0ptr);
		} else
#endif
			cmn_err(CE_PANIC, "Unexpected breakpoint in system mode");
		break;

	case NMIFLT:		/* NMI */
		cmn_err(CE_NOTE, "Unexpected NMI in system mode!");
		break;

	/* floating point extension faults */

	case NOEXTFLT:          /* no extension */
/* #ifdef XXX - MS_EMULATOR */
		if (fp_vers == FP_XOUT)
			cmn_err(CE_PANIC,"NOEXTFLT in kernel mode\n");
		else
/* #endif XXX - MS_EMULATOR */
			fpnoextflt(r0ptr);
		break;

	case EXTOVRFLT:         /* extension address overrun */
/* #ifdef XXX - MS_EMULATOR */
		if (fp_vers == FP_XOUT)
			cmn_err(CE_WARN,"EXTOVRFLT in kernel mode\n");
		else
/* #endif XXX - MS_EMULATOR */
			fpextovrflt(r0ptr);
		break;

	case EXTERRFLT:         /* extension error */
/* #ifdef XXX - MS_EMULATOR */
		if (fp_vers == FP_XOUT)
			cmn_err(CE_WARN,"floating point trap in kernel mode\n");
		else
/* #endif XXX - MS_EMULATOR */
			fpexterrflt(r0ptr);
		break;

	case MCEFLT:		/* machine check exception */
		if (cpu_family == 5) {
			/* it is a P5 */
			_rdmsr(0, &mceflt_addr);
			_rdmsr(1, &mcetype);
			cmn_err(CE_WARN,
				"Machine Check Exception in kernel mode at 0x%x %x type 0x%x\n",
				mceflt_addr[1], mceflt_addr[0], mcetype[0]);
			/* nice to decode the cycle type and print the cycle definitions */
		}

		/* fall through */

	case GPFLT:		/* general protection violation */
		if (!kern_gpfault(r0ptr))
			break;

		/* fall through */

	/*
	 * default action on kernel mode trap
	 * is to display registers and panic.
	 */
	default:
		snap(r0ptr,"PANIC");
		cmn_err(CE_PANIC, "Kernel mode trap. Type 0x%x", vector);
	}

	/* Restore fault-catch flags */
	u.u_fault_catch.fc_flags = fcflags;
}

/*
 * Called from the trap handler when a trap occurs on the user stack.
 * Computes the associated fault (if any) to be delivered to the process,
 * stops the process if a debugger is tracing that fault, and subsequently
 * (or instead) delivers a signal to the process.
 *
 * When this function returns all registers are popped, so modifications
 */

void
u_trap(r0ptr)
int    *r0ptr;         /* pointer to registers on stack */
{
	u_int		fault, errcode;
	k_siginfo_t	info;
	register time_t	syst;
	register struct proc	*pp;
	int	vector;
	u_int	 debugstatus, dr7;
	unsigned long	mceflt_addr[2], mcetype[2];
#ifdef MERGE386
	extern int  merge386flt();
#endif

	vector =(int)r0ptr[TRAPNO];
	u.u_ar0 = r0ptr;	/* point to registers on stack */
	pp = u.u_procp;
	syst = pp->p_stime;

	fault = 0;
	struct_zero((caddr_t)&info, sizeof(info));

	switch (vector) {
		case SGLSTP: { /* debugging related */

			if (u.u_debugon) {
				debugstatus = u.u_debugreg[DR_STATUS];
				dr7 = u.u_debugreg[DR_CONTROL];
			} else {
				debugstatus = _dr6();
				dr7 = _dr7();
			}
			_wdr6(0);

			/* Mask out disabled breakpoints */
			if (!(dr7 & DR_ENABLE0))
				debugstatus &= ~DR_TRAP0;
			if (!(dr7 & DR_ENABLE1))
				debugstatus &= ~DR_TRAP1;
			if (!(dr7 & DR_ENABLE2))
				debugstatus &= ~DR_TRAP2;
			if (!(dr7 & DR_ENABLE3))
				debugstatus &= ~DR_TRAP3;
			if (u.u_debugon)
				u.u_debugreg[DR_STATUS] = debugstatus;

			/* Was it single-stepping ? */
			if (debugstatus & DR_SINGLESTEP) {
				/* turn off trace */
				((flags_t *)&r0ptr[EFL])->fl_tf = 0;

				if (u.u_escbug) {
					u.u_escbug = NULL;
					preempt();
					break;
				}
#ifdef VPIX
				if (pp->p_v86) {
					/* Virt intr for v86 task */
					v86setint(pp->p_v86, V86VI_SGLSTP);
					break;
				}
#endif
				info.si_signo = SIGTRAP;
				info.si_code = TRAP_BRKPT;
				fault = FLTBPT;
				break;
			}

			/* Was it from a debug register setting ? */
			else if (debugstatus & DR_TRAPS) {
				((flags_t *)&r0ptr[EFL])->fl_rf = 1;
#ifdef VPIX
				if (pp->p_v86) {
					/* Virt intr for v86 task */
					v86setint(pp->p_v86, V86VI_SGLSTP);
					break;
				}
#endif
				info.si_signo = SIGTRAP;
				info.si_code = TRAP_BRKPT;
				fault = FLTBPT;
				break;
			}
			else {
				cmn_err(CE_WARN,
					"Unexpected INT 1 in user mode, dr6=%x",
					debugstatus);
				/* Drop through */
			}
#ifdef VPIX
			if (pp -> p_v86)       /* Virt intr for dual mode process */
			{
				v86setint(pp -> p_v86, V86VI_SGLSTP);
				break;
			}
#endif

		}

		case BPTFLT:		/* breakpoint */
		{
#ifdef VPIX
			if (pp -> p_v86)       /* Virt intr for dual mode process */
			{
				v86setint(pp -> p_v86, V86VI_BRKPT);
				break;
			}
#endif

			info.si_signo = SIGTRAP;
			info.si_code = TRAP_BRKPT;
			fault = FLTBPT;
			break;
		}

		case INVOPFLT:		/* invalid opcode */
#ifdef VPIX
			if (pp -> p_v86)       /* Virt intr for dual mode process */
			{
				v86setint(pp -> p_v86, V86VI_INVOP);
				break;
			}
#endif
			info.si_signo = SIGILL;
			info.si_code = ILL_ILLOPC;
			fault = FLTILL;
			break;

		case DIVERR:		/* divide by 0 */
#ifdef VPIX
			if (pp -> p_v86)       /* Virt intr for dual mode process */
			{
				v86setint(pp -> p_v86, V86VI_DIV0);
				break;
			}
#endif
			info.si_signo = SIGFPE;
			info.si_code = FPE_INTDIV;
			fault = FLTIZDIV;
			break;

		case PGFLT:		/* page fault */

			/* For 80386 B1 stepping Errata #9 -
			 *		page fault code unreliable
			 * Check page directory and page table entries to see
			 * if the page is present, and force page-not-present
			 * or protection-violation errcode accordingly.
			 */
			if (do386b1) {
				register pte_t	*pte;

				if (!(pte = vatopdte(_cr2()))->pgm.pg_v ||
				    !(pte = vatopte(_cr2(), pte))->pgm.pg_v) {
					/* Page not present */
					/* Assume read access; no way to know */
					errcode = PF_ERR_PAGE;
				} else {
					/* Protection violation */
					/* Assume write access; it's the only
					   case that matters */
					errcode = PF_ERR_PROT|PF_ERR_WRITE;
				}
			} else
				errcode = r0ptr[ERR];
			if (usrxmemflt(errcode, _cr2(), &info)) {
				if (info.si_signo == SIGSEGV)
					fault = FLTBOUNDS;
				else if (info.si_signo == SIGBUS) 
					fault = FLTACCESS;
				else if (info.si_signo == SIGFPE)
					fault = FLTFPE;
				else
					fault = FLTILL;
			}
			else 
				fault = FLTPAGE;
			break;

		case NMIFLT:		/* NMI */
			cmn_err(CE_NOTE, "Unexpected NMI in user mode!");
			break;

#ifdef MERGE386
		case MERGE386FLT:	/* Merge pseudo fault */
			fault = merge386flt(r0ptr, &info);
			break;
#endif
		case MCEFLT:		/* machine check exception */
			if (cpu_family == 5) {
				/* it is a P5 */
				_rdmsr(0, &mceflt_addr);
				_rdmsr(1, &mcetype);
				cmn_err(CE_WARN,
				"Machine Check Exception in user mode at 0x%x %x type 0x%x\n",
					mceflt_addr[1], mceflt_addr[0], mcetype[0]);
				/* decode the cycle type and print the cycle definitions */
			}
			cmn_err(CE_PANIC, "User mode trap. Type 0x%x", vector);
			snap(r0ptr,"PANIC");

		case DBLFLT:		/* double fault	*/
			cmn_err(CE_WARN,"Unexpected User mode Double fault\n");
			info.si_signo = SIGSEGV;
			info.si_code = SEGV_MAPERR;
			fault = FLTACCESS;
			break;

		case INVTSSFLT:		/* invalid TSS fault */
			cmn_err(CE_WARN, "Unexpected User mode Invalid TSS\n");
			info.si_signo = SIGSEGV;
			info.si_code = SEGV_MAPERR;
			fault = FLTACCESS;
			break;

		case SEGNPFLT:		/* segment not present */
			cmn_err(CE_WARN, 
			    "Unexpected User mode Invalid Segment\n");
			info.si_signo = SIGSEGV;
			info.si_code = SEGV_MAPERR;
			fault = FLTACCESS;
			break;

		default:
			printf("Unknown interrupt\n");
			info.si_signo = SIGKILL;
			break;

		case GPFLT:		/* general protection fault */
		case STKFLT:		/* stack fault */
#ifdef VPIX
		{
			info.si_signo = SIGSEGV;
			info.si_code = SEGV_MAPERR;
			fault = FLTBOUNDS;
			break;
		}
#endif

		case BOUNDFLT:		/* bound fault */
#ifdef VPIX
			if (pp -> p_v86)       /* Virt intr for dual mode process */
			{
				v86setint(pp -> p_v86, V86VI_BOUND);
				break;
			}
#endif
		case INTOFLT:		/* overflow */
		{
#ifdef VPIX
			if (pp -> p_v86)       /* Virt intr for dual mode process */
			{
				v86setint(pp -> p_v86, V86VI_OVERFLOW);
				break;
			}
#endif

			info.si_signo = SIGSEGV;
			info.si_code = SEGV_MAPERR;
			fault = FLTBOUNDS;
			break;
		}
		/* floating point extension faults */

		case NOEXTFLT:		/* no extension */
/* #ifdef XXX - MS_EMULATOR */
			if (fp_vers == FP_XOUT) {
				info.si_signo = SIGFPE;
				info.si_code = ILL_ILLOPC;
				fault = FLTFPE;
			}
			else
/* #endif XXX - MS_EMULATOR */
				fpnoextflt(r0ptr);
			break;

		case EXTOVRFLT:         /* extension address overrun */
/* #ifdef XXX - MS_EMULATOR */
			if (fp_vers == FP_XOUT)
				cmn_err(CE_PANIC, "EXTOVRFLT in user mode with no 287/387\n");
			else
/* #endif XXX - MS_EMULATOR */
				fpextovrflt(r0ptr);
			break;

		case EXTERRFLT:         /* extension error */
/* #ifdef XXX - MS_EMULATOR */
			if (fp_vers == FP_XOUT) {
				info.si_signo = fpukill(r0ptr);
				info.si_code = ILL_ILLOPC;
				fault = FLTFPE;
			} else
/* #endif XXX - MS_EMULATOR */
				fpexterrflt(r0ptr);
			break;
	}

	if (fault 
		&& info.si_signo != SIGKILL
	    	&& prismember(&pp->p_fltmask, fault) 
	    	&& stop_on_fault(fault, &info) == 0)
			info.si_signo = 0;

	trapsig(pp, &info);
	if (runrun != 0)
		preempt();
	if (ISSIG(pp, FORREAL)) {
/* #ifdef XXX - MS_EMULATOR */
		if (((short)u.u_ar0[CS] == FPESEL) && (fp_vers != FP_XOUT))
/* #else
		if ((short)u.u_ar0[CS] == FPESEL)

#endif XXX - MS_EMULATOR 
*/
			fpeclean();
		psig();
	}

	if (u.u_prof.pr_scale & ~1) {
		addupc((void(*)())u.u_ar0[EIP], (int)(pp->p_stime - syst));
	}

	CL_TRAPRET(pp, pp->p_clproc);
}

/*
 * Called from the trap handler when a system call occurs.
 *
 * On entry, assume all registers are pushed.  r0ptr points to registers
 * on stack.  EAX contains system call opcode.  Arguments to system call are
 * still on user's stack.
 *
 * When this function returns all registers are popped, so modifications
 * to registers on the stack will modify the registers.
 */

void
systrap(r0ptr)
register int	*r0ptr;		/* registers on stack */
{
	register u_int	*sp;
	register struct sysent	*callp;
	register proc_t	*pp;
	time_t	syst;
	pid_t	pid;
	u_int	scall;
	int	error = 0;
	rval_t	rval;
	flags_t	*flags = (flags_t *)&r0ptr[EFL];

	pp = u.u_procp;
	syst = pp->p_stime;
	pid = pp->p_pid;

	if (u.u_debugon)
		_wdr7(0);

	sysinfo.syscall++;
	u.u_error = 0;
	u.u_ar0 = r0ptr;
	flags->fl_cf = 0;	/* clear carry bit in flags */

	u.u_syscall = r0ptr[EAX];
	if ((u.u_syscall & 0xff) >= sysentsize)
		u.u_syscall = 0;  /* set to illegal value if off end of table */
	
	scall = u.u_syscall & 0xff;
	callp = &sysent[scall];

	u.u_sysabort = 0;

	/*
	 * Do stop-on-syscall-entry test
	 */
	if (u.u_systrap 
		&& prismember(&u.u_entrymask, scall) 
		&& stop(pp, PR_SYSENTRY, scall, 0))
		swtch();

	/* get arguments in U block, if there are any */
	/* argurments are passed in on user's stack   */

	if (callp->sy_narg) {
		register u_int	*sp = (u_int *)r0ptr[UESP] + 1;
		register u_int	*argp = (u_int *)u.u_arg;

	 	/* inline expansion of ucopyin to speed up syscall */
		if (!VALID_USR_RANGE((caddr_t)sp, callp->sy_narg*sizeof(*sp)))
			error =  EFAULT;
		else {
			CATCH_FAULTS(CATCH_UFAULT) {
				switch(callp->sy_narg) {
					case 4: argp[3] = sp[3]; /* FALLTHRU */
					case 3: argp[2] = sp[2]; /* FALLTHRU */
					case 2: argp[1] = sp[1]; /* FALLTHRU */
					case 1:	argp[0] = *sp;
						break;
					default:
       						bcopy((caddr_t)sp, 
						   (caddr_t)argp,
						   callp->sy_narg*sizeof(*sp));
				}
			}
			error = END_CATCH();
		}
		if (error)
			goto skip;
	}

	rval.r_val1 = 0;			/* default return values */
	rval.r_val2 = r0ptr[EDX];

 	u.u_rval1 = rval.r_val1;		/* XXX */
 	u.u_rval2 = rval.r_val2;		/* XXX */
	u.u_ap = u.u_arg;

        /*
         *  If auditing is on, determine if this event is to be audited
	 *  for this process, and initialize audit structure accordingly
	 */
        if (audit_on)
                adt_auditchk(callp - sysent, u.u_ap);

#ifdef	DEBUG
	sysin();
#endif

	if (u.u_sysabort) {
		/*
		 * u_sysabort may have been set by a debugger while
		 * the process was stopped.  If so, don't execute
		 * the syscall code.
		 */
		u.u_sysabort = 0;
		error = EINTR;
	} else {
		if ((callp->sy_flags & SETJUMP) && setjmp(&u.u_qsav)) {
			if (u.u_error == 0)
				error = EINTR;
			else			/* save in error */
				error = u.u_error;
		}
		else {
#ifdef KPERF
			if (pre_trace) {
				kpftraceflg = 1;
				pre_trace = 0;
			}
			if (kpftraceflg) 
				kperf_write(KPT_SYSCALL, *callp->sy_call, curproc);
#endif /* KPERF */

			error = (*callp->sy_call)(u.u_ap, &rval);
		}
	}


	/* Device drivers cannot not be trusted to stay at spl0 */
	spl0();

skip:
	if (error) {
#ifdef DEBUG
		sysout();
#endif

		if (error == EFBIG && scall != SYS_semsys)
			psignal(u.u_procp, SIGXFSZ);
		else if (error == EINTR) {
			register int cursig = u.u_procp->p_cursig;
			if ((cursig && sigismember(&u.u_sigrestart, cursig)))
				error = ERESTART;
		}

		/* Enhanced Application Compatibility */

		if (isCOFF)
			error = coff_errno(error);

		/* Enhanced Application Compatibility */

/* #ifdef XXX - MS_EMULATOR */
		r0ptr[EAX] = error & 0377;		/* set return value */
/* #else
		r0ptr[EAX] = error; */			/* set return value */
/* #endif XXX - MS_EMULATOR  */
		u.u_error = 0;
		flags->fl_cf = 1;		/* set carry flag */
	} else {
#ifdef DEBUG
		sysok();
#endif
		r0ptr[EAX] = rval.r_val1;
		r0ptr[EDX] = rval.r_val2;
	}

        /*
         *  If auditing is on and this event is being audited,
 	 *  dump a final event audit record to the audit buffer.
 	 */
	if(audit_on)
		if(adt_auditme(A_SYSCHK,0,0))
			adt_record(callp - sysent,error,u.u_ap,&rval);

 	/*
 	 * Reset pp in case we are the child process returning from
 	 * a fork system call.
 	 */
 	pp = u.u_procp;
 
	/*
	 * Do stop-on-syscall-exit test.
	 */
	if (u.u_systrap 
		&& prismember(&u.u_exitmask, scall)
		&& stop(pp, PR_SYSEXIT, scall, 0))
			swtch();

	/*
	 * If we are the parent returning from a successful
	 * vfork, wait for the child to exec or exit.
	 */
	if (scall == SYS_vfork && pid == pp->p_pid && error == 0)
		vfwait((pid_t)rval.r_val1);

	if (runrun != 0) 
		preempt();

	/*
	** If a single-step trap occurred on a syscall (see k_trap())
	** recognize it now.
	*/
	if (u.u_debugpend)
	{
		psignal(pp, SIGTRAP);
		u.u_debugpend = 0;
	}

	if (ISSIG(pp, FORREAL))
		psig();
	/*
	* Delay covert channel processing as much as possible.
	* Apply the limiter on wakeprocs() channel (which may
	* not go to sleep) at this time.
	*/

	if (u.u_covchan) {
		cc_spec_wakeup.cc_bits =
			(long)u.u_covchan * CCBITS_SPEC_WAKEUP;
		cc_limiter(&cc_spec_wakeup, u.u_cred);
		u.u_covchan = 0;
		u.u_systrap &= ~P_CC;
	}



	/*
	 * If pid != pp->p_pid, then we are the child
	 * returning from a fork system call.  In this
	 * case, ignore syst since our time was reset
	 * in fork.
	*/

	if (u.u_prof.pr_scale & ~1)
		addupc((void(*)())u.u_ar0[EIP],
			pid == pp->p_pid ? (int)(pp->p_stime - syst)
					 : (int)pp->p_stime);

 	CL_TRAPRET(pp, pp->p_clproc);
}

/*
 * nonexistent system call-- signal bad system call.
 */

/* ARGSUSED */
int
nosys(uap, rvp)
char	*uap;
rval_t	*rvp;
{
	psignal(u.u_procp, SIGSYS);
	return EINVAL;
}

/* 
 * package not installed -- return ENOPKG error  (STUBS support)
 */

/* ARGSUSED */
int
nopkg(uap, rvp)
char	*uap;
rval_t	*rvp;
{
	return ENOPKG;
}

/*
 * internal function call for uninstalled package -- panic system.  If the
 * system ever gets here, it means that an internal routine was called for
 * an optional package, and the OS is in trouble.  (STUBS support)
 */

void
noreach()
{
	cmn_err(CE_PANIC,"Call to internal routine of uninstalled package");
}

/*
 * stray interrupts enter here
 */

void
intnull()
{
}

/*
 * Ignored system call
 */
void 
nullsys()
{
}

void
stray(addr)
{
	cmn_err(CE_NOTE,"stray interrupt at %x\n", addr);
}

int
kern_gpfault(r0ptr)
int *r0ptr;
{
	int *prevr0ptr;
	int tmp_eip = r0ptr[EIP];
	int tmp_cs = r0ptr[CS];
	int tmp_efl = r0ptr[EFL];
	int tmp_uesp = r0ptr[UESP];
	int tmp_ss = r0ptr[SS];

	if (u.u_sigfault) {	/* Allow the user to blow	*/
				/* himself away during		*/
				/* signal processing.		*/
		/* Set u.u_usigfailed so that the user could	*/
		/* tell that he dumped core because he stepped	*/
		/* on the register save area while doing his	*/
		/* own signal processing.			*/
		u.u_usigfailed = 1;
		asm("	sti");	/* remember, interrupts were	*/
				/* disabled in sig_clean	*/
		u.u_ar0 = r0ptr;/* core will use these registers */
		exit( (core("core", u.u_procp, u.u_cred,
			u.u_rlimit[RLIMIT_CORE].rlim_cur, SIGSEGV) ?
				CLD_DUMPED|CLD_KILLED : CLD_KILLED), SIGSEGV);
		return 0;
	}

	/*
	** Returns true if the kernel should panic.
	**
	** ptrace(), the user who trashes his stack during signal handling,
	** the [23]87 emulators, and i286 emulator,
	** and perhaps other circumstances will occasionally set various user
	** registers or selectors to real bad values. Unfortunately, the
	** values of the user registers seem to cause a k_trap() instead of
	** a u_trap() in the iret instruction.
	** PROBLEM: Suppose user signal handling code trashes its stack and
	** sets itself as a signal-handler. Then the program (properly) grows
	** a user stack until it can no longer be grown, fails sendsig(), and
	** dumps a core().
	** PROBLEM: The kernel accidentally trashes its own stack, but doesn't
	** discover this until the common_iret. Since the value of EIP can
	** be anything at all (the user could have set it to, for example,
	** a kernel address), we have no choice but treat this as a user-trap
	** and deliver a SIGSEGV to the user process. But to protect ourselves,
	** we must check that we are not delivering a signal to a system
	** process.
	** PROBLEM: The stack is where we were about to return to user-mode,
	** almost entirely popped. The user's accumulators and segments were
	** restored before the fault and the old flag and error code have been
	** destroyed. There is no privilege level transition when the processor
	** faults on the iret. As a consequence, there is no ESP on the stack.
	** Don't we want to record the correct context for sdb and the user's
	** process? Yes. Before we deliver the signal, copy the bogus iret
	** registers from the area beneath the current frame into the current
	** frame. psig() will either deliver the signal (and push these
	** registers onto the user stack) or output the contents of the
	** kernel stack to a core file. But afterwards, completely pop
	** the kernel stack.
	*/
	if (!(r0ptr[EIP] <= (int) common_iret && r0ptr[EIP] >= (int)do_ret) ||
	    (u.u_procp->p_flag & SSYS) && !u.u_lock)
		return 1;

	/*
	** Preserve the registers at the iret.
	*/
	tmp_eip = r0ptr[EIP];
	tmp_cs = r0ptr[CS];
	tmp_efl = r0ptr[EFL];
	tmp_uesp = r0ptr[UESP];
	tmp_ss = r0ptr[SS];

	/*
	** It had just popped off the ERR code when it got
	** the fault in the iret, and the new fault 
	** pushed new EFL flags on. Copy this stuff into
	** the current frame for use by ptrace() and core().
	*/
	prevr0ptr = &r0ptr[EFL] - ERR;
	r0ptr[EIP] = prevr0ptr[EIP];
	r0ptr[CS] = prevr0ptr[CS];
	r0ptr[EFL] = prevr0ptr[EFL];
	r0ptr[UESP] = prevr0ptr[UESP];
	r0ptr[SS] = prevr0ptr[SS];
	u.u_ar0 = r0ptr;

#ifdef maybe_later
	psignal(u.u_procp, SIGSEGV);
	if (ISSIG(u.u_procp, FORREAL)) {
		psig();
	} 

	/*
	** It didn't exit. So lets completely pop the kernel
	** stack before attempting to return to the user.
	** First, return the signal handling stuff into
	** for returning to the user to the bottom of the
	** stack.
	*/
	prevr0ptr[EIP] = r0ptr[EIP];
	prevr0ptr[CS] = r0ptr[CS];
	prevr0ptr[EFL] = r0ptr[EFL];
	prevr0ptr[UESP] = r0ptr[UESP];
	prevr0ptr[SS] = r0ptr[SS];

	/*
	** Now, set up things to attempt two irets in
	** quick succession, the first to the second, the
	** second to the user.
	*/
	r0ptr[EIP] = tmp_eip;
	r0ptr[CS] = tmp_cs;
	r0ptr[EFL] = tmp_efl;
	r0ptr[UESP] = tmp_uesp;
	r0ptr[SS] = tmp_ss;
	return 0;

#else

	/* forget about all the comments above. */
	/* if we return from psig or are holding or ignoring SIGSEGV */
	/* in the original code (following the else), the kernel */
	/* can get into an infinite loop if the user EIP being returned */
	/* to in common_iret is outside the user code segment limit */
	/* This can happen if the user EIP register is set to -1 */
	/* using prioctl(2).  The common_iret does an iret which */
	/* pops the saved (bad) eip from the stack which causes */
	/* a general protection fault which is how we got here  */
	/* in the first place (ugh).  */
	u.u_ar0 = r0ptr;/* core will use these registers */
	exit( (core("core", u.u_procp, u.u_cred,
		u.u_rlimit[RLIMIT_CORE].rlim_cur, SIGSEGV) ?
			CLD_DUMPED|CLD_KILLED : CLD_KILLED), SIGSEGV);
	return 0;
#endif
}


/* dispTSS - Display a TSS structure */

dispTSS(tss)
struct tss386 *tss;
{
    char *tss_str="tss386.%s:\t0x%x\n";       /* Format string */

    tss = (struct tss386 *) &u.u_tss; /* Ptr to TSS struct */
					/* Override the argument passed in */

    printf(tss_str,"t_link",tss->t_link);
    printf(tss_str,"t_esp0",tss->t_esp0);
    printf(tss_str,"t_ss0",tss->t_ss0);
    printf(tss_str,"t_esp1",tss->t_esp1);
    printf(tss_str,"t_ss1",tss->t_ss1);
    printf(tss_str,"t_esp2",tss->t_esp2);
    printf(tss_str,"t_ss2",tss->t_ss2);
    printf(tss_str,"t_cr3",tss->t_cr3);
    printf(tss_str,"t_eip",tss->t_eip);
    printf(tss_str,"t_eflags",tss->t_eflags);
    printf(tss_str,"t_eax",tss->t_eax);
    printf(tss_str,"t_ecx",tss->t_ecx);
    printf(tss_str,"t_edx",tss->t_edx);
    printf(tss_str,"t_ebx",tss->t_ebx);
}

STATIC void
fault_to_info(flt, infop)
int	 flt;
register k_siginfo_t	*infop;
{
	switch(FC_CODE(flt)) {
	case 0:
		infop->si_signo = 0;
		break;

	case FC_NOMAP:
		infop->si_signo = SIGSEGV;
		infop->si_code = SEGV_MAPERR;
		break;

	case FC_PROT:
		infop->si_signo = SIGSEGV;
		infop->si_code = SEGV_ACCERR;
		break;

	case FC_OBJERR:
		infop->si_signo = SIGBUS;
		infop->si_code = BUS_OBJERR;
		infop->si_errno = FC_ERRNO(flt);
		break;

	default:
		infop->si_signo = SIGKILL;
		break;
	}
}

#ifdef WEITEK
extern int init_weitek_pt();
extern int map_weitek_pt();
#endif

int
usrxmemflt(errcode, faultadr, infop)
	register int	errcode;
	register addr_t	faultadr;
	k_siginfo_t	*infop;
{
	register proc_t	*pp = u.u_procp;
	int	 flt;
	struct   seg    *sp;
	struct   as     *asp;
	enum seg_rw rw;


	infop->si_addr = (caddr_t)faultadr;

#ifdef KPERF
	if (kpftraceflg) {
		Kpc = geteip();
		kperf_write(KPT_UXMEMF, Kpc, curproc);
	}
#endif /* KPERF */

	if ((asp = pp->p_as) == (struct as *)NULL)
		cmn_err(CE_PANIC, "usrxmemflt: no as allocated: %x",faultadr);

	sp = as_segat(asp, faultadr);

	if (errcode & PF_ERR_WRITE)
		rw = S_WRITE;
	else
		rw = S_READ;

	switch (errcode & PF_ERR_MASK) {

	case PF_ERR_PAGE:
		if (sp == NULL) {
#ifndef ADDR0
		/*
	 	 * For backward compatibility, we open a door here to 
	 	 * allow virtual page 0 to be accessible (read-only) even with
	 	 * the new 4.0 virtual map.  This is for those applications 
	 	 * that depend on NULL pointer dereferences to work.
	 	 *
	 	 * We map in the whole page and create a zfod segment
		 * for this page.
	 	 */

		    if (btop(faultadr) == 0) {
			    if (as_map(asp, (caddr_t)0, PAGESIZE,
					segvn_create, zfod_argsp)) {
				infop->si_signo = SIGSEGV;
				infop->si_code = SEGV_MAPERR;
				break;
			    }

			    /* disallow write to emulate 3.2 map */
			    (void)as_setprot(asp, (caddr_t)0, PAGESIZE,
					PROT_ALL & ~PROT_WRITE);
		    } else
#endif

#ifdef WEITEK
		    if (((unsigned long) faultadr & WEITEK_ADDRS) 
				== WEITEK_VADDR) { /* Weitek Address? */
			if (weitek_kind & WEITEK_HW) {	/* chip present */

				if (weitek_pt < 0)
					init_weitek_pt();

				map_weitek_pt();
				flushtlb();
				u.u_weitek = WEITEK_HW;
				init_weitek();
				weitek_proc = u.u_procp;
				return(0);
			}
		    } else
#endif
		    if (faultadr <= 
			    (pp->p_stkbase - pp->p_stksize + sizeof(int)) &&
			    !grow((int *)(_VOID *)faultadr)) {
			infop->si_signo = SIGSEGV;
			infop->si_code = SEGV_MAPERR;
			break;
		    }
		}
		if (flt = as_fault(asp, faultadr, 1, (enum fault_type)F_INVAL, rw))
			fault_to_info(flt, infop);
		break;

	case PF_ERR_PROT:
		if (errcode & PF_ERR_WRITE) {
		    	if (flt = as_fault(asp, faultadr, 1, (enum fault_type)F_PROT, rw))
				fault_to_info(flt, infop);
			break;
		}
		else {
			infop->si_signo = SIGSEGV;
			infop->si_code = SEGV_MAPERR;
		}
		break;

	default:
		cmn_err(CE_PANIC, "usrxmemflt, impossible condition") ;
	}
	return(infop->si_signo);
}

/* ARGSUSED */
STATIC int
krnxmemflt(errcode, faultadr)
	register int	errcode;
	register addr_t	faultadr;
{
	register int	flt;
	struct as *asp;
	enum seg_rw rw;

#ifdef KPERF
	if (kpftraceflg) {
		Kpc = geteip();
		kperf_write(KPT_UXMEMF, Kpc, curproc);
	}
#endif /* KPERF */

	if (errcode & PF_ERR_WRITE)
		rw = S_WRITE;
	else
		rw = S_READ;

	asp = &kas;

	switch(errcode & PF_ERR_MASK) {

	case PF_ERR_PAGE:

		flt = as_fault(asp, faultadr, 1, (enum fault_type)F_INVAL, rw);
		break;

	case PF_ERR_PROT:

		flt = as_fault(asp, faultadr, 1, (enum fault_type)F_PROT, rw);
		break;

	default:
		flt = FC_HWERR;
		break;
	}

	if (flt == 0)
		return 0;

	/* convert fault code to errno and return it */
	if (FC_CODE(flt) == FC_OBJERR)
		return FC_ERRNO(flt);
	else
		return EFAULT;
}

/* 
 * Handle stop-on-fault processing for the debugger.
 * Returns 0 if the fault is cleared during the stop,
 * nonzero if it isn't.
 */
STATIC int
stop_on_fault(fault, sip)
	register u_int		fault;
	register k_siginfo_t	*sip;
{
	register proc_t *p = u.u_procp;

	ASSERT(prismember(&p->p_fltmask, fault));

	/*
	 * Record current fault and siginfo structure
	 * so debugger can find it.
	 */
	p->p_curflt = (u_char)fault;
	u.u_siginfo = *sip;

	if (stop(p, PR_FAULTED, fault, 0))
		swtch();
	fault = p->p_curflt;
	p->p_curflt = 0;
	return fault;
}
