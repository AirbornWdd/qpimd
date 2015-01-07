/*
 * IS-IS Rout(e)ing protocol - isis_kroute.c   
 *
 * Copyright (C) 2001,2002   Sampo Saaristo
 *                           Tampere University of Technology      
 *                           Institute of Communications Engineering
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public Licenseas published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
 * more details.

 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <kroute.h>

#include "thread.h"
#include "command.h"
#include "memory.h"
#include "log.h"
#include "if.h"
#include "network.h"
#include "prefix.h"
#include "zclient.h"
#include "stream.h"
#include "linklist.h"

#include "isisd/dict.h"
#include "isisd/isis_constants.h"
#include "isisd/isis_common.h"
#include "isisd/isis_flags.h"
#include "isisd/isis_misc.h"
#include "isisd/isis_circuit.h"
#include "isisd/isis_tlv.h"
#include "isisd/isisd.h"
#include "isisd/isis_circuit.h"
#include "isisd/isis_csm.h"
#include "isisd/isis_lsp.h"
#include "isisd/isis_route.h"
#include "isisd/isis_kroute.h"

struct zclient *zclient = NULL;

/* Router-id update message from kroute. */
static int
isis_router_id_update_kroute (int command, struct zclient *zclient,
			     kroute_size_t length)
{
  struct isis_area *area;
  struct listnode *node;
  struct prefix router_id;

  kroute_router_id_update_read (zclient->ibuf, &router_id);
  if (isis->router_id == router_id.u.prefix4.s_addr)
    return 0;

  isis->router_id = router_id.u.prefix4.s_addr;
  for (ALL_LIST_ELEMENTS_RO (isis->area_list, node, area))
    if (listcount (area->area_addrs) > 0)
      lsp_regenerate_schedule (area, area->is_type, 0);

  return 0;
}

static int
isis_kroute_if_add (int command, struct zclient *zclient, kroute_size_t length)
{
  struct interface *ifp;

  ifp = kroute_interface_add_read (zclient->ibuf);

  if (isis->debugs & DEBUG_KROUTE)
    zlog_debug ("Kroute I/F add: %s index %d flags %ld metric %d mtu %d",
		ifp->name, ifp->ifindex, (long)ifp->flags, ifp->metric, ifp->mtu);

  if (if_is_operative (ifp))
    isis_csm_state_change (IF_UP_FROM_Z, circuit_scan_by_ifp (ifp), ifp);

  return 0;
}

static int
isis_kroute_if_del (int command, struct zclient *zclient, kroute_size_t length)
{
  struct interface *ifp;
  struct stream *s;

  s = zclient->ibuf;
  ifp = kroute_interface_state_read (s);

  if (!ifp)
    return 0;

  if (if_is_operative (ifp))
    zlog_warn ("Kroute: got delete of %s, but interface is still up",
	       ifp->name);

  if (isis->debugs & DEBUG_KROUTE)
    zlog_debug ("Kroute I/F delete: %s index %d flags %ld metric %d mtu %d",
		ifp->name, ifp->ifindex, (long)ifp->flags, ifp->metric, ifp->mtu);

  isis_csm_state_change (IF_DOWN_FROM_Z, circuit_scan_by_ifp (ifp), ifp);

  /* Cannot call if_delete because we should retain the pseudo interface
     in case there is configuration info attached to it. */
  if_delete_retain(ifp);

  ifp->ifindex = IFINDEX_INTERNAL;

  return 0;
}

static int
isis_kroute_if_state_up (int command, struct zclient *zclient,
			kroute_size_t length)
{
  struct interface *ifp;

  ifp = kroute_interface_state_read (zclient->ibuf);

  if (ifp == NULL)
    return 0;

  isis_csm_state_change (IF_UP_FROM_Z, circuit_scan_by_ifp (ifp), ifp);

  return 0;
}

static int
isis_kroute_if_state_down (int command, struct zclient *zclient,
			  kroute_size_t length)
{
  struct interface *ifp;
  struct isis_circuit *circuit;

  ifp = kroute_interface_state_read (zclient->ibuf);

  if (ifp == NULL)
    return 0;

  circuit = isis_csm_state_change (IF_DOWN_FROM_Z, circuit_scan_by_ifp (ifp),
                                   ifp);
  if (circuit)
    SET_FLAG(circuit->flags, ISIS_CIRCUIT_FLAPPED_AFTER_SPF);

  return 0;
}

static int
isis_kroute_if_address_add (int command, struct zclient *zclient,
			   kroute_size_t length)
{
  struct connected *c;
  struct prefix *p;
  char buf[BUFSIZ];

  c = kroute_interface_address_read (KROUTE_INTERFACE_ADDRESS_ADD,
				    zclient->ibuf);

  if (c == NULL)
    return 0;

  p = c->address;

  prefix2str (p, buf, BUFSIZ);
#ifdef EXTREME_DEBUG
  if (p->family == AF_INET)
    zlog_debug ("connected IP address %s", buf);
#ifdef HAVE_IPV6
  if (p->family == AF_INET6)
    zlog_debug ("connected IPv6 address %s", buf);
#endif /* HAVE_IPV6 */
#endif /* EXTREME_DEBUG */
  if (if_is_operative (c->ifp))
    isis_circuit_add_addr (circuit_scan_by_ifp (c->ifp), c);

  return 0;
}

static int
isis_kroute_if_address_del (int command, struct zclient *client,
			   kroute_size_t length)
{
  struct connected *c;
  struct interface *ifp;
#ifdef EXTREME_DEBUG
  struct prefix *p;
  u_char buf[BUFSIZ];
#endif /* EXTREME_DEBUG */

  c = kroute_interface_address_read (KROUTE_INTERFACE_ADDRESS_DELETE,
				    zclient->ibuf);

  if (c == NULL)
    return 0;

  ifp = c->ifp;

#ifdef EXTREME_DEBUG
  p = c->address;
  prefix2str (p, buf, BUFSIZ);

  if (p->family == AF_INET)
    zlog_debug ("disconnected IP address %s", buf);
#ifdef HAVE_IPV6
  if (p->family == AF_INET6)
    zlog_debug ("disconnected IPv6 address %s", buf);
#endif /* HAVE_IPV6 */
#endif /* EXTREME_DEBUG */

  if (if_is_operative (ifp))
    isis_circuit_del_addr (circuit_scan_by_ifp (ifp), c);
  connected_free (c);

  return 0;
}

static void
isis_kroute_route_add_ipv4 (struct prefix *prefix,
			   struct isis_route_info *route_info)
{
  u_char message, flags;
  int psize;
  struct stream *stream;
  struct isis_nexthop *nexthop;
  struct listnode *node;

  if (CHECK_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_SYNCED))
    return;

  if (zclient->redist[KROUTE_ROUTE_ISIS])
    {
      message = 0;
      flags = 0;

      SET_FLAG (message, ZAPI_MESSAGE_NEXTHOP);
      SET_FLAG (message, ZAPI_MESSAGE_METRIC);
#if 0
      SET_FLAG (message, ZAPI_MESSAGE_DISTANCE);
#endif

      stream = zclient->obuf;
      stream_reset (stream);
      zclient_create_header (stream, KROUTE_IPV4_ROUTE_ADD);
      /* type */
      stream_putc (stream, KROUTE_ROUTE_ISIS);
      /* flags */
      stream_putc (stream, flags);
      /* message */
      stream_putc (stream, message);
      /* SAFI */
      stream_putw (stream, SAFI_UNICAST);
      /* prefix information */
      psize = PSIZE (prefix->prefixlen);
      stream_putc (stream, prefix->prefixlen);
      stream_write (stream, (u_char *) & prefix->u.prefix4, psize);

      stream_putc (stream, listcount (route_info->nexthops));

      /* Nexthop, ifindex, distance and metric information */
      for (ALL_LIST_ELEMENTS_RO (route_info->nexthops, node, nexthop))
	{
	  /* FIXME: can it be ? */
	  if (nexthop->ip.s_addr != INADDR_ANY)
	    {
	      stream_putc (stream, KROUTE_NEXTHOP_IPV4);
	      stream_put_in_addr (stream, &nexthop->ip);
	    }
	  else
	    {
	      stream_putc (stream, KROUTE_NEXTHOP_IFINDEX);
	      stream_putl (stream, nexthop->ifindex);
	    }
	}
#if 0
      if (CHECK_FLAG (message, ZAPI_MESSAGE_DISTANCE))
	stream_putc (stream, route_info->depth);
#endif
      if (CHECK_FLAG (message, ZAPI_MESSAGE_METRIC))
	stream_putl (stream, route_info->cost);

      stream_putw_at (stream, 0, stream_get_endp (stream));
      zclient_send_message(zclient);
      SET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_SYNCED);
      UNSET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_RESYNC);
    }
}

static void
isis_kroute_route_del_ipv4 (struct prefix *prefix,
			   struct isis_route_info *route_info)
{
  struct zapi_ipv4 api;
  struct prefix_ipv4 prefix4;

  if (zclient->redist[KROUTE_ROUTE_ISIS])
    {
      api.type = KROUTE_ROUTE_ISIS;
      api.flags = 0;
      api.message = 0;
      api.safi = SAFI_UNICAST;
      prefix4.family = AF_INET;
      prefix4.prefixlen = prefix->prefixlen;
      prefix4.prefix = prefix->u.prefix4;
      zapi_ipv4_route (KROUTE_IPV4_ROUTE_DELETE, zclient, &prefix4, &api);
    }
  UNSET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_SYNCED);

  return;
}

#ifdef HAVE_IPV6
void
isis_kroute_route_add_ipv6 (struct prefix *prefix,
			   struct isis_route_info *route_info)
{
  struct zapi_ipv6 api;
  struct in6_addr **nexthop_list;
  unsigned int *ifindex_list;
  struct isis_nexthop6 *nexthop6;
  int i, size;
  struct listnode *node;
  struct prefix_ipv6 prefix6;

  if (CHECK_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_SYNCED))
    return;

  api.type = KROUTE_ROUTE_ISIS;
  api.flags = 0;
  api.message = 0;
  api.safi = SAFI_UNICAST;
  SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
  SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
  SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
  api.metric = route_info->cost;
#if 0
  SET_FLAG (api.message, ZAPI_MESSAGE_DISTANCE);
  api.distance = route_info->depth;
#endif
  api.nexthop_num = listcount (route_info->nexthops6);
  api.ifindex_num = listcount (route_info->nexthops6);

  /* allocate memory for nexthop_list */
  size = sizeof (struct isis_nexthop6 *) * listcount (route_info->nexthops6);
  nexthop_list = (struct in6_addr **) XMALLOC (MTYPE_ISIS_TMP, size);
  if (!nexthop_list)
    {
      zlog_err ("isis_kroute_add_route_ipv6: out of memory!");
      return;
    }

  /* allocate memory for ifindex_list */
  size = sizeof (unsigned int) * listcount (route_info->nexthops6);
  ifindex_list = (unsigned int *) XMALLOC (MTYPE_ISIS_TMP, size);
  if (!ifindex_list)
    {
      zlog_err ("isis_kroute_add_route_ipv6: out of memory!");
      XFREE (MTYPE_ISIS_TMP, nexthop_list);
      return;
    }

  /* for each nexthop */
  i = 0;
  for (ALL_LIST_ELEMENTS_RO (route_info->nexthops6, node, nexthop6))
    {
      if (!IN6_IS_ADDR_LINKLOCAL (&nexthop6->ip6) &&
	  !IN6_IS_ADDR_UNSPECIFIED (&nexthop6->ip6))
	{
	  api.nexthop_num--;
	  api.ifindex_num--;
	  continue;
	}

      nexthop_list[i] = &nexthop6->ip6;
      ifindex_list[i] = nexthop6->ifindex;
      i++;
    }

  api.nexthop = nexthop_list;
  api.ifindex = ifindex_list;

  if (api.nexthop_num && api.ifindex_num)
    {
      prefix6.family = AF_INET6;
      prefix6.prefixlen = prefix->prefixlen;
      memcpy (&prefix6.prefix, &prefix->u.prefix6, sizeof (struct in6_addr));
      zapi_ipv6_route (KROUTE_IPV6_ROUTE_ADD, zclient, &prefix6, &api);
      SET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_SYNCED);
      UNSET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_RESYNC);
    }

  XFREE (MTYPE_ISIS_TMP, nexthop_list);
  XFREE (MTYPE_ISIS_TMP, ifindex_list);

  return;
}

static void
isis_kroute_route_del_ipv6 (struct prefix *prefix,
			   struct isis_route_info *route_info)
{
  struct zapi_ipv6 api;
  struct in6_addr **nexthop_list;
  unsigned int *ifindex_list;
  struct isis_nexthop6 *nexthop6;
  int i, size;
  struct listnode *node;
  struct prefix_ipv6 prefix6;

  if (CHECK_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_SYNCED))
    return;

  api.type = KROUTE_ROUTE_ISIS;
  api.flags = 0;
  api.message = 0;
  api.safi = SAFI_UNICAST;
  SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
  SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
  api.nexthop_num = listcount (route_info->nexthops6);
  api.ifindex_num = listcount (route_info->nexthops6);

  /* allocate memory for nexthop_list */
  size = sizeof (struct isis_nexthop6 *) * listcount (route_info->nexthops6);
  nexthop_list = (struct in6_addr **) XMALLOC (MTYPE_ISIS_TMP, size);
  if (!nexthop_list)
    {
      zlog_err ("isis_kroute_route_del_ipv6: out of memory!");
      return;
    }

  /* allocate memory for ifindex_list */
  size = sizeof (unsigned int) * listcount (route_info->nexthops6);
  ifindex_list = (unsigned int *) XMALLOC (MTYPE_ISIS_TMP, size);
  if (!ifindex_list)
    {
      zlog_err ("isis_kroute_route_del_ipv6: out of memory!");
      XFREE (MTYPE_ISIS_TMP, nexthop_list);
      return;
    }

  /* for each nexthop */
  i = 0;
  for (ALL_LIST_ELEMENTS_RO (route_info->nexthops6, node, nexthop6))
    {
      if (!IN6_IS_ADDR_LINKLOCAL (&nexthop6->ip6) &&
	  !IN6_IS_ADDR_UNSPECIFIED (&nexthop6->ip6))
	{
	  api.nexthop_num--;
	  api.ifindex_num--;
	  continue;
	}

      nexthop_list[i] = &nexthop6->ip6;
      ifindex_list[i] = nexthop6->ifindex;
      i++;
    }

  api.nexthop = nexthop_list;
  api.ifindex = ifindex_list;

  if (api.nexthop_num && api.ifindex_num)
    {
      prefix6.family = AF_INET6;
      prefix6.prefixlen = prefix->prefixlen;
      memcpy (&prefix6.prefix, &prefix->u.prefix6, sizeof (struct in6_addr));
      zapi_ipv6_route (KROUTE_IPV6_ROUTE_DELETE, zclient, &prefix6, &api);
      UNSET_FLAG (route_info->flag, ISIS_ROUTE_FLAG_KROUTE_SYNCED);
    }

  XFREE (MTYPE_ISIS_TMP, nexthop_list);
  XFREE (MTYPE_ISIS_TMP, ifindex_list);
}

#endif /* HAVE_IPV6 */

void
isis_kroute_route_update (struct prefix *prefix,
			 struct isis_route_info *route_info)
{
  if (zclient->sock < 0)
    return;

  if (!zclient->redist[KROUTE_ROUTE_ISIS])
    return;

  if (CHECK_FLAG (route_info->flag, ISIS_ROUTE_FLAG_ACTIVE))
    {
      if (prefix->family == AF_INET)
	isis_kroute_route_add_ipv4 (prefix, route_info);
#ifdef HAVE_IPV6
      else if (prefix->family == AF_INET6)
	isis_kroute_route_add_ipv6 (prefix, route_info);
#endif /* HAVE_IPV6 */
    }
  else
    {
      if (prefix->family == AF_INET)
	isis_kroute_route_del_ipv4 (prefix, route_info);
#ifdef HAVE_IPV6
      else if (prefix->family == AF_INET6)
	isis_kroute_route_del_ipv6 (prefix, route_info);
#endif /* HAVE_IPV6 */
    }
  return;
}

static int
isis_kroute_read_ipv4 (int command, struct zclient *zclient,
		      kroute_size_t length)
{
  struct stream *stream;
  struct zapi_ipv4 api;
  struct prefix_ipv4 p;
  unsigned long ifindex;
  struct in_addr nexthop;

  stream = zclient->ibuf;
  memset (&p, 0, sizeof (struct prefix_ipv4));
  ifindex = 0;

  api.type = stream_getc (stream);
  api.flags = stream_getc (stream);
  api.message = stream_getc (stream);

  p.family = AF_INET;
  p.prefixlen = stream_getc (stream);
  stream_get (&p.prefix, stream, PSIZE (p.prefixlen));

  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP))
    {
      api.nexthop_num = stream_getc (stream);
      nexthop.s_addr = stream_get_ipv4 (stream);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_IFINDEX))
    {
      api.ifindex_num = stream_getc (stream);
      ifindex = stream_getl (stream);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_DISTANCE))
    api.distance = stream_getc (stream);
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_METRIC))
    api.metric = stream_getl (stream);
  else
    api.metric = 0;

  if (command == KROUTE_IPV4_ROUTE_ADD)
    {
      if (isis->debugs & DEBUG_KROUTE)
	zlog_debug ("IPv4 Route add from Z");
    }

  return 0;
}

#ifdef HAVE_IPV6
static int
isis_kroute_read_ipv6 (int command, struct zclient *zclient,
		      kroute_size_t length)
{
  return 0;
}
#endif

#define ISIS_TYPE_IS_REDISTRIBUTED(T) \
T == KROUTE_ROUTE_MAX ? zclient->default_information : zclient->redist[type]

int
isis_distribute_list_update (int routetype)
{
  return 0;
}

#if 0 /* Not yet. */
static int
isis_redistribute_default_set (int routetype, int metric_type,
			       int metric_value)
{
  return 0;
}
#endif /* 0 */

void
isis_kroute_init ()
{
  zclient = zclient_new ();
  zclient_init (zclient, KROUTE_ROUTE_ISIS);
  zclient->router_id_update = isis_router_id_update_kroute;
  zclient->interface_add = isis_kroute_if_add;
  zclient->interface_delete = isis_kroute_if_del;
  zclient->interface_up = isis_kroute_if_state_up;
  zclient->interface_down = isis_kroute_if_state_down;
  zclient->interface_address_add = isis_kroute_if_address_add;
  zclient->interface_address_delete = isis_kroute_if_address_del;
  zclient->ipv4_route_add = isis_kroute_read_ipv4;
  zclient->ipv4_route_delete = isis_kroute_read_ipv4;
#ifdef HAVE_IPV6
  zclient->ipv6_route_add = isis_kroute_read_ipv6;
  zclient->ipv6_route_delete = isis_kroute_read_ipv6;
#endif /* HAVE_IPV6 */

  return;
}
