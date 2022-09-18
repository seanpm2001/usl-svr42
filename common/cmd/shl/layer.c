/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)shl:layer.c	1.8.8.1"
#ident "$Header: /sms/sinixV5.4es/rcs/s19-full/usr/src/cmd/shl/layer.c,v 1.1 91/02/28 20:09:27 ccs Exp $"
#include	"defs.h"

char *malloc();

create(name)
	char *name;
{
	int i;
	int n;
	char c;
	pid_t spawn_shell();

	if (i = get_new_layer(name))
	{
		layers[i] = (struct layer *)malloc(sizeof(struct layer));

		if (*name)
			strcpy(layers[i]->name, name);
		else
			sprintf(layers[i]->name, "(%d)", i);
				
		if (layers[i]->p_grp = spawn_shell(i))
		{	
			while ((n = read(fildes[0], &c, 1)) < 0)
				;
			if (n > 0)
			{
				if (ioctl(cntl_chan_fd, SXTIOCSWTCH, i) != SYSERROR)
				{
					close(fildes[0]);
					push(i);
					adj_utmp(i);
					wait_for_fg(i);
					adj_utmp(0);
					return;
				}
				else
				{
					pfmt(stderr, MM_ERROR, badswitch,
						strerror (errno));
					kill(-layers[i]->p_grp, SIGKILL);
				}
			}
			close(fildes[0]);
		}		

		free_layer(i);
	}
}


destroy(name)
	char *name;
{
	int i;

	if (i = lookup_layer(name))
	{	
		kill(-layers[i]->p_grp, SIGHUP);
		if (i == top())
			pop();
	}
}


resume(name)
	char *name;
{
	int i;

	if (i = lookup_layer(name))
	{
		pfmt(stdout, MM_INFO, resuming, name);
		
		rstttysets(i);
		if (ioctl(cntl_chan_fd, SXTIOCSWTCH, i) != SYSERROR)
		{
			push(i);
			adj_utmp(i);
			wait_for_fg(i);
			adj_utmp(0);
		}
		else
			pfmt(stderr, MM_ERROR, badswitch, strerror (errno));
	}
}


resume_current()
{
	int n;

	while (n = top())
	{
		if (layers[n] == 0)
			pop();
		else
			break;
	}

	if (n)
	{
		pfmt(stdout, MM_INFO, resuming, layers[n]->name);

		rstttysets(n);
		if (ioctl(cntl_chan_fd, SXTIOCSWTCH, n) == SYSERROR)
		{
			pop();
			pfmt(stderr, MM_ERROR, badswitch, strerror(errno));
		}
		else
		{
			adj_utmp(n);
			wait_for_fg(n);
			adj_utmp(0);
		}
	}
}


kill_all()
{
	int i;
	int retval;
	int found = 0;
	void sigalrm();
	signal(SIGCLD, SIG_IGN);

	for (i = 1; i <= max_index; ++i)
		if (layers[i])
		{
			kill(-layers[i]->p_grp, SIGHUP);
			found = 1;
		}
	signal(SIGALRM, sigalrm);
	if (found) {
		alarm(3);
		wait(&retval);
	}
}

wait_for_fg(n)
	int n;
{
	char rdbuf[4];

	while (read(cntl_chan_fd, rdbuf, sizeof(int)) == SYSERROR)
		;

	/* save current tty settings */
	savettysets(n);
	ioctl(cntl_chan_fd, TCSETAW, &ttysave);
}


block(name)
	char *name;
{
	int i;

	if (i = lookup_layer(name))
	{
		if (ioctl(cntl_chan_fd, SXTIOCBLK, i) == SYSERROR)
			pfmt(stderr, MM_ERROR, badswitch, strerror(errno));
	}
}


unblock(name)
	char *name;
{
	int i;

	if (i = lookup_layer(name))
	{
		if (ioctl(cntl_chan_fd, SXTIOCUBLK, i) == SYSERROR)
			pfmt(stderr, MM_ERROR, badswitch, strerror(errno));
	}
}


struct termio ttysettings[MAX_LAYERS];

savettysets(n)
	int n;
{
	/* save tty settings for a winder */

	if (n >= MAX_LAYERS)
	{
		pfmt(stderr, MM_WARNING, ":11:Trying to save bad tty %d\n", n);
		return;
	}

	if (ioctl(cntl_chan_fd, TCGETA, &ttysettings[n]) == SYSERROR)
	{
		pfmt(stderr, MM_WARNING, ":12:Savetty ioctl fails %d\n", n);
	}
}

rstttysets(n)
	int n;
{
	/* reset tty settings for a winder */
	if (n >= MAX_LAYERS)
	{
		pfmt(stderr, MM_WARNING, ":13:Trying to reset bad tty %d\n", n);
		return;
	}

	if (ioctl(cntl_chan_fd, TCSETA, &ttysettings[n]) == SYSERROR)
	{
		pfmt(stderr, MM_WARNING, ":14:Resettty ioctl fails %d\n", n);
	}
}
