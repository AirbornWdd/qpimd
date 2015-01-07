/*
 * Kroute debug related function
 * Copyright (C) 1999 Kunihiro Ishiguro
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
 * along with GNU Kroute; see the file COPYING.  If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.  
 */

#include <kroute.h>
#include "command.h"
#include "debug.h"

/* For debug statement. */
unsigned long kroute_debug_event;
unsigned long kroute_debug_packet;
unsigned long kroute_debug_kernel;
unsigned long kroute_debug_rib;

DEFUN (show_debugging_kroute,
       show_debugging_kroute_cmd,
       "show debugging kroute",
       SHOW_STR
       "Kroute configuration\n"
       "Debugging information\n")
{
  vty_out (vty, "Kroute debugging status:%s", VTY_NEWLINE);

  if (IS_KROUTE_DEBUG_EVENT)
    vty_out (vty, "  Kroute event debugging is on%s", VTY_NEWLINE);

  if (IS_KROUTE_DEBUG_PACKET)
    {
      if (IS_KROUTE_DEBUG_SEND && IS_KROUTE_DEBUG_RECV)
	{
	  vty_out (vty, "  Kroute packet%s debugging is on%s",
		   IS_KROUTE_DEBUG_DETAIL ? " detail" : "",
		   VTY_NEWLINE);
	}
      else
	{
	  if (IS_KROUTE_DEBUG_SEND)
	    vty_out (vty, "  Kroute packet send%s debugging is on%s",
		     IS_KROUTE_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	  else
	    vty_out (vty, "  Kroute packet receive%s debugging is on%s",
		     IS_KROUTE_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	}
    }

  if (IS_KROUTE_DEBUG_KERNEL)
    vty_out (vty, "  Kroute kernel debugging is on%s", VTY_NEWLINE);

  if (IS_KROUTE_DEBUG_RIB)
    vty_out (vty, "  Kroute RIB debugging is on%s", VTY_NEWLINE);
  if (IS_KROUTE_DEBUG_RIB_Q)
    vty_out (vty, "  Kroute RIB queue debugging is on%s", VTY_NEWLINE);

  return CMD_SUCCESS;
}

DEFUN (debug_kroute_events,
       debug_kroute_events_cmd,
       "debug kroute events",
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute events\n")
{
  kroute_debug_event = KROUTE_DEBUG_EVENT;
  return CMD_WARNING;
}

DEFUN (debug_kroute_packet,
       debug_kroute_packet_cmd,
       "debug kroute packet",
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute packet\n")
{
  kroute_debug_packet = KROUTE_DEBUG_PACKET;
  kroute_debug_packet |= KROUTE_DEBUG_SEND;
  kroute_debug_packet |= KROUTE_DEBUG_RECV;
  return CMD_SUCCESS;
}

DEFUN (debug_kroute_packet_direct,
       debug_kroute_packet_direct_cmd,
       "debug kroute packet (recv|send)",
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n")
{
  kroute_debug_packet = KROUTE_DEBUG_PACKET;
  if (strncmp ("send", argv[0], strlen (argv[0])) == 0)
    kroute_debug_packet |= KROUTE_DEBUG_SEND;
  if (strncmp ("recv", argv[0], strlen (argv[0])) == 0)
    kroute_debug_packet |= KROUTE_DEBUG_RECV;
  kroute_debug_packet &= ~KROUTE_DEBUG_DETAIL;
  return CMD_SUCCESS;
}

DEFUN (debug_kroute_packet_detail,
       debug_kroute_packet_detail_cmd,
       "debug kroute packet (recv|send) detail",
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n"
       "Debug option set detaied information\n")
{
  kroute_debug_packet = KROUTE_DEBUG_PACKET;
  if (strncmp ("send", argv[0], strlen (argv[0])) == 0)
    kroute_debug_packet |= KROUTE_DEBUG_SEND;
  if (strncmp ("recv", argv[0], strlen (argv[0])) == 0)
    kroute_debug_packet |= KROUTE_DEBUG_RECV;
  kroute_debug_packet |= KROUTE_DEBUG_DETAIL;
  return CMD_SUCCESS;
}

DEFUN (debug_kroute_kernel,
       debug_kroute_kernel_cmd,
       "debug kroute kernel",
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute between kernel interface\n")
{
  kroute_debug_kernel = KROUTE_DEBUG_KERNEL;
  return CMD_SUCCESS;
}

DEFUN (debug_kroute_rib,
       debug_kroute_rib_cmd,
       "debug kroute rib",
       DEBUG_STR
       "Kroute configuration\n"
       "Debug RIB events\n")
{
  SET_FLAG (kroute_debug_rib, KROUTE_DEBUG_RIB);
  return CMD_SUCCESS;
}

DEFUN (debug_kroute_rib_q,
       debug_kroute_rib_q_cmd,
       "debug kroute rib queue",
       DEBUG_STR
       "Kroute configuration\n"
       "Debug RIB events\n"
       "Debug RIB queueing\n")
{
  SET_FLAG (kroute_debug_rib, KROUTE_DEBUG_RIB_Q);
  return CMD_SUCCESS;
}

DEFUN (no_debug_kroute_events,
       no_debug_kroute_events_cmd,
       "no debug kroute events",
       NO_STR
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute events\n")
{
  kroute_debug_event = 0;
  return CMD_SUCCESS;
}

DEFUN (no_debug_kroute_packet,
       no_debug_kroute_packet_cmd,
       "no debug kroute packet",
       NO_STR
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute packet\n")
{
  kroute_debug_packet = 0;
  return CMD_SUCCESS;
}

DEFUN (no_debug_kroute_packet_direct,
       no_debug_kroute_packet_direct_cmd,
       "no debug kroute packet (recv|send)",
       NO_STR
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute packet\n"
       "Debug option set for receive packet\n"
       "Debug option set for send packet\n")
{
  if (strncmp ("send", argv[0], strlen (argv[0])) == 0)
    kroute_debug_packet &= ~KROUTE_DEBUG_SEND;
  if (strncmp ("recv", argv[0], strlen (argv[0])) == 0)
    kroute_debug_packet &= ~KROUTE_DEBUG_RECV;
  return CMD_SUCCESS;
}

DEFUN (no_debug_kroute_kernel,
       no_debug_kroute_kernel_cmd,
       "no debug kroute kernel",
       NO_STR
       DEBUG_STR
       "Kroute configuration\n"
       "Debug option set for kroute between kernel interface\n")
{
  kroute_debug_kernel = 0;
  return CMD_SUCCESS;
}

DEFUN (no_debug_kroute_rib,
       no_debug_kroute_rib_cmd,
       "no debug kroute rib",
       NO_STR
       DEBUG_STR
       "Kroute configuration\n"
       "Debug kroute RIB\n")
{
  kroute_debug_rib = 0;
  return CMD_SUCCESS;
}

DEFUN (no_debug_kroute_rib_q,
       no_debug_kroute_rib_q_cmd,
       "no debug kroute rib queue",
       NO_STR
       DEBUG_STR
       "Kroute configuration\n"
       "Debug kroute RIB\n"
       "Debug RIB queueing\n")
{
  UNSET_FLAG (kroute_debug_rib, KROUTE_DEBUG_RIB_Q);
  return CMD_SUCCESS;
}

/* Debug node. */
struct cmd_node debug_node =
{
  DEBUG_NODE,
  "",				/* Debug node has no interface. */
  1
};

static int
config_write_debug (struct vty *vty)
{
  int write = 0;

  if (IS_KROUTE_DEBUG_EVENT)
    {
      vty_out (vty, "debug kroute events%s", VTY_NEWLINE);
      write++;
    }
  if (IS_KROUTE_DEBUG_PACKET)
    {
      if (IS_KROUTE_DEBUG_SEND && IS_KROUTE_DEBUG_RECV)
	{
	  vty_out (vty, "debug kroute packet%s%s",
		   IS_KROUTE_DEBUG_DETAIL ? " detail" : "",
		   VTY_NEWLINE);
	  write++;
	}
      else
	{
	  if (IS_KROUTE_DEBUG_SEND)
	    vty_out (vty, "debug kroute packet send%s%s",
		     IS_KROUTE_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	  else
	    vty_out (vty, "debug kroute packet recv%s%s",
		     IS_KROUTE_DEBUG_DETAIL ? " detail" : "",
		     VTY_NEWLINE);
	  write++;
	}
    }
  if (IS_KROUTE_DEBUG_KERNEL)
    {
      vty_out (vty, "debug kroute kernel%s", VTY_NEWLINE);
      write++;
    }
  if (IS_KROUTE_DEBUG_RIB)
    {
      vty_out (vty, "debug kroute rib%s", VTY_NEWLINE);
      write++;
    }
  if (IS_KROUTE_DEBUG_RIB_Q)
    {
      vty_out (vty, "debug kroute rib queue%s", VTY_NEWLINE);
      write++;
    }
  return write;
}

void
kroute_debug_init (void)
{
  kroute_debug_event = 0;
  kroute_debug_packet = 0;
  kroute_debug_kernel = 0;
  kroute_debug_rib = 0;

  install_node (&debug_node, config_write_debug);

  install_element (VIEW_NODE, &show_debugging_kroute_cmd);

  install_element (ENABLE_NODE, &show_debugging_kroute_cmd);
  install_element (ENABLE_NODE, &debug_kroute_events_cmd);
  install_element (ENABLE_NODE, &debug_kroute_packet_cmd);
  install_element (ENABLE_NODE, &debug_kroute_packet_direct_cmd);
  install_element (ENABLE_NODE, &debug_kroute_packet_detail_cmd);
  install_element (ENABLE_NODE, &debug_kroute_kernel_cmd);
  install_element (ENABLE_NODE, &debug_kroute_rib_cmd);
  install_element (ENABLE_NODE, &debug_kroute_rib_q_cmd);
  install_element (ENABLE_NODE, &no_debug_kroute_events_cmd);
  install_element (ENABLE_NODE, &no_debug_kroute_packet_cmd);
  install_element (ENABLE_NODE, &no_debug_kroute_kernel_cmd);
  install_element (ENABLE_NODE, &no_debug_kroute_rib_cmd);
  install_element (ENABLE_NODE, &no_debug_kroute_rib_q_cmd);

  install_element (CONFIG_NODE, &debug_kroute_events_cmd);
  install_element (CONFIG_NODE, &debug_kroute_packet_cmd);
  install_element (CONFIG_NODE, &debug_kroute_packet_direct_cmd);
  install_element (CONFIG_NODE, &debug_kroute_packet_detail_cmd);
  install_element (CONFIG_NODE, &debug_kroute_kernel_cmd);
  install_element (CONFIG_NODE, &debug_kroute_rib_cmd);
  install_element (CONFIG_NODE, &debug_kroute_rib_q_cmd);
  install_element (CONFIG_NODE, &no_debug_kroute_events_cmd);
  install_element (CONFIG_NODE, &no_debug_kroute_packet_cmd);
  install_element (CONFIG_NODE, &no_debug_kroute_kernel_cmd);
  install_element (CONFIG_NODE, &no_debug_kroute_rib_cmd);
  install_element (CONFIG_NODE, &no_debug_kroute_rib_q_cmd);
}
