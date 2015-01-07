/*
 * RIPngd and kroute interface.
 * Copyright (C) 1998, 1999 Kunihiro Ishiguro
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

#include "command.h"
#include "prefix.h"
#include "stream.h"
#include "routemap.h"
#include "zclient.h"
#include "log.h"

#include "ripngd/ripngd.h"

/* All information about kroute. */
struct zclient *zclient = NULL;

/* Callback prototypes for kroute client service. */
int ripng_interface_up (int, struct zclient *, kroute_size_t);
int ripng_interface_down (int, struct zclient *, kroute_size_t);
int ripng_interface_add (int, struct zclient *, kroute_size_t);
int ripng_interface_delete (int, struct zclient *, kroute_size_t);
int ripng_interface_address_add (int, struct zclient *, kroute_size_t);
int ripng_interface_address_delete (int, struct zclient *, kroute_size_t);

void
ripng_kroute_ipv6_add (struct prefix_ipv6 *p, struct in6_addr *nexthop,
		      unsigned int ifindex, u_char metric)
{
  struct zapi_ipv6 api;

  if (zclient->redist[KROUTE_ROUTE_RIPNG])
    {
      api.type = KROUTE_ROUTE_RIPNG;
      api.flags = 0;
      api.message = 0;
      api.safi = SAFI_UNICAST;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
      api.ifindex_num = 1;
      api.ifindex = &ifindex;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = metric;
      
      zapi_ipv6_route (KROUTE_IPV6_ROUTE_ADD, zclient, p, &api);
    }
}

void
ripng_kroute_ipv6_delete (struct prefix_ipv6 *p, struct in6_addr *nexthop,
			 unsigned int ifindex)
{
  struct zapi_ipv6 api;

  if (zclient->redist[KROUTE_ROUTE_RIPNG])
    {
      api.type = KROUTE_ROUTE_RIPNG;
      api.flags = 0;
      api.message = 0;
      api.safi = SAFI_UNICAST;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
      api.ifindex_num = 1;
      api.ifindex = &ifindex;

      zapi_ipv6_route (KROUTE_IPV6_ROUTE_DELETE, zclient, p, &api);
    }
}

/* Kroute route add and delete treatment. */
static int
ripng_kroute_read_ipv6 (int command, struct zclient *zclient,
		       kroute_size_t length)
{
  struct stream *s;
  struct zapi_ipv6 api;
  unsigned long ifindex;
  struct in6_addr nexthop;
  struct prefix_ipv6 p;

  s = zclient->ibuf;
  ifindex = 0;
  memset (&nexthop, 0, sizeof (struct in6_addr));

  /* Type, flags, message. */
  api.type = stream_getc (s);
  api.flags = stream_getc (s);
  api.message = stream_getc (s);

  /* IPv6 prefix. */
  memset (&p, 0, sizeof (struct prefix_ipv6));
  p.family = AF_INET6;
  p.prefixlen = stream_getc (s);
  stream_get (&p.prefix, s, PSIZE (p.prefixlen));

  /* Nexthop, ifindex, distance, metric. */
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP))
    {
      api.nexthop_num = stream_getc (s);
      stream_get (&nexthop, s, 16);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_IFINDEX))
    {
      api.ifindex_num = stream_getc (s);
      ifindex = stream_getl (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_DISTANCE))
    api.distance = stream_getc (s);
  else
    api.distance = 0;
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_METRIC))
    api.metric = stream_getl (s);
  else
    api.metric = 0;

  if (command == KROUTE_IPV6_ROUTE_ADD)
    ripng_redistribute_add (api.type, RIPNG_ROUTE_REDISTRIBUTE, &p, ifindex, &nexthop);
  else
    ripng_redistribute_delete (api.type, RIPNG_ROUTE_REDISTRIBUTE, &p, ifindex);

  return 0;
}

void
ripng_zclient_reset (void)
{
  zclient_reset (zclient);
}

static int
ripng_redistribute_unset (int type)
{
  if (! zclient->redist[type])
    return CMD_SUCCESS;

  zclient->redist[type] = 0;

  if (zclient->sock > 0)
    kroute_redistribute_send (KROUTE_REDISTRIBUTE_DELETE, zclient, type);

  ripng_redistribute_withdraw (type);
  
  return CMD_SUCCESS;
}

int
ripng_redistribute_check (int type)
{
  return (zclient->redist[type]);
}

static void
ripng_redistribute_metric_set (int type, int metric)
{
  ripng->route_map[type].metric_config = 1;
  ripng->route_map[type].metric = metric;
}

static int
ripng_redistribute_metric_unset (int type)
{
  ripng->route_map[type].metric_config = 0;
  ripng->route_map[type].metric = 0;
  return 0;
}

static void
ripng_redistribute_routemap_set (int type, const char *name)
{
  if (ripng->route_map[type].name)
    free (ripng->route_map[type].name);

  ripng->route_map[type].name = strdup (name);
  ripng->route_map[type].map = route_map_lookup_by_name (name);
}

static void
ripng_redistribute_routemap_unset (int type)
{
  if (ripng->route_map[type].name)
    free (ripng->route_map[type].name);

  ripng->route_map[type].name = NULL;
  ripng->route_map[type].map = NULL;
}

/* Redistribution types */
static struct {
  int type;
  int str_min_len;
  const char *str;
} redist_type[] = {
  {KROUTE_ROUTE_KERNEL,  1, "kernel"},
  {KROUTE_ROUTE_CONNECT, 1, "connected"},
  {KROUTE_ROUTE_STATIC,  1, "static"},
  {KROUTE_ROUTE_OSPF6,   1, "ospf6"},
  {KROUTE_ROUTE_BGP,     2, "bgp"},
  {KROUTE_ROUTE_BABEL,   2, "babel"},
  {0, 0, NULL}
};

void
ripng_redistribute_clean ()
{
  int i;

  for (i = 0; redist_type[i].str; i++)
    {
      if (zclient->redist[redist_type[i].type])
        {
          if (zclient->sock > 0)
            kroute_redistribute_send (KROUTE_REDISTRIBUTE_DELETE,
                                     zclient, redist_type[i].type);

          zclient->redist[redist_type[i].type] = 0;

          /* Remove the routes from RIPng table. */
          ripng_redistribute_withdraw (redist_type[i].type);
        }
    }
}

DEFUN (router_kroute,
       router_kroute_cmd,
       "router kroute",
       "Enable a routing process\n"
       "Make connection to kroute daemon\n")
{
  vty->node = KROUTE_NODE;
  zclient->enable = 1;
  zclient_start (zclient);
  return CMD_SUCCESS;
}

DEFUN (no_router_kroute,
       no_router_kroute_cmd,
       "no router kroute",
       NO_STR
       "Disable a routing process\n"
       "Stop connection to kroute daemon\n")
{
  zclient->enable = 0;
  zclient_stop (zclient);
  return CMD_SUCCESS;
}

DEFUN (ripng_redistribute_ripng,
       ripng_redistribute_ripng_cmd,
       "redistribute ripng",
       "Redistribute information from another routing protocol\n"
       "RIPng route\n")
{
  zclient->redist[KROUTE_ROUTE_RIPNG] = 1;
  return CMD_SUCCESS;
}

DEFUN (no_ripng_redistribute_ripng,
       no_ripng_redistribute_ripng_cmd,
       "no redistribute ripng",
       NO_STR
       "Redistribute information from another routing protocol\n"
       "RIPng route\n")
{
  zclient->redist[KROUTE_ROUTE_RIPNG] = 0;
  return CMD_SUCCESS;
}

DEFUN (ripng_redistribute_type,
       ripng_redistribute_type_cmd,
       "redistribute " BANE_REDIST_STR_RIPNGD,
       "Redistribute\n"
       BANE_REDIST_HELP_STR_RIPNGD)
{
  int type;

  type = proto_redistnum(AFI_IP6, argv[0]);

  if (type < 0)
    {
      vty_out(vty, "Invalid type %s%s", argv[0], VTY_NEWLINE);
      return CMD_WARNING;
    }

  zclient_redistribute (KROUTE_REDISTRIBUTE_ADD, zclient, type);
  return CMD_SUCCESS;
}

DEFUN (no_ripng_redistribute_type,
       no_ripng_redistribute_type_cmd,
       "no redistribute " BANE_REDIST_STR_RIPNGD,
       NO_STR
       "Redistribute\n"
       BANE_REDIST_HELP_STR_RIPNGD)
{
  int type;

  type = proto_redistnum(AFI_IP6, argv[0]);

  if (type < 0)
    {
      vty_out(vty, "Invalid type %s%s", argv[0], VTY_NEWLINE);
      return CMD_WARNING;
    }

  ripng_redistribute_metric_unset (type);
  ripng_redistribute_routemap_unset (type);
  return ripng_redistribute_unset (type);
}


DEFUN (ripng_redistribute_type_metric,
       ripng_redistribute_type_metric_cmd,
       "redistribute " BANE_REDIST_STR_RIPNGD " metric <0-16>",
       "Redistribute\n"
       BANE_REDIST_HELP_STR_RIPNGD
       "Metric\n"
       "Metric value\n")
{
  int type;
  int metric;

  metric = atoi (argv[1]);
  type = proto_redistnum(AFI_IP6, argv[0]);

  if (type < 0)
    {
      vty_out(vty, "Invalid type %s%s", argv[0], VTY_NEWLINE);
      return CMD_WARNING;
    }

  ripng_redistribute_metric_set (type, metric);
  zclient_redistribute (KROUTE_REDISTRIBUTE_ADD, zclient, type);
  return CMD_SUCCESS;
}

ALIAS (no_ripng_redistribute_type,
       no_ripng_redistribute_type_metric_cmd,
       "no redistribute " BANE_REDIST_STR_RIPNGD " metric <0-16>",
       NO_STR
       "Redistribute\n"
       BANE_REDIST_HELP_STR_RIPNGD
       "Metric\n"
       "Metric value\n")

DEFUN (ripng_redistribute_type_routemap,
       ripng_redistribute_type_routemap_cmd,
       "redistribute " BANE_REDIST_STR_RIPNGD " route-map WORD",
       "Redistribute\n"
       BANE_REDIST_HELP_STR_RIPNGD
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int type;

  type = proto_redistnum(AFI_IP6, argv[0]);

  if (type < 0)
    {
      vty_out(vty, "Invalid type %s%s", argv[0], VTY_NEWLINE);
      return CMD_WARNING;
    }

  ripng_redistribute_routemap_set (type, argv[1]);
  zclient_redistribute (KROUTE_REDISTRIBUTE_ADD, zclient, type);
 return CMD_SUCCESS;
}

ALIAS (no_ripng_redistribute_type,
       no_ripng_redistribute_type_routemap_cmd,
       "no redistribute " BANE_REDIST_STR_RIPNGD " route-map WORD",
       NO_STR
       "Redistribute\n"
       BANE_REDIST_HELP_STR_RIPNGD
       "Route map reference\n"
       "Pointer to route-map entries\n")

DEFUN (ripng_redistribute_type_metric_routemap,
       ripng_redistribute_type_metric_routemap_cmd,
       "redistribute " BANE_REDIST_STR_RIPNGD " metric <0-16> route-map WORD",
       "Redistribute\n"
       BANE_REDIST_HELP_STR_RIPNGD
       "Metric\n"
       "Metric value\n"
       "Route map reference\n"
       "Pointer to route-map entries\n")
{
  int type;
  int metric;

  type = proto_redistnum(AFI_IP6, argv[0]);
  metric = atoi (argv[1]);

  if (type < 0)
    {
      vty_out(vty, "Invalid type %s%s", argv[0], VTY_NEWLINE);
      return CMD_WARNING;
    }

  ripng_redistribute_metric_set (type, metric);
  ripng_redistribute_routemap_set (type, argv[2]);
  zclient_redistribute (KROUTE_REDISTRIBUTE_ADD, zclient, type);
  return CMD_SUCCESS;
}

ALIAS (no_ripng_redistribute_type,
       no_ripng_redistribute_type_metric_routemap_cmd,
       "no redistribute " BANE_REDIST_STR_RIPNGD " metric <0-16> route-map WORD",
       NO_STR
       "Redistribute\n"
       BANE_REDIST_HELP_STR_RIPNGD
       "Route map reference\n"
       "Pointer to route-map entries\n")

void
ripng_redistribute_write (struct vty *vty, int config_mode)
{
  int i;

  for (i = 0; i < KROUTE_ROUTE_MAX; i++)
    if (i != zclient->redist_default && zclient->redist[i])
      {
      if (config_mode)
	{
	  if (ripng->route_map[i].metric_config)
	    {
	      if (ripng->route_map[i].name)
		vty_out (vty, " redistribute %s metric %d route-map %s%s",
			 kroute_route_string(i), ripng->route_map[i].metric,
			ripng->route_map[i].name, VTY_NEWLINE);
	      else
		vty_out (vty, " redistribute %s metric %d%s",
			kroute_route_string(i), ripng->route_map[i].metric,
			VTY_NEWLINE);
	    }
	  else
	    {
	      if (ripng->route_map[i].name)
		vty_out (vty, " redistribute %s route-map %s%s",
			 kroute_route_string(i), ripng->route_map[i].name,
			 VTY_NEWLINE);
	      else
		vty_out (vty, " redistribute %s%s", kroute_route_string(i),
			 VTY_NEWLINE);
	    }
	}
      else
	vty_out (vty, "    %s", kroute_route_string(i));
      }
}

/* RIPng configuration write function. */
static int
kroute_config_write (struct vty *vty)
{
  if (! zclient->enable)
    {
      vty_out (vty, "no router kroute%s", VTY_NEWLINE);
      return 1;
    }
  else if (! zclient->redist[KROUTE_ROUTE_RIPNG])
    {
      vty_out (vty, "router kroute%s", VTY_NEWLINE);
      vty_out (vty, " no redistribute ripng%s", VTY_NEWLINE);
      return 1;
    }
  return 0;
}

/* Kroute node structure. */
static struct cmd_node kroute_node =
{
  KROUTE_NODE,
  "%s(config-router)# ",
};

/* Initialize kroute structure and it's commands. */
void
kroute_init ()
{
  /* Allocate kroute structure. */
  zclient = zclient_new ();
  zclient_init (zclient, KROUTE_ROUTE_RIPNG);

  zclient->interface_up = ripng_interface_up;
  zclient->interface_down = ripng_interface_down;
  zclient->interface_add = ripng_interface_add;
  zclient->interface_delete = ripng_interface_delete;
  zclient->interface_address_add = ripng_interface_address_add;
  zclient->interface_address_delete = ripng_interface_address_delete;
  zclient->ipv6_route_add = ripng_kroute_read_ipv6;
  zclient->ipv6_route_delete = ripng_kroute_read_ipv6;
  
  /* Install kroute node. */
  install_node (&kroute_node, kroute_config_write);

  /* Install command element for kroute node. */ 
  install_element (CONFIG_NODE, &router_kroute_cmd);
  install_element (CONFIG_NODE, &no_router_kroute_cmd);
  install_default (KROUTE_NODE);
  install_element (KROUTE_NODE, &ripng_redistribute_ripng_cmd);
  install_element (KROUTE_NODE, &no_ripng_redistribute_ripng_cmd);

  /* Install command elements to ripng node */
  install_element (RIPNG_NODE, &ripng_redistribute_type_cmd);
  install_element (RIPNG_NODE, &ripng_redistribute_type_routemap_cmd);
  install_element (RIPNG_NODE, &ripng_redistribute_type_metric_cmd);
  install_element (RIPNG_NODE, &ripng_redistribute_type_metric_routemap_cmd);
  install_element (RIPNG_NODE, &no_ripng_redistribute_type_cmd);
  install_element (RIPNG_NODE, &no_ripng_redistribute_type_routemap_cmd);
  install_element (RIPNG_NODE, &no_ripng_redistribute_type_metric_cmd);
  install_element (RIPNG_NODE, &no_ripng_redistribute_type_metric_routemap_cmd);
}
