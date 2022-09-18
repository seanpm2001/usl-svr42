/*	Copyright (c) 1990, 1991, 1992 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1985, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)idload:idload.c	1.9.16.4"
#ident "$Header: /sms/sinixV5.4es/rcs/s19-full/usr/src/cmd/idload/idload.c,v 1.1 91/02/28 17:34:26 ccs Exp $"
#include <stdio.h>
#include <sys/types.h>
#include <sys/idtab.h>
#include <sys/nserve.h>
#include <sys/rf_sys.h>
#include <fcntl.h>
#include <sys/ksym.h>
#include <nserve.h>
#include <sys/param.h>
#include <sys/tiuser.h>
#include <sys/stat.h>
#include <nsaddr.h>
#include <sys/rf_cirmgr.h>
#include <pn.h>

#define MGDP_STR	"maxgdp"
#define GDP_STR		"gdp"
#define GLOBAL_STR	"Global"

#define MEMF		"/dev/kmem"
#define ID_DATA		"/etc/iddata"

#define	ERROR		-1
#define	NO_ERROR	0
#define	NO_MAPPING	1


extern char *optarg;
extern int   optind;

char	*cmd_name;

main(argc, argv)
int   argc;
char *argv[];
{
	int	n_update = 0;
	int	error = 0;
	int	num_args = 0;
	int	gflag = 0, uflag = 0;
	int	kflag = 0;
	int	usr_map_stat = 0, grp_map_stat = 0;
	int	c;

	char	*u_rules = "/etc/rfs/auth.info/uid.rules";
	char	*g_rules = "/etc/rfs/auth.info/gid.rules";
	char	*passdir = NULL;

	cmd_name = argv[0];

	/*
	 *	Parse arguments.
	 */

	while ((c = getopt(argc, argv, "ng:u:k")) != EOF) {
		switch (c) {
			case 'n':
				if (n_update || kflag)
					error = 1;
				else
					n_update = 1;
				break;
			case 'g':
				if (gflag || kflag)
					error = 1;
				else {
					g_rules = optarg;
					gflag = 1;
				}
				break;
			case 'u':
				if (uflag || kflag)
					error = 1;
				else {
					u_rules = optarg;
					uflag = 1;
				}
				break;
			case 'k':
				if (kflag || n_update || uflag || gflag)
					error = 1;
				else {
					kflag = 1;
				}
				break;
			case '?':
				error = 1;
		}
	}

	num_args = argc - optind;

	if (num_args == 1)
		passdir = argv[optind];

	if (num_args == 1 && kflag)
		error = 1;

	if (num_args > 1 || error) {
		fprintf(stderr, "%s: usage: %s [-n] [-g gid_rules] [-u uid_rules] [directory]\n", argv[0], argv[0]);
		fprintf(stderr, "%s:        %s -k\n", argv[0], argv[0]);
		exit(1);
	}

#ifdef OLDSEC
	if (geteuid() != (uid_t)0) {
		fprintf(stderr, "%s: must be super-user\n", argv[0]);
		exit(1);
	}
#endif /*OLDSEC*/

	/*
	 *	If the "-k" flag is set, then print out the mapping
	 *	information in the kernel.  Otherwise, parse the user-
	 *	level mapping files (the code to do that is in
	 *	libns:uidmap.c).
	 */

	if (kflag) {
		if (getmap() == ERROR) {
			fprintf(stderr, "%s: could not obtain idmapping information from the kernel\n", argv[0]);
			exit(1);
		} else {
			exit(0);
		}
	}

	/*
	 *	Clear the translation tables and
	 *	perform user id mapping for all machines.
	 */

	if (n_update == 0)
		rfsys(RF_SETIDMAP, NULL, UID_MAP, NULL);

	usr_map_stat = uidmap(UID_MAP, u_rules, passdir, NULL, n_update);

	/*
	 *	Perform group id mapping for all machines.
	 */

	grp_map_stat = uidmap(GID_MAP, g_rules, passdir, NULL, n_update);

	if (grp_map_stat == 0 || usr_map_stat == 0)
		exit(1);
	else
		exit(0);
}

/*
 *	Get the idmapping that is currently in the kernel
 */

getmap()
{
	int	mem, gdpsiz, size, i;
	int	table, global;
	int	errorstate;
	struct	gdp *gdpspace;
	struct	stat iddata, uxdata;
	long     gdpaddr = 0;
	struct mioc_rksym rks;

	/*
	 *	open file to access memory
	 */

	if ((mem = open(MEMF, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: cannot open %s\n", cmd_name, MEMF);
		return(ERROR);
	}

	/*
	 *	read in the value of maxgdp to get the size of the
	 *	gdp table.
	 */

	rks.mirk_symname = MGDP_STR;
	rks.mirk_buf = (void *) &gdpsiz;
	rks.mirk_buflen = sizeof(gdpsiz);
	if (ioctl(mem, MIOC_READKSYM, &rks) != 0) {
		fprintf(stderr, "%s: cannot get kernel-level data\n", cmd_name);
		return(ERROR);
	}

	size = gdpsiz * sizeof(struct gdp);

	/*
	 *	read in the gdp table.
	 */

	rks.mirk_symname = GDP_STR;
	rks.mirk_buflen = size;
	if (((rks.mirk_buf = (void *)malloc(size)) == NULL)
	 || (ioctl(mem, MIOC_IREADKSYM, &rks) != 0)) {
		fprintf(stderr, "%s: cannot get gdp table kernel-level data\n",
				cmd_name);
		return(ERROR);
	}
	gdpspace = rks.mirk_buf;

	/*
	 *	The following loop prints out idtable information
	 *	for uid mappings (i == 0) and gid mappings (i == 1).
	 */

	printf("%-5s %-14s %-11s %-14s %-14s %-14s\n", "TYPE",
		"MACHINE", "REM_ID", "REM_NAME", "LOC_ID", "LOC_NAME");

	errorstate = NO_ERROR;

	for (i = 0; i < 2; i ++) {
		/*
		 *	Print the global specification.
		 */

		printf("\n");
		global = pr_global(i, mem);

		/*
		 *	Print out the values of the idtables associated
		 *	with each gdp structure.
		 */

		table = pr_gdp(i, gdpspace, gdpsiz, mem);

		if (global == NO_MAPPING && table == NO_MAPPING)
			no_mappings(i);

		if (global == ERROR || table == ERROR) {
			fprintf(stderr, "%s: error in accessing the kernel-level idtables\n", cmd_name);
			errorstate = ERROR;
		}
	}

	return(errorstate);
}

pr_global(flag, mem)
int flag, mem;
{
	char	*globp[2];
	struct mioc_rksym rks;

	/*
	 *	Read in the value of the pointers to the idtables
	 *	for the global specification.
	 */

	rks.mirk_symname = GLOBAL_STR;
	rks.mirk_buf = (void *) globp;
	rks.mirk_buflen = sizeof(globp);
	if (ioctl(mem, MIOC_READKSYM, &rks) != 0)
		return(ERROR);

	if (globp[flag] == 0)
		return(NO_MAPPING);
	else
		return(pr_table(flag, "GLOBAL", mem, globp[flag]));
}

pr_gdp(flag, gdpspace, gdpsiz, mem)
int	flag;
struct	gdp *gdpspace;
int	gdpsiz;
int	mem;
{
	struct	gdp *gdpp, *lastgdp, *temp;
	int	already_done;
	int	count = 0;

	lastgdp = gdpspace + gdpsiz;

	/*
	 *	Go through the gdp table and get the idmap table
	 *	this machine has created for each remote machine.
	 */

	for (gdpp = gdpspace; gdpp < lastgdp; gdpp++) {
		if (!gdpp->constate)
			continue;

		/*
		 *	If gdpp->idmap[flag] is 0, then no specific
		 *	mapping for the remote machine was specified.
		 */

		if (gdpp->idmap[flag] == 0)
			continue;
	
		/*
		 *	Check to make sure that this machine has
		 *	not already been processed.
		 */

		count ++;
		already_done = 0;
		for (temp = gdpspace; temp < gdpp; temp++) {
			if (temp->constate &&
			  strcmp(temp->token.t_uname,
			    gdpp->token.t_uname) == 0) {
				already_done = 1;
				break;
			}
		}

		if (already_done)
			continue;

		if (pr_table(flag, gdpp->token.t_uname, mem, gdpp->idmap[flag]) == ERROR)
			return(ERROR);
	}
	if (count == 0)
		return(NO_MAPPING);
	else
		return(NO_ERROR);
}

pr_table(flag, name, mem, place)
int flag;
char *name;
int mem;
struct idhead *place;
{
	struct	idhead headbuf;
	struct	idtab *where, *idtable, *ptr;
	char	*type = (flag == 0)? "USR": "GRP";
	int	i, size;
	char	rem_str[16], temp_str[16];

	/*
	 *	Read in the header of the idmap table and
	 *	print out thr default values.
	 */

	if (rread(mem, place, &headbuf, sizeof(headbuf)) == ERROR)
		return(ERROR);

	print_def(type, headbuf.i_default, name);

	/*
	 *	headbuf.i_cend is the offset
	 *	(in struct idtabs) to the actual idtable.
	 */

	where = (struct idtab *)(place) + headbuf.i_cend;
	size = sizeof(struct idtab) * headbuf.i_size;

	if (size == 0)
		return(NO_ERROR);

	if (((idtable = (struct idtab *)malloc(size)) == NULL)
	 || (rread(mem, where, idtable, size) == ERROR))
		return(ERROR);

	/*
	 *	Go through the idtable and print out each entry.
	 *	Note: if the local value (ptr->i_loc) is a -1,
	 *	then the entry represents a range of remote
	 *	values.  Therefore, print out something like:
	 *		0       -1
	 *		100     999
	 *	as:
	 *		0-100   999
	 */

	rem_str[0] = '\0';
	for (i = 0; i < (int)headbuf.i_size; i++) {
		ptr = idtable + i;
		sprintf(temp_str, "%ld", ptr->i_rem);
		strcat(rem_str, temp_str);
		if (ptr->i_loc == (uid_t)-1) {
			strcat(rem_str, "-");
			continue;
		}
		printf("%-5s %-14s %-11s %-14s %-14d %-14s\n",
		  type, name, rem_str, "n/a", ptr->i_loc,
		  ptr->i_loc == MAXUID + 1? "guest_id":"n/a");
		rem_str[0] = '\0';
	}
	return(NO_ERROR);
}

print_def(type, def, name)
char *type;
int   def;
char *name;
{
	char def_str[15];

	/*
	 *	Print out the default value for the remote
	 *	machine.
	 */

	sprintf(def_str, "%d", def);
	printf("%-5s %-14s %-11s %-14s %-14s %-14s\n",
		type, name, "DEFAULT", "n/a",
		def == 0? "transparent": def_str,
		def == UID_NOBODY? "guest_id":"n/a");
}

no_mappings(flag)
int flag;
{
	printf("%-5s %-14s %-11s %-14s %-14d %-14s\n",
		flag == 0? "USR":"GRP", "GLOBAL", "DEFAULT", "n/a",
		UID_NOBODY, "guest_id");
}

rread(device, position, buffer, count)
char *buffer;
int count, device;
long position;
{
	/*
	 *	Seeks to "position" on device "device" and reads "count"
	 *	bytes into "buffer".
	 */

	if ((lseek(device, position, 0) == (off_t) -1)
	 || (read(device, buffer, (unsigned) count) == ERROR)) {
		fprintf(stderr, "idload: error in reading %s\n",MEMF);
		return(ERROR);
	}
	return(NO_ERROR);
}
