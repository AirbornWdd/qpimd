/* kroute daemon main routine.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Kroute.
 *
 * GNU Kroute is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Kroute is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Kroute; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <kroute.h>

#include <lib/version.h>
#include "getopt.h"
#include "command.h"
#include "thread.h"
#include "filter.h"
#include "memory.h"
#include "prefix.h"
#include "log.h"
#include "plist.h"
#include "privs.h"
#include "sigevent.h"

#include "kroute/rib.h"
#include "kroute/zserv.h"
#include "kroute/debug.h"
#include "kroute/router-id.h"
#include "kroute/irdp.h"
#include "kroute/rtadv.h"

/* Kroute instance */
struct kroute_t krouted =
{
  .rtm_table_default = 0,
};

/* process id. */
pid_t pid;

/* Pacify zclient.o in libkroute, which expects this variable. */
struct thread_master *master;

/* Route retain mode flag. */
int retain_mode = 0;

/* Don't delete kernel route. */
int keep_kernel_mode = 0;

#ifdef HAVE_NETLINK
/* Receive buffer size for netlink socket */
u_int32_t nl_rcvbufsize = 0;
#endif /* HAVE_NETLINK */

/* Command line options. */
struct option longopts[] = 
{
  { "batch",       no_argument,       NULL, 'b'},
  { "daemon",      no_argument,       NULL, 'd'},
  { "keep_kernel", no_argument,       NULL, 'k'},
  { "config_file", required_argument, NULL, 'f'},
  { "pid_file",    required_argument, NULL, 'i'},
  { "socket",      required_argument, NULL, 'z'},
  { "help",        no_argument,       NULL, 'h'},
  { "vty_addr",    required_argument, NULL, 'A'},
  { "vty_port",    required_argument, NULL, 'P'},
  { "retain",      no_argument,       NULL, 'r'},
  { "dryrun",      no_argument,       NULL, 'C'},
#ifdef HAVE_NETLINK
  { "nl-bufsize",  required_argument, NULL, 's'},
#endif /* HAVE_NETLINK */
  { "user",        required_argument, NULL, 'u'},
  { "group",       required_argument, NULL, 'g'},
  { "version",     no_argument,       NULL, 'v'},
  { 0 }
};

kroute_capabilities_t _caps_p [] = 
{
  ZCAP_NET_ADMIN,
  ZCAP_SYS_ADMIN,
  ZCAP_NET_RAW,
};

/* kroute privileges to run with */
struct kroute_privs_t zserv_privs =
{
#if defined(BANE_USER) && defined(BANE_GROUP)
  .user = BANE_USER,
  .group = BANE_GROUP,
#endif
#ifdef VTY_GROUP
  .vty_group = VTY_GROUP,
#endif
  .caps_p = _caps_p,
  .cap_num_p = sizeof(_caps_p)/sizeof(_caps_p[0]),
  .cap_num_i = 0
};

/* Default configuration file path. */
char config_default[] = SYSCONFDIR DEFAULT_CONFIG_FILE;

/* Process ID saved for use by init system */
const char *pid_file = PATH_KROUTE_PID;

/* Help information display. */
static void
usage (char *progname, int status)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    {    
      printf ("Usage : %s [OPTION...]\n\n"\
	      "Daemon which manages kernel routing table management and "\
	      "redistribution between different routing protocols.\n\n"\
	      "-b, --batch        Runs in batch mode\n"\
	      "-d, --daemon       Runs in daemon mode\n"\
	      "-f, --config_file  Set configuration file name\n"\
	      "-i, --pid_file     Set process identifier file name\n"\
	      "-z, --socket       Set path of kroute socket\n"\
	      "-k, --keep_kernel  Don't delete old routes which installed by "\
				  "kroute.\n"\
	      "-C, --dryrun       Check configuration for validity and exit\n"\
	      "-A, --vty_addr     Set vty's bind address\n"\
	      "-P, --vty_port     Set vty's port number\n"\
	      "-r, --retain       When program terminates, retain added route "\
				  "by kroute.\n"\
	      "-u, --user         User to run as\n"\
	      "-g, --group	  Group to run as\n", progname);
#ifdef HAVE_NETLINK
      printf ("-s, --nl-bufsize   Set netlink receive buffer size\n");
#endif /* HAVE_NETLINK */
      printf ("-v, --version      Print program version\n"\
	      "-h, --help         Display this help and exit\n"\
	      "\n"\
	      "Report bugs to %s\n", KROUTE_BUG_ADDRESS);
    }

  exit (status);
}

/* SIGHUP handler. */
static void 
sighup (void)
{
  zlog_info ("SIGHUP received");

  /* Reload of config file. */
  ;
}

/* SIGINT handler. */
static void
sigint (void)
{
  zlog_notice ("Terminating on signal");

  if (!retain_mode)
    rib_close ();
#ifdef HAVE_IRDP
  irdp_finish();
#endif

  exit (0);
}

/* SIGUSR1 handler. */
static void
sigusr1 (void)
{
  zlog_rotate (NULL);
}

struct bane_signal_t kroute_signals[] =
{
  { 
    .signal = SIGHUP, 
    .handler = &sighup,
  },
  {
    .signal = SIGUSR1,
    .handler = &sigusr1,
  },
  {
    .signal = SIGINT,
    .handler = &sigint,
  },
  {
    .signal = SIGTERM,
    .handler = &sigint,
  },
};

/* Main startup routine. */
int
main (int argc, char **argv)
{
  char *p;
  char *vty_addr = NULL;
  int vty_port = KROUTE_VTY_PORT;
  int dryrun = 0;
  int batch_mode = 0;
  int daemon_mode = 0;
  char *config_file = NULL;
  char *progname;
  struct thread thread;
  char *zserv_path = NULL;

  /* Set umask before anything for security */
  umask (0027);

  /* preserve my name */
  progname = ((p = strrchr (argv[0], '/')) ? ++p : argv[0]);

  zlog_default = openzlog (progname, ZLOG_KROUTE,
			   LOG_CONS|LOG_NDELAY|LOG_PID, LOG_DAEMON);

  while (1) 
    {
      int opt;
  
#ifdef HAVE_NETLINK  
      opt = getopt_long (argc, argv, "bdkf:i:z:hA:P:ru:g:vs:C", longopts, 0);
#else
      opt = getopt_long (argc, argv, "bdkf:i:z:hA:P:ru:g:vC", longopts, 0);
#endif /* HAVE_NETLINK */

      if (opt == EOF)
	break;

      switch (opt) 
	{
	case 0:
	  break;
	case 'b':
	  batch_mode = 1;
	case 'd':
	  daemon_mode = 1;
	  break;
	case 'k':
	  keep_kernel_mode = 1;
	  break;
	case 'C':
	  dryrun = 1;
	  break;
	case 'f':
	  config_file = optarg;
	  break;
	case 'A':
	  vty_addr = optarg;
	  break;
        case 'i':
          pid_file = optarg;
          break;
	case 'z':
	  zserv_path = optarg;
	  break;
	case 'P':
	  /* Deal with atoi() returning 0 on failure, and kroute not
	     listening on kroute port... */
	  if (strcmp(optarg, "0") == 0) 
	    {
	      vty_port = 0;
	      break;
	    } 
	  vty_port = atoi (optarg);
	  if (vty_port <= 0 || vty_port > 0xffff)
	    vty_port = KROUTE_VTY_PORT;
	  break;
	case 'r':
	  retain_mode = 1;
	  break;
#ifdef HAVE_NETLINK
	case 's':
	  nl_rcvbufsize = atoi (optarg);
	  break;
#endif /* HAVE_NETLINK */
	case 'u':
	  zserv_privs.user = optarg;
	  break;
	case 'g':
	  zserv_privs.group = optarg;
	  break;
	case 'v':
	  print_version (progname);
	  exit (0);
	  break;
	case 'h':
	  usage (progname, 0);
	  break;
	default:
	  usage (progname, 1);
	  break;
	}
    }

  /* Make master thread emulator. */
  krouted.master = thread_master_create ();

  /* privs initialise */
  zprivs_init (&zserv_privs);

  /* Vty related initialize. */
  signal_init (krouted.master, Q_SIGC(kroute_signals), kroute_signals);
  cmd_init (1);
  vty_init (krouted.master);
  memory_init ();

  /* Kroute related initialize. */
  kroute_init ();
  rib_init ();
  kroute_if_init ();
  kroute_debug_init ();
  router_id_init();
  kroute_vty_init ();
  access_list_init ();
  prefix_list_init ();
  rtadv_init ();
#ifdef HAVE_IRDP
  irdp_init();
#endif

  /* For debug purpose. */
  /* SET_FLAG (kroute_debug_event, KROUTE_DEBUG_EVENT); */

  /* Make kernel routing socket. */
  kernel_init ();
  interface_list ();
  route_read ();

  /* Sort VTY commands. */
  sort_node ();

#ifdef HAVE_SNMP
  kroute_snmp_init ();
#endif /* HAVE_SNMP */

  /* Process the configuration file. Among other configuration
  *  directives we can meet those installing static routes. Such
  *  requests will not be executed immediately, but queued in
  *  kroute->ribq structure until we enter the main execution loop.
  *  The notifications from kernel will show originating PID equal
  *  to that after daemon() completes (if ever called).
  */
  vty_read_config (config_file, config_default);

  /* Don't start execution if we are in dry-run mode */
  if (dryrun)
    return(0);
  
  /* Clean up rib. */
  rib_weed_tables ();

  /* Exit when kroute is working in batch mode. */
  if (batch_mode)
    exit (0);

  /* Daemonize. */
  if (daemon_mode && daemon (0, 0) < 0)
    {
      zlog_err("Kroute daemon failed: %s", strerror(errno));
      exit (1);
    }

  /* Output pid of kroute. */
  pid_output (pid_file);

  /* After we have successfully acquired the pidfile, we can be sure
  *  about being the only copy of kroute process, which is submitting
  *  changes to the FIB.
  *  Clean up kroute-originated routes. The requests will be sent to OS
  *  immediately, so originating PID in notifications from kernel
  *  will be equal to the current getpid(). To know about such routes,
  * we have to have route_read() called before.
  */
  if (! keep_kernel_mode)
    rib_sweep_route ();

  /* Needed for BSD routing socket. */
  pid = getpid ();

  /* This must be done only after locking pidfile (bug #403). */
  kroute_zserv_socket_init (zserv_path);

  /* Make vty server socket. */
  vty_serv_sock (vty_addr, vty_port, KROUTE_VTYSH_PATH);

  /* Print banner. */
  zlog_notice ("Kroute %s starting: vty@%d", BANE_VERSION, vty_port);

  while (thread_fetch (krouted.master, &thread))
    thread_call (&thread);

  /* Not reached... */
  return 0;
}
