/* Copyright (C) 2002-2003 IP Infusion, Inc. All Rights Reserved. */

#include <kroute.h>

#include <lib/version.h>
#include "getopt.h"
#include "command.h"
#include "thread.h"
#include "memory.h"
#include "prefix.h"
#include "filter.h"
#include "log.h"
#include "privs.h"
#include "sigevent.h"
#include "zclient.h"

#include "pimd/pimd.h"

/* Global variable container. */
extern s_int32_t pim_vty_port;
int pim_start (s_int32_t, char *, s_int32_t, char *);
void pim_stop (void);

/* pimd options, we use GNU getopt library. */
struct option longopts[] = 
  {
    { "daemon",      no_argument,       NULL, 'd'},
    { "config_file", required_argument, NULL, 'f'},
    { "vty_port",    required_argument, NULL, 'P'},
    { "version",     no_argument,       NULL, 'v'},
    { "help",        no_argument,       NULL, 'h'},
    { 0 }
  };

/* pimd privileges */
kroute_capabilities_t _caps_p [] = 
{
  ZCAP_NET_RAW,
  ZCAP_BIND
};

struct kroute_privs_t pimd_privs =
{
#if defined(BANE_USER)
  .user = BANE_USER,
#endif
#if defined BANE_GROUP
  .group = BANE_GROUP,
#endif
#ifdef VTY_GROUP
  .vty_group = VTY_GROUP,
#endif
  .caps_p = _caps_p,
  .cap_num_p = 2,
  .cap_num_i = 0
};

/* Configuration file and directory. */
char config_default[] = SYSCONFDIR PIMD_DEFAULT_CONFIG;
char *config_file = NULL;

/* pimd program name */
char *progname = NULL;

/* Route retain mode flag. */
int retain_mode = 0;

/* PIM VTY bind address. */
char *vty_addr = NULL;

/* PIM VTY connection port. */
int vty_port = PIM_VTY_PORT;

/* Master of threads. */
struct thread_master *master;

/* Process ID saved for use by init system */
const char *pid_file = PATH_PIMD_PID;

static void
pim_usage (int status, char *progname)
{
  if (status != 0)
    fprintf (stderr, "Try `%s --help' for more information.\n", progname);
  else
    {    
      printf ("Usage : %s [OPTION...]\n\n\
Daemon which manages kernel routing table management and \
redistribution between different routing protocols.\n\n\
-d, --daemon       Runs in daemon mode\n\
-f, --config_file  Set configuration file name\n\
-P, --vty_port     Set vty's port number\n\
-v, --version      Print program version\n\
-h, --help         Display this help and exit\n\
\n\
Report bugs to %s\n", progname, KROUTE_BUG_ADDRESS);
    }

  exit (status);
}

/* SIGHUP handler. */
void 
sighup (int sig)
{
  zlog_info ("SIGHUP received");

  /* Terminate all thread. */
  zlog_info ("pimd restarting!");

  /* Try to return to normal operation. */
}

/* SIGINT handler. */
void
sigint (int sig)
{
  zlog_info ("Terminating on signal");

  /* Stop PIM-SM module.  */
  pim_stop ();

  exit (0);
}

/* SIGUSR1 handler. */
void
sigusr1 (int sig)
{
  zlog_rotate (NULL);
}

static struct bane_signal_t pimd_signals[] =
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

int
main (int argc, char **argv)
{
  result_t ret;
  char *p;
  s_int32_t daemon_mode = 0;
  s_int32_t opt;
	
  /* Set umask before anything for security */
  umask (0027);

#ifdef VTYSH
  /* Unlink vtysh domain socket. */
  unlink (PIM_VTYSH_PATH);
#endif /* VTYSH */

  /* Preserve name of myself. */
  progname = ((p = pal_strrchr (argv[0], '/')) ? ++p : argv[0]);

  /* First of all we need logging init. */
  zlog_default = openzlog (progname, ZLOG_PIM,
			   LOG_CONS|LOG_NDELAY|LOG_PID, LOG_DAEMON);

  /* Command line argument treatment. */
  while (1) 
    {
      opt = getopt_long (argc, argv, "df:hp:P:rv", longopts, 0);
    
      if (opt == EOF)
        break;

      switch (opt) 
        {
        case 0:
          break;
        case 'd':
          daemon_mode = 1;
          break;
        case 'f':
          config_file = optarg;
          break;
        case 'P':
          vty_port = atoi (optarg);
          break;
        case 'v':
          print_version (progname);
          exit (0);
        case 'h':
          pim_usage (0, progname);
          break;
        default:
          pim_usage (1, progname);
          break;
        }
    }

  /* Prepare master thread. */
  master = thread_master_create ();

  /* Library initialization. */
  zprivs_init (&pimd_privs);
  signal_init (master, Q_SIGC(pimd_signals), pimd_signals);
  cmd_init (1);
  vty_init (master);
  memory_init ();

  /* PIM related initialization. */
  pim_init();
  /* Sort all installed commands. */
  sort_node ();

  /* Get configuration file. */
  vty_read_config (config_file, config_default);
  
  /* Change to the daemon program. */
  if (daemon_mode && daemon (0, 0) < 0)
    {
      zlog_err("PIMd daemon failed: %s", strerror(errno));
      exit (1);
    }

  /* Pid file create. */
  pid_output (pid_file);

  /* Create VTY's socket */
  vty_serv_sock (vty_addr, vty_port, PIM_VTYSH_PATH);

  /* Print banner. */
  zlog_notice ("PIMd %s starting: vty@%d", BANE_VERSION, vty_port);

  /* Execute each thread. */
  while (thread_fetch (master, &thread))
    thread_call (&thread);

  /* Not reached. */
  exit (0);
}

