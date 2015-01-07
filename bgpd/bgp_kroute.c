/* kroute client
   Copyright (C) 1997, 98, 99 Kunihiro Ishiguro

This file is part of GNU Kroute.

GNU Kroute is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

GNU Kroute is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Kroute; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <kroute.h>

#include "command.h"
#include "stream.h"
#include "network.h"
#include "prefix.h"
#include "log.h"
#include "sockunion.h"
#include "zclient.h"
#include "routemap.h"
#include "thread.h"

#include "bgpd/bgpd.h"
#include "bgpd/bgp_route.h"
#include "bgpd/bgp_attr.h"
#include "bgpd/bgp_nexthop.h"
#include "bgpd/bgp_kroute.h"
#include "bgpd/bgp_fsm.h"
#include "bgpd/bgp_debug.h"
#include "bgpd/bgp_mpath.h"

/* All information about kroute. */
struct zclient *zclient = NULL;
struct in_addr router_id_kroute;

/* Growable buffer for nexthops sent to kroute */
struct stream *bgp_nexthop_buf = NULL;

/* Router-id update message from kroute. */
static int
bgp_router_id_update (int command, struct zclient *zclient, kroute_size_t length)
{
  struct prefix router_id;
  struct listnode *node, *nnode;
  struct bgp *bgp;

  kroute_router_id_update_read(zclient->ibuf,&router_id);

  if (BGP_DEBUG(kroute, KROUTE))
    {
      char buf[128];
      prefix2str(&router_id, buf, sizeof(buf));
      zlog_debug("Kroute rcvd: router id update %s", buf);
    }

  router_id_kroute = router_id.u.prefix4;

  for (ALL_LIST_ELEMENTS (bm->bgp, node, nnode, bgp))
    {
      if (!bgp->router_id_static.s_addr)
        bgp_router_id_set (bgp, &router_id.u.prefix4);
    }

  return 0;
}

/* Inteface addition message from kroute. */
static int
bgp_interface_add (int command, struct zclient *zclient, kroute_size_t length)
{
  struct interface *ifp;

  ifp = kroute_interface_add_read (zclient->ibuf);

  if (BGP_DEBUG(kroute, KROUTE) && ifp)
    zlog_debug("Kroute rcvd: interface add %s", ifp->name);

  return 0;
}

static int
bgp_interface_delete (int command, struct zclient *zclient,
		      kroute_size_t length)
{
  struct stream *s;
  struct interface *ifp;

  s = zclient->ibuf;
  ifp = kroute_interface_state_read (s);
  ifp->ifindex = IFINDEX_INTERNAL;

  if (BGP_DEBUG(kroute, KROUTE))
    zlog_debug("Kroute rcvd: interface delete %s", ifp->name);

  return 0;
}

static int
bgp_interface_up (int command, struct zclient *zclient, kroute_size_t length)
{
  struct stream *s;
  struct interface *ifp;
  struct connected *c;
  struct listnode *node, *nnode;

  s = zclient->ibuf;
  ifp = kroute_interface_state_read (s);

  if (! ifp)
    return 0;

  if (BGP_DEBUG(kroute, KROUTE))
    zlog_debug("Kroute rcvd: interface %s up", ifp->name);

  for (ALL_LIST_ELEMENTS (ifp->connected, node, nnode, c))
    bgp_connected_add (c);

  return 0;
}

static int
bgp_interface_down (int command, struct zclient *zclient, kroute_size_t length)
{
  struct stream *s;
  struct interface *ifp;
  struct connected *c;
  struct listnode *node, *nnode;

  s = zclient->ibuf;
  ifp = kroute_interface_state_read (s);
  if (! ifp)
    return 0;

  if (BGP_DEBUG(kroute, KROUTE))
    zlog_debug("Kroute rcvd: interface %s down", ifp->name);

  for (ALL_LIST_ELEMENTS (ifp->connected, node, nnode, c))
    bgp_connected_delete (c);

  /* Fast external-failover (Currently IPv4 only) */
  {
    struct listnode *mnode;
    struct bgp *bgp;
    struct peer *peer;
    struct interface *peer_if;

    for (ALL_LIST_ELEMENTS_RO (bm->bgp, mnode, bgp))
      {
	if (CHECK_FLAG (bgp->flags, BGP_FLAG_NO_FAST_EXT_FAILOVER))
	  continue;

	for (ALL_LIST_ELEMENTS (bgp->peer, node, nnode, peer))
	  {
	    if (peer->ttl != 1)
	      continue;

	    if (peer->su.sa.sa_family == AF_INET)
	      peer_if = if_lookup_by_ipv4 (&peer->su.sin.sin_addr);
	    else
	      continue;

	    if (ifp == peer_if)
	      BGP_EVENT_ADD (peer, BGP_Stop);
	  }
      }
  }

  return 0;
}

static int
bgp_interface_address_add (int command, struct zclient *zclient,
			   kroute_size_t length)
{
  struct connected *ifc;

  ifc = kroute_interface_address_read (command, zclient->ibuf);

  if (ifc == NULL)
    return 0;

  if (BGP_DEBUG(kroute, KROUTE))
    {
      char buf[128];
      prefix2str(ifc->address, buf, sizeof(buf));
      zlog_debug("Kroute rcvd: interface %s address add %s",
		 ifc->ifp->name, buf);
    }

  if (if_is_operative (ifc->ifp))
    bgp_connected_add (ifc);

  return 0;
}

static int
bgp_interface_address_delete (int command, struct zclient *zclient,
			      kroute_size_t length)
{
  struct connected *ifc;

  ifc = kroute_interface_address_read (command, zclient->ibuf);

  if (ifc == NULL)
    return 0;

  if (BGP_DEBUG(kroute, KROUTE))
    {
      char buf[128];
      prefix2str(ifc->address, buf, sizeof(buf));
      zlog_debug("Kroute rcvd: interface %s address delete %s",
		 ifc->ifp->name, buf);
    }

  if (if_is_operative (ifc->ifp))
    bgp_connected_delete (ifc);

  connected_free (ifc);

  return 0;
}

/* Kroute route add and delete treatment. */
static int
kroute_read_ipv4 (int command, struct zclient *zclient, kroute_size_t length)
{
  struct stream *s;
  struct zapi_ipv4 api;
  struct in_addr nexthop;
  struct prefix_ipv4 p;

  s = zclient->ibuf;
  nexthop.s_addr = 0;

  /* Type, flags, message. */
  api.type = stream_getc (s);
  api.flags = stream_getc (s);
  api.message = stream_getc (s);

  /* IPv4 prefix. */
  memset (&p, 0, sizeof (struct prefix_ipv4));
  p.family = AF_INET;
  p.prefixlen = stream_getc (s);
  stream_get (&p.prefix, s, PSIZE (p.prefixlen));

  /* Nexthop, ifindex, distance, metric. */
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP))
    {
      api.nexthop_num = stream_getc (s);
      nexthop.s_addr = stream_get_ipv4 (s);
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_IFINDEX))
    {
      api.ifindex_num = stream_getc (s);
      stream_getl (s); /* ifindex, unused */
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_DISTANCE))
    api.distance = stream_getc (s);
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_METRIC))
    api.metric = stream_getl (s);
  else
    api.metric = 0;

  if (command == KROUTE_IPV4_ROUTE_ADD)
    {
      if (BGP_DEBUG(kroute, KROUTE))
	{
	  char buf[2][INET_ADDRSTRLEN];
	  zlog_debug("Kroute rcvd: IPv4 route add %s %s/%d nexthop %s metric %u",
		     kroute_route_string(api.type),
		     inet_ntop(AF_INET, &p.prefix, buf[0], sizeof(buf[0])),
		     p.prefixlen,
		     inet_ntop(AF_INET, &nexthop, buf[1], sizeof(buf[1])),
		     api.metric);
	}
      bgp_redistribute_add((struct prefix *)&p, &nexthop, NULL,
			   api.metric, api.type);
    }
  else
    {
      if (BGP_DEBUG(kroute, KROUTE))
	{
	  char buf[2][INET_ADDRSTRLEN];
	  zlog_debug("Kroute rcvd: IPv4 route delete %s %s/%d "
		     "nexthop %s metric %u",
		     kroute_route_string(api.type),
		     inet_ntop(AF_INET, &p.prefix, buf[0], sizeof(buf[0])),
		     p.prefixlen,
		     inet_ntop(AF_INET, &nexthop, buf[1], sizeof(buf[1])),
		     api.metric);
	}
      bgp_redistribute_delete((struct prefix *)&p, api.type);
    }

  return 0;
}

#ifdef HAVE_IPV6
/* Kroute route add and delete treatment. */
static int
kroute_read_ipv6 (int command, struct zclient *zclient, kroute_size_t length)
{
  struct stream *s;
  struct zapi_ipv6 api;
  struct in6_addr nexthop;
  struct prefix_ipv6 p;

  s = zclient->ibuf;
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
      stream_getl (s); /* ifindex, unused */
    }
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_DISTANCE))
    api.distance = stream_getc (s);
  else
    api.distance = 0;
  if (CHECK_FLAG (api.message, ZAPI_MESSAGE_METRIC))
    api.metric = stream_getl (s);
  else
    api.metric = 0;

  /* Simply ignore link-local address. */
  if (IN6_IS_ADDR_LINKLOCAL (&p.prefix))
    return 0;

  if (command == KROUTE_IPV6_ROUTE_ADD)
    {
      if (BGP_DEBUG(kroute, KROUTE))
	{
	  char buf[2][INET6_ADDRSTRLEN];
	  zlog_debug("Kroute rcvd: IPv6 route add %s %s/%d nexthop %s metric %u",
		     kroute_route_string(api.type),
		     inet_ntop(AF_INET6, &p.prefix, buf[0], sizeof(buf[0])),
		     p.prefixlen,
		     inet_ntop(AF_INET, &nexthop, buf[1], sizeof(buf[1])),
		     api.metric);
	}
      bgp_redistribute_add ((struct prefix *)&p, NULL, &nexthop,
			    api.metric, api.type);
    }
  else
    {
      if (BGP_DEBUG(kroute, KROUTE))
	{
	  char buf[2][INET6_ADDRSTRLEN];
	  zlog_debug("Kroute rcvd: IPv6 route delete %s %s/%d "
		     "nexthop %s metric %u",
		     kroute_route_string(api.type),
		     inet_ntop(AF_INET6, &p.prefix, buf[0], sizeof(buf[0])),
		     p.prefixlen,
		     inet_ntop(AF_INET6, &nexthop, buf[1], sizeof(buf[1])),
		     api.metric);
	}
      bgp_redistribute_delete ((struct prefix *) &p, api.type);
    }
  
  return 0;
}
#endif /* HAVE_IPV6 */

struct interface *
if_lookup_by_ipv4 (struct in_addr *addr)
{
  struct listnode *ifnode;
  struct listnode *cnode;
  struct interface *ifp;
  struct connected *connected;
  struct prefix_ipv4 p;
  struct prefix *cp; 
  
  p.family = AF_INET;
  p.prefix = *addr;
  p.prefixlen = IPV4_MAX_BITLEN;

  for (ALL_LIST_ELEMENTS_RO (iflist, ifnode, ifp))
    {
      for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, connected))
	{
	  cp = connected->address;
	    
	  if (cp->family == AF_INET)
	    if (prefix_match (cp, (struct prefix *)&p))
	      return ifp;
	}
    }
  return NULL;
}

struct interface *
if_lookup_by_ipv4_exact (struct in_addr *addr)
{
  struct listnode *ifnode;
  struct listnode *cnode;
  struct interface *ifp;
  struct connected *connected;
  struct prefix *cp; 
  
  for (ALL_LIST_ELEMENTS_RO (iflist, ifnode, ifp))
    {
      for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, connected))
	{
	  cp = connected->address;
	    
	  if (cp->family == AF_INET)
	    if (IPV4_ADDR_SAME (&cp->u.prefix4, addr))
	      return ifp;
	}
    }
  return NULL;
}

#ifdef HAVE_IPV6
struct interface *
if_lookup_by_ipv6 (struct in6_addr *addr)
{
  struct listnode *ifnode;
  struct listnode *cnode;
  struct interface *ifp;
  struct connected *connected;
  struct prefix_ipv6 p;
  struct prefix *cp; 
  
  p.family = AF_INET6;
  p.prefix = *addr;
  p.prefixlen = IPV6_MAX_BITLEN;

  for (ALL_LIST_ELEMENTS_RO (iflist, ifnode, ifp))
    {
      for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, connected))
	{
	  cp = connected->address;
	    
	  if (cp->family == AF_INET6)
	    if (prefix_match (cp, (struct prefix *)&p))
	      return ifp;
	}
    }
  return NULL;
}

struct interface *
if_lookup_by_ipv6_exact (struct in6_addr *addr)
{
  struct listnode *ifnode;
  struct listnode *cnode;
  struct interface *ifp;
  struct connected *connected;
  struct prefix *cp; 

  for (ALL_LIST_ELEMENTS_RO (iflist, ifnode, ifp))
    {
      for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, connected))
	{
	  cp = connected->address;
	    
	  if (cp->family == AF_INET6)
	    if (IPV6_ADDR_SAME (&cp->u.prefix6, addr))
	      return ifp;
	}
    }
  return NULL;
}

static int
if_get_ipv6_global (struct interface *ifp, struct in6_addr *addr)
{
  struct listnode *cnode;
  struct connected *connected;
  struct prefix *cp; 
  
  for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, connected))
    {
      cp = connected->address;
	    
      if (cp->family == AF_INET6)
	if (! IN6_IS_ADDR_LINKLOCAL (&cp->u.prefix6))
	  {
	    memcpy (addr, &cp->u.prefix6, IPV6_MAX_BYTELEN);
	    return 1;
	  }
    }
  return 0;
}

static int
if_get_ipv6_local (struct interface *ifp, struct in6_addr *addr)
{
  struct listnode *cnode;
  struct connected *connected;
  struct prefix *cp; 
  
  for (ALL_LIST_ELEMENTS_RO (ifp->connected, cnode, connected))
    {
      cp = connected->address;
	    
      if (cp->family == AF_INET6)
	if (IN6_IS_ADDR_LINKLOCAL (&cp->u.prefix6))
	  {
	    memcpy (addr, &cp->u.prefix6, IPV6_MAX_BYTELEN);
	    return 1;
	  }
    }
  return 0;
}
#endif /* HAVE_IPV6 */

int
bgp_nexthop_set (union sockunion *local, union sockunion *remote, 
		 struct bgp_nexthop *nexthop, struct peer *peer)
{
  int ret = 0;
  struct interface *ifp = NULL;

  memset (nexthop, 0, sizeof (struct bgp_nexthop));

  if (!local)
    return -1;
  if (!remote)
    return -1;

  if (local->sa.sa_family == AF_INET)
    {
      nexthop->v4 = local->sin.sin_addr;
      ifp = if_lookup_by_ipv4 (&local->sin.sin_addr);
    }
#ifdef HAVE_IPV6
  if (local->sa.sa_family == AF_INET6)
    {
      if (IN6_IS_ADDR_LINKLOCAL (&local->sin6.sin6_addr))
	{
	  if (peer->ifname)
	    ifp = if_lookup_by_index (if_nametoindex (peer->ifname));
	}
      else
	ifp = if_lookup_by_ipv6 (&local->sin6.sin6_addr);
    }
#endif /* HAVE_IPV6 */

  if (!ifp)
    return -1;

  nexthop->ifp = ifp;

  /* IPv4 connection. */
  if (local->sa.sa_family == AF_INET)
    {
#ifdef HAVE_IPV6
      /* IPv6 nexthop*/
      ret = if_get_ipv6_global (ifp, &nexthop->v6_global);

      /* There is no global nexthop. */
      if (!ret)
	if_get_ipv6_local (ifp, &nexthop->v6_global);
      else
	if_get_ipv6_local (ifp, &nexthop->v6_local);
#endif /* HAVE_IPV6 */
    }

#ifdef HAVE_IPV6
  /* IPv6 connection. */
  if (local->sa.sa_family == AF_INET6)
    {
      struct interface *direct = NULL;

      /* IPv4 nexthop.  I don't care about it. */
      if (peer->local_id.s_addr)
	nexthop->v4 = peer->local_id;

      /* Global address*/
      if (! IN6_IS_ADDR_LINKLOCAL (&local->sin6.sin6_addr))
	{
	  memcpy (&nexthop->v6_global, &local->sin6.sin6_addr, 
		  IPV6_MAX_BYTELEN);

	  /* If directory connected set link-local address. */
	  direct = if_lookup_by_ipv6 (&remote->sin6.sin6_addr);
	  if (direct)
	    if_get_ipv6_local (ifp, &nexthop->v6_local);
	}
      else
	/* Link-local address. */
	{
	  ret = if_get_ipv6_global (ifp, &nexthop->v6_global);

	  /* If there is no global address.  Set link-local address as
             global.  I know this break RFC specification... */
	  if (!ret)
	    memcpy (&nexthop->v6_global, &local->sin6.sin6_addr, 
		    IPV6_MAX_BYTELEN);
	  else
	    memcpy (&nexthop->v6_local, &local->sin6.sin6_addr, 
		    IPV6_MAX_BYTELEN);
	}
    }

  if (IN6_IS_ADDR_LINKLOCAL (&local->sin6.sin6_addr) ||
      if_lookup_by_ipv6 (&remote->sin6.sin6_addr))
    peer->shared_network = 1;
  else
    peer->shared_network = 0;

  /* KAME stack specific treatment.  */
#ifdef KAME
  if (IN6_IS_ADDR_LINKLOCAL (&nexthop->v6_global)
      && IN6_LINKLOCAL_IFINDEX (nexthop->v6_global))
    {
      SET_IN6_LINKLOCAL_IFINDEX (nexthop->v6_global, 0);
    }
  if (IN6_IS_ADDR_LINKLOCAL (&nexthop->v6_local)
      && IN6_LINKLOCAL_IFINDEX (nexthop->v6_local))
    {
      SET_IN6_LINKLOCAL_IFINDEX (nexthop->v6_local, 0);
    }
#endif /* KAME */
#endif /* HAVE_IPV6 */
  return ret;
}

void
bgp_kroute_announce (struct prefix *p, struct bgp_info *info, struct bgp *bgp, safi_t safi)
{
  int flags;
  u_char distance;
  struct peer *peer;
  struct bgp_info *mpinfo;
  size_t oldsize, newsize;

  if (zclient->sock < 0)
    return;

  if (! zclient->redist[KROUTE_ROUTE_BGP])
    return;

  flags = 0;
  peer = info->peer;

  if (peer_sort (peer) == BGP_PEER_IBGP || peer_sort (peer) == BGP_PEER_CONFED)
    {
      SET_FLAG (flags, KROUTE_FLAG_IBGP);
      SET_FLAG (flags, KROUTE_FLAG_INTERNAL);
    }

  if ((peer_sort (peer) == BGP_PEER_EBGP && peer->ttl != 1)
      || CHECK_FLAG (peer->flags, PEER_FLAG_DISABLE_CONNECTED_CHECK))
    SET_FLAG (flags, KROUTE_FLAG_INTERNAL);

  /* resize nexthop buffer size if necessary */
  if ((oldsize = stream_get_size (bgp_nexthop_buf)) <
      (sizeof (struct in_addr *) * (bgp_info_mpath_count (info) + 1)))
    {
      newsize = (sizeof (struct in_addr *) * (bgp_info_mpath_count (info) + 1));
      newsize = stream_resize (bgp_nexthop_buf, newsize);
      if (newsize == oldsize)
	{
	  zlog_err ("can't resize nexthop buffer");
	  return;
	}
    }

  stream_reset (bgp_nexthop_buf);

  if (p->family == AF_INET)
    {
      struct zapi_ipv4 api;
      struct in_addr *nexthop;

      api.flags = flags;
      nexthop = &info->attr->nexthop;
      stream_put (bgp_nexthop_buf, &nexthop, sizeof (struct in_addr *));
      for (mpinfo = bgp_info_mpath_first (info); mpinfo;
	   mpinfo = bgp_info_mpath_next (mpinfo))
	{
	  nexthop = &mpinfo->attr->nexthop;
	  stream_put (bgp_nexthop_buf, &nexthop, sizeof (struct in_addr *));
	}

      api.type = KROUTE_ROUTE_BGP;
      api.message = 0;
      api.safi = safi;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1 + bgp_info_mpath_count (info);
      api.nexthop = (struct in_addr **)STREAM_DATA (bgp_nexthop_buf);
      api.ifindex_num = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = info->attr->med;

      distance = bgp_distance_apply (p, info, bgp);

      if (distance)
	{
	  SET_FLAG (api.message, ZAPI_MESSAGE_DISTANCE);
	  api.distance = distance;
	}

      if (BGP_DEBUG(kroute, KROUTE))
	{
	  int i;
	  char buf[2][INET_ADDRSTRLEN];
	  zlog_debug("Kroute send: IPv4 route add %s/%d nexthop %s metric %u"
		     " count %d",
		     inet_ntop(AF_INET, &p->u.prefix4, buf[0], sizeof(buf[0])),
		     p->prefixlen,
		     inet_ntop(AF_INET, api.nexthop[0], buf[1], sizeof(buf[1])),
		     api.metric, api.nexthop_num);
	  for (i = 1; i < api.nexthop_num; i++)
	    zlog_debug("Kroute send: IPv4 route add [nexthop %d] %s",
		       i, inet_ntop(AF_INET, api.nexthop[i], buf[1],
				    sizeof(buf[1])));
	}

      zapi_ipv4_route (KROUTE_IPV4_ROUTE_ADD, zclient, 
                       (struct prefix_ipv4 *) p, &api);
    }
#ifdef HAVE_IPV6
  /* We have to think about a IPv6 link-local address curse. */
  if (p->family == AF_INET6)
    {
      unsigned int ifindex;
      struct in6_addr *nexthop;
      struct zapi_ipv6 api;

      ifindex = 0;
      nexthop = NULL;
      
      assert (info->attr->extra);
      
      /* Only global address nexthop exists. */
      if (info->attr->extra->mp_nexthop_len == 16)
	nexthop = &info->attr->extra->mp_nexthop_global;
      
      /* If both global and link-local address present. */
      if (info->attr->extra->mp_nexthop_len == 32)
	{
	  /* Workaround for Cisco's nexthop bug.  */
	  if (IN6_IS_ADDR_UNSPECIFIED (&info->attr->extra->mp_nexthop_global)
	      && peer->su_remote->sa.sa_family == AF_INET6)
	    nexthop = &peer->su_remote->sin6.sin6_addr;
	  else
	    nexthop = &info->attr->extra->mp_nexthop_local;

	  if (info->peer->nexthop.ifp)
	    ifindex = info->peer->nexthop.ifp->ifindex;
	}

      if (nexthop == NULL)
	return;

      if (IN6_IS_ADDR_LINKLOCAL (nexthop) && ! ifindex)
	{
	  if (info->peer->ifname)
	    ifindex = if_nametoindex (info->peer->ifname);
	  else if (info->peer->nexthop.ifp)
	    ifindex = info->peer->nexthop.ifp->ifindex;
	}

      /* Make Kroute API structure. */
      api.flags = flags;
      api.type = KROUTE_ROUTE_BGP;
      api.message = 0;
      api.safi = safi;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
      api.ifindex_num = 1;
      api.ifindex = &ifindex;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = info->attr->med;

      if (BGP_DEBUG(kroute, KROUTE))
	{
	  char buf[2][INET6_ADDRSTRLEN];
	  zlog_debug("Kroute send: IPv6 route add %s/%d nexthop %s metric %u",
		     inet_ntop(AF_INET6, &p->u.prefix6, buf[0], sizeof(buf[0])),
		     p->prefixlen,
		     inet_ntop(AF_INET6, nexthop, buf[1], sizeof(buf[1])),
		     api.metric);
	}

      zapi_ipv6_route (KROUTE_IPV6_ROUTE_ADD, zclient, 
                       (struct prefix_ipv6 *) p, &api);
    }
#endif /* HAVE_IPV6 */
}

void
bgp_kroute_withdraw (struct prefix *p, struct bgp_info *info, safi_t safi)
{
  int flags;
  struct peer *peer;

  if (zclient->sock < 0)
    return;

  if (! zclient->redist[KROUTE_ROUTE_BGP])
    return;

  peer = info->peer;
  flags = 0;

  if (peer_sort (peer) == BGP_PEER_IBGP)
    {
      SET_FLAG (flags, KROUTE_FLAG_INTERNAL);
      SET_FLAG (flags, KROUTE_FLAG_IBGP);
    }

  if ((peer_sort (peer) == BGP_PEER_EBGP && peer->ttl != 1)
      || CHECK_FLAG (peer->flags, PEER_FLAG_DISABLE_CONNECTED_CHECK))
    SET_FLAG (flags, KROUTE_FLAG_INTERNAL);

  if (p->family == AF_INET)
    {
      struct zapi_ipv4 api;
      struct in_addr *nexthop;

      api.flags = flags;
      nexthop = &info->attr->nexthop;

      api.type = KROUTE_ROUTE_BGP;
      api.message = 0;
      api.safi = safi;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      api.ifindex_num = 0;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = info->attr->med;

      if (BGP_DEBUG(kroute, KROUTE))
	{
	  char buf[2][INET_ADDRSTRLEN];
	  zlog_debug("Kroute send: IPv4 route delete %s/%d nexthop %s metric %u",
		     inet_ntop(AF_INET, &p->u.prefix4, buf[0], sizeof(buf[0])),
		     p->prefixlen,
		     inet_ntop(AF_INET, nexthop, buf[1], sizeof(buf[1])),
		     api.metric);
	}

      zapi_ipv4_route (KROUTE_IPV4_ROUTE_DELETE, zclient, 
                       (struct prefix_ipv4 *) p, &api);
    }
#ifdef HAVE_IPV6
  /* We have to think about a IPv6 link-local address curse. */
  if (p->family == AF_INET6)
    {
      struct zapi_ipv6 api;
      unsigned int ifindex;
      struct in6_addr *nexthop;
      
      assert (info->attr->extra);
      
      ifindex = 0;
      nexthop = NULL;

      /* Only global address nexthop exists. */
      if (info->attr->extra->mp_nexthop_len == 16)
	nexthop = &info->attr->extra->mp_nexthop_global;

      /* If both global and link-local address present. */
      if (info->attr->extra->mp_nexthop_len == 32)
	{
	  nexthop = &info->attr->extra->mp_nexthop_local;
	  if (info->peer->nexthop.ifp)
	    ifindex = info->peer->nexthop.ifp->ifindex;
	}

      if (nexthop == NULL)
	return;

      if (IN6_IS_ADDR_LINKLOCAL (nexthop) && ! ifindex)
	if (info->peer->ifname)
	  ifindex = if_nametoindex (info->peer->ifname);

      api.flags = flags;
      api.type = KROUTE_ROUTE_BGP;
      api.message = 0;
      api.safi = safi;
      SET_FLAG (api.message, ZAPI_MESSAGE_NEXTHOP);
      api.nexthop_num = 1;
      api.nexthop = &nexthop;
      SET_FLAG (api.message, ZAPI_MESSAGE_IFINDEX);
      api.ifindex_num = 1;
      api.ifindex = &ifindex;
      SET_FLAG (api.message, ZAPI_MESSAGE_METRIC);
      api.metric = info->attr->med;

      if (BGP_DEBUG(kroute, KROUTE))
	{
	  char buf[2][INET6_ADDRSTRLEN];
	  zlog_debug("Kroute send: IPv6 route delete %s/%d nexthop %s metric %u",
		     inet_ntop(AF_INET6, &p->u.prefix6, buf[0], sizeof(buf[0])),
		     p->prefixlen,
		     inet_ntop(AF_INET6, nexthop, buf[1], sizeof(buf[1])),
		     api.metric);
	}

      zapi_ipv6_route (KROUTE_IPV6_ROUTE_DELETE, zclient, 
                       (struct prefix_ipv6 *) p, &api);
    }
#endif /* HAVE_IPV6 */
}

/* Other routes redistribution into BGP. */
int
bgp_redistribute_set (struct bgp *bgp, afi_t afi, int type)
{
  /* Set flag to BGP instance. */
  bgp->redist[afi][type] = 1;

  /* Return if already redistribute flag is set. */
  if (zclient->redist[type])
    return CMD_WARNING;

  zclient->redist[type] = 1;

  /* Return if kroute connection is not established. */
  if (zclient->sock < 0)
    return CMD_WARNING;

  if (BGP_DEBUG(kroute, KROUTE))
    zlog_debug("Kroute send: redistribute add %s", kroute_route_string(type));
    
  /* Send distribute add message to kroute. */
  kroute_redistribute_send (KROUTE_REDISTRIBUTE_ADD, zclient, type);

  return CMD_SUCCESS;
}

/* Redistribute with route-map specification.  */
int
bgp_redistribute_rmap_set (struct bgp *bgp, afi_t afi, int type, 
                           const char *name)
{
  if (bgp->rmap[afi][type].name
      && (strcmp (bgp->rmap[afi][type].name, name) == 0))
    return 0;

  if (bgp->rmap[afi][type].name)
    free (bgp->rmap[afi][type].name);
  bgp->rmap[afi][type].name = strdup (name);
  bgp->rmap[afi][type].map = route_map_lookup_by_name (name);

  return 1;
}

/* Redistribute with metric specification.  */
int
bgp_redistribute_metric_set (struct bgp *bgp, afi_t afi, int type,
			     u_int32_t metric)
{
  if (bgp->redist_metric_flag[afi][type]
      && bgp->redist_metric[afi][type] == metric)
    return 0;

  bgp->redist_metric_flag[afi][type] = 1;
  bgp->redist_metric[afi][type] = metric;

  return 1;
}

/* Unset redistribution.  */
int
bgp_redistribute_unset (struct bgp *bgp, afi_t afi, int type)
{
  /* Unset flag from BGP instance. */
  bgp->redist[afi][type] = 0;

  /* Unset route-map. */
  if (bgp->rmap[afi][type].name)
    free (bgp->rmap[afi][type].name);
  bgp->rmap[afi][type].name = NULL;
  bgp->rmap[afi][type].map = NULL;

  /* Unset metric. */
  bgp->redist_metric_flag[afi][type] = 0;
  bgp->redist_metric[afi][type] = 0;

  /* Return if kroute connection is disabled. */
  if (! zclient->redist[type])
    return CMD_WARNING;
  zclient->redist[type] = 0;

  if (bgp->redist[AFI_IP][type] == 0 
      && bgp->redist[AFI_IP6][type] == 0 
      && zclient->sock >= 0)
    {
      /* Send distribute delete message to kroute. */
      if (BGP_DEBUG(kroute, KROUTE))
	zlog_debug("Kroute send: redistribute delete %s",
		   kroute_route_string(type));
      kroute_redistribute_send (KROUTE_REDISTRIBUTE_DELETE, zclient, type);
    }
  
  /* Withdraw redistributed routes from current BGP's routing table. */
  bgp_redistribute_withdraw (bgp, afi, type);

  return CMD_SUCCESS;
}

/* Unset redistribution route-map configuration.  */
int
bgp_redistribute_routemap_unset (struct bgp *bgp, afi_t afi, int type)
{
  if (! bgp->rmap[afi][type].name)
    return 0;

  /* Unset route-map. */
  free (bgp->rmap[afi][type].name);
  bgp->rmap[afi][type].name = NULL;
  bgp->rmap[afi][type].map = NULL;

  return 1;
}

/* Unset redistribution metric configuration.  */
int
bgp_redistribute_metric_unset (struct bgp *bgp, afi_t afi, int type)
{
  if (! bgp->redist_metric_flag[afi][type])
    return 0;

  /* Unset metric. */
  bgp->redist_metric_flag[afi][type] = 0;
  bgp->redist_metric[afi][type] = 0;

  return 1;
}

void
bgp_zclient_reset (void)
{
  zclient_reset (zclient);
}

void
bgp_kroute_init (void)
{
  /* Set default values. */
  zclient = zclient_new ();
  zclient_init (zclient, KROUTE_ROUTE_BGP);
  zclient->router_id_update = bgp_router_id_update;
  zclient->interface_add = bgp_interface_add;
  zclient->interface_delete = bgp_interface_delete;
  zclient->interface_address_add = bgp_interface_address_add;
  zclient->interface_address_delete = bgp_interface_address_delete;
  zclient->ipv4_route_add = kroute_read_ipv4;
  zclient->ipv4_route_delete = kroute_read_ipv4;
  zclient->interface_up = bgp_interface_up;
  zclient->interface_down = bgp_interface_down;
#ifdef HAVE_IPV6
  zclient->ipv6_route_add = kroute_read_ipv6;
  zclient->ipv6_route_delete = kroute_read_ipv6;
#endif /* HAVE_IPV6 */

  /* Interface related init. */
  if_init ();

  bgp_nexthop_buf = stream_new(BGP_NEXTHOP_BUF_SIZE);
}
