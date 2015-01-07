/* Copyright (C) 2002-2003 IP Infusion, Inc. All Rights Reserved. */

#include "if.h"
#include "thread.h"
#include "table.h"
#include "log.h"
#include "prefix.h"
#include "stream.h"
#include "nsm_client.h"
#include "network.h"
#include "nexthop.h"

#include "pimd/pimd.h"
#include "pimd/pim_nexthop.h"
#include "pimd/pim_vif.h"
#include "pimd/pim_neighbor.h"
#include "pimd/pim_mrt.h"
#include "pimd/pim_macrochange.h"

#define NSM_IPV4_NEXTHOP_QUERY_SIZE         7

extern struct nsm_client *pim_nc;

/* Allocate new PIM nexthop structure.  */
struct pim_nexthop *
pim_nexthop_new ()
{
  return (struct pim_nexthop *) XCALLOC (MTYPE_PIM_NEXTHOP,
					 sizeof (struct pim_nexthop));
}

/* Add nexthop to the end of the list.  */
void
pim_nexthop_add (struct pim_nexthop *pn, struct nexthop *nexthop)
{
  struct nexthop *last;

  for (last = pn->nexthop; last && last->next; last = last->next)
    ;

  if (last)
    last->next = nexthop;
  else
    pn->nexthop = nexthop;

  nexthop->prev = last;
}

/* Free PIM nexthop structure.  */
void
pim_nexthop_free (struct pim_nexthop *pn)
{
  struct nexthop *n;
  struct nexthop *next;

  /* Tag 1. Please donot delete. */
  for (n = pn->nexthop; n; n = next)
    {
      next = n->next;
      XFREE (MTYPE_NEXTHOP, n);
    }

  XFREE (MTYPE_PIM_NEXTHOP, pn);
}

/* Query nexthop to NSM.  */
struct pim_nexthop *
pim_nexthop_query (struct pim_top *pim_top,  struct pal_in4_addr *addr)
{
  int i;
  struct nsm_msg_route_lookup_ipv4 msg;
  struct nsm_msg_route_ipv4 route;
  int ret;
  struct nexthop *nexthop;
  struct pim_nexthop *pn;

  
  pal_mem_set (&msg, 0, sizeof(struct nsm_msg_route_lookup_ipv4));
  pal_mem_set (&route, 0, sizeof(struct nsm_msg_route_ipv4));
  
  msg.addr = *addr;
  
  ret = nsm_client_route_lookup_ipv4 (0, 0, pim_nc, &msg,
				      BEST_MATCH_LOOKUP, &route);

  if (ret < 0)
    return NULL;

  if (route.nexthop_num)
    {
      pn = pim_nexthop_new ();

      if (pn == NULL)
        return NULL;

      pn->refcnt = 0;

      pn->metric = route.metric;
      pn->preference = route.distance;
      pn->nexthop_num = route.nexthop_num;

      SET_FLAG (pn->flag, PIM_NEXTHOP_NEW);
      UNSET_FLAG (pn->flag, PIM_NEXTHOP_UNR);

      /* Copy the resolved address to the pim_nexthop structure
       */
      pn->addr = *addr;
      
      for (i = 0; i < route.nexthop_num; i++)
	{
	  nexthop = XCALLOC (MTYPE_NEXTHOP, sizeof (struct nexthop));

          if (nexthop == NULL)
            {
              pim_nexthop_free (pn);
              return NULL;
            }

	  nexthop->gate.ipv4.s_addr = route.nexthop[i].addr.s_addr;
	  nexthop->ifindex = route.nexthop[i].ifindex;
	  
	  pim_nexthop_add (pn, nexthop);
	}
      return pn;
    }
  return NULL;
}

/* MRIB.next_hop(S) macro.  */
struct pim_neighbor *
pim_mrib_next_hop_s (struct pal_in4_addr *addr)
{
  struct pim_nexthop *pn;
  struct nexthop *nexthop;
  struct interface *ifp;
  struct pim_vif *vif;
  struct ipi_vr *vr = ipi_vr_get_privileged (pim_top->zg);

  /* Lookup nexthop */
  pn = pim_nexthop_lookup (addr);
  if (! pn)
    {
      if (IS_DEBUG_PIM_NEXTHOP(pim_top))
     	PIMLOG_INFO (pim_top->zg, "MRIB.next_hop_s(%r): Can't find nexthop",  addr);
      return NULL;
    }
  else
    if (IS_DEBUG_PIM_NEXTHOP(pim_top))
      PIMLOG_INFO (pim_top->zg, "MRIB.next_hop_s(%r): nexthop %r",  addr, &pn->nexthop->gate.ipv4);

  /* Set the flag for type for next hop
   */
  if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_NEW))
    {
      UNSET_FLAG (pn->flag, PIM_NEXTHOP_NEW);
    }
  SET_FLAG (pn->flag, PIM_NEXTHOP_S);
  

  /* Lookup neighbor using the nexthop.  */
  nexthop = pn->nexthop;
  if (nexthop->ifindex)
    {
      ifp = if_lookup_by_index (&vr->ifm, nexthop->ifindex);
      if (! ifp)
	return NULL;

      vif = ifp->info;
      if (vif->index < 0)
	return NULL;

      return pim_nbr (vif->index, &nexthop->gate.ipv4);
    }
  return NULL;
}

/* MRIB.next_hop (RP(G)) macro.  Argument is address of RP.  */
struct pim_neighbor *
pim_mrib_next_hop_rp (struct pal_in4_addr *addr)
{
  struct pim_nexthop *pn;
  struct nexthop *nexthop;
  struct interface *ifp;
  struct pim_vif *vif;
  struct pim_neighbor *nbr;
  struct ipi_vr *vr = ipi_vr_get_privileged (pim_top->zg);

  /* Lookup nexthop */
  pn = pim_nexthop_lookup (addr);
  if (! pn)
    {
      if (IS_DEBUG_PIM_NEXTHOP(pim_top))
	PIMLOG_INFO (pim_top->zg, "MRIB.next_hop_rp(%r): Can't find nexthop",  addr);
      return NULL;
    }
  else
    if (IS_DEBUG_PIM_NEXTHOP(pim_top))
      PIMLOG_INFO (pim_top->zg, "MRIB.next_hop_rp(%r): nexthop %r",  addr, &pn->nexthop->gate.ipv4);

  /* Set the flag for type for next hop
   */
  if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_NEW))
    {
      UNSET_FLAG (pn->flag, PIM_NEXTHOP_NEW);
    }
  SET_FLAG (pn->flag, PIM_NEXTHOP_RP);

  /* Lookup neighbor using the nexthop.  */
  nexthop = pn->nexthop;
  if (nexthop->gate.ipv4.s_addr)
    {
      ifp = if_lookup_by_index (&vr->ifm, nexthop->ifindex);
      if (! ifp)
	return NULL;

      vif = ifp->info;
      if (vif->index < 0)
	return NULL;

      nbr = pim_nbr (vif->index, &nexthop->gate.ipv4);
    }
  else
    {
      ifp = if_lookup_by_index (&vr->ifm, nexthop->ifindex);
      if (! ifp)
	return NULL;

      vif = ifp->info;
      if (vif->index < 0)
	return NULL;

      nbr = pim_nbr (vif->index, addr);
    }
  return nbr;
}

/* MRIB.metric(X) macro.  Argument is address of RP or S.  Returns an
   infinite metric if there is no route to the destination Reference:
   draft-05, pg. 70.  */
u_int32_t
pim_mrib_metric (struct pal_in4_addr *addr)
{
  struct pim_nexthop *pn;

  /* Lookup nexthop.  */
  pn = pim_nexthop_lookup (addr);
  if (! pn)
    {
      if (IS_DEBUG_PIM_NEXTHOP(pim_top))
	PIMLOG_INFO (pim_top->zg, "MRIB.metric(%r): Can't find nexthop",  addr);
      return PIM_ASSERT_INF;
    }
  else
    {
      if (IS_DEBUG_PIM_NEXTHOP(pim_top))
     	PIMLOG_INFO (pim_top->zg, "MRIB.metric(%r): nexthop %r metric %d",  
		   addr, &pn->nexthop->gate.ipv4, pn->metric);
      return pn->metric;
    }
}

/* MRIB.pref(X) macro.  Argument is address of RP or S.  Returns an
   infinite preference if there is no route to the destination
   Reference: draft-05, pg. 70.  */
u_int32_t
pim_mrib_pref (struct pal_in4_addr *addr)
{
  struct pim_nexthop *pn;

  /* Lookup nexthop */
  pn = pim_nexthop_lookup (addr);
  if (! pn)
    {
      if (IS_DEBUG_PIM_NEXTHOP(pim_top))
	PIMLOG_INFO (pim_top->zg, "MRIB.pref(%r): Can't find nexthop",  addr);
      return PIM_ASSERT_INF;
    }
  else
    {
      if (IS_DEBUG_PIM_NEXTHOP(pim_top))
     	PIMLOG_INFO (pim_top->zg, "MRIB.pref(%r): nexthop %r preference %d",  
		   addr, &pn->nexthop->gate.ipv4, pn->preference);
      return ((u_int32_t)pn->preference);
    }
}

/* Check RP nexthop.  */
struct pim_nexthop *
pim_nexthop_lookup (struct pal_in4_addr *addr)
{
  struct prefix p;
  struct route_node *rn;
  struct pim_nexthop *pn;

  if (! pim_top)
    return NULL;

  /* Prepare prefix structure.  */
  pal_mem_set (&p, 0, sizeof (struct prefix));
  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_BITLEN;
  p.u.prefix4 = *addr;

  /* First lookup cache.  */
  rn = route_node_get (pim_top->nexthop_cache, &p);
  pn = rn->info;

  /* When there is a cache information immediately return it.  */
  if (pn)
    {
      route_unlock_node (rn);

      if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_UNR))
	{
	  if (IS_DEBUG_PIM_NEXTHOP(pim_top))
	    PIMLOG_INFO (pim_top->zg, "Nexthop for %r unreachable",  
		       addr);
	  return NULL;
	}

      /* Return nexthop information.  */
      return pn;
    }

  /* Query to NSM.  */
  pn = pim_nexthop_query (pim_top,  addr);


  /* When nexthop is not reachable, put negative cache.  */
  if (! pn)
    {
      if (IS_DEBUG_PIM_NEXTHOP(pim_top))
	PIMLOG_INFO (pim_top->zg, "Nexthop for %r not found in NSM",  
		   addr);
      
      pn = pim_nexthop_new ();

      if (pn == NULL)
        return NULL;

      pn->refcnt = 0;

      pn->metric = 0;
      pn->preference = 0;
      pn->nexthop_num = 0;

      SET_FLAG (pn->flag, PIM_NEXTHOP_NEW);
      SET_FLAG (pn->flag, PIM_NEXTHOP_UNR);

      /* Copy the resolved address to the pim_nexthop structure
       */
      pn->addr = *addr;
      
      /* Link this new unresolved nexthop */
      rn->info = pn;
      pn->rn = rn;
      return NULL;
    }

  /* Add new PIM nexthop information.  */
  rn->info = pn;
  pn->rn = rn;

  return pn;
}

int
pim_nexthop_same (struct nexthop *n1, struct nexthop *n2)
{
  if (! IPV4_ADDR_SAME (&n1->gate.ipv4, &n2->gate.ipv4)
      || n1->ifindex != n2->ifindex)
    {
      return 0; /* different */
    }
  return 1;
}

int
pim_nexthop_changed (struct pim_nexthop *pn1, struct pim_nexthop *pn2)
{
  int i;
  struct nexthop *n1, *n2;

  /* If the reachability changes, the nexthops have changed */
  if ( CHECK_FLAG (pn1->flag, PIM_NEXTHOP_UNR) !=
       CHECK_FLAG (pn2->flag, PIM_NEXTHOP_UNR) )
    return 1;

  if (pn1->nexthop_num != pn2->nexthop_num)
    return 1;

  n1 = pn1->nexthop;
  n2 = pn2->nexthop;

  for (i = 0; i < pn1->nexthop_num; i++)
    {
      if (! pim_nexthop_same (n1, n2))
	return 1;
      n1 = n1->next;
      n2 = n2->next;
    }
  if ((pn1->metric != pn2->metric) ||
      (pn1->preference != pn2->preference))
    return 1;	      
  return 0;
}

/* Periodical check of PIM RP nexthop.  */
int
pim_nexthop_scan (struct thread *t)
{
  struct pim_top *pim_top; 
  struct route_node *rn;
  struct pim_nexthop *pn;
  struct pim_nexthop *update;

  pim_top = THREAD_ARG (t);
  pim_top->t_nexthop = NULL;

  PIM_TIMER_ON ( pim_top->t_nexthop, pim_nexthop_scan, pim_top, 
		 PIM_NEXTHOP_SCAN_INTERVAL_DEFAULT);

  /* Examine all of nexthop information.  */
  for (rn = route_top (pim_top->nexthop_cache); rn; rn = route_next (rn))
    if ((pn = rn->info) != NULL)
      {
	/* Query to NSM.  */
	update = pim_nexthop_query (pim_top,  &rn->p.u.prefix4);

	/* Update nexthop reachability.  */
	if (update)
	  {
	    /* Check nexthop is changed or not.  */
	    if (pim_nexthop_changed (pn, update))
	      {
		if(IS_DEBUG_PIM_NEXTHOP(pim_top))
		  PIMLOG_INFO (pim_top->zg, "Nexthop for %s %r has changed", 
			     ((CHECK_FLAG(pn->flag, PIM_NEXTHOP_RP))
			      ? "RP" : "Source"), &pn->addr);

		/* It is important to update the Patricia node with
		   the new nexthop so that the PIM-SM macros in the
		   pim_handle_nexthop_change_rp() function get the new
		   nexthop.  */
		rn->info = update;
		update->rn = rn;

		/* Copy the refcnt from old nexthop to new nexthop */
		update->refcnt = pn->refcnt;

		/* Handle the change in nexthop.  */
                if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_NEW))
                  {
                    pim_handle_nexthop_change_rp (pn);
                    pim_handle_nexthop_change_s (pn);
                  }
                else
                  {
                    if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_RP))
                      pim_handle_nexthop_change_rp (pn);
                    if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_S))
                      pim_handle_nexthop_change_s (pn);
                  }

		/* Free the old nexthop */
		pim_nexthop_free (pn);
	      }
	    else
	      {
		pim_nexthop_free (update);
	      }
	  }
	else
	  {
	    /* If the nexthop is already unreachable, do nothing */
	    if (!CHECK_FLAG (pn->flag, PIM_NEXTHOP_UNR))
	      {
				/* Set the unreachable flag */
		SET_FLAG (pn->flag, PIM_NEXTHOP_UNR);

	    	/* Nexthop becomes unreachable.  */
	    	if(IS_DEBUG_PIM_NEXTHOP(pim_top))
		  PIMLOG_INFO (pim_top->zg, "Nexthop for %s %r has become unreachable", 
			     ((CHECK_FLAG(pn->flag, PIM_NEXTHOP_RP))
			      ? "RP":"Source"), &pn->addr);

	    	/* Handle the change in nexthop.  */
                if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_NEW))
                  {
                    pim_handle_nexthop_change_rp (pn);
                    pim_handle_nexthop_change_s (pn);
                  }
                else
                  {
                    if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_RP))
                      pim_handle_nexthop_change_rp (pn);
                    if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_S))
                      pim_handle_nexthop_change_s (pn);
                  }
	      }
	  }
      }
  return 0;
}

void 
pim_nexthop_inc_refcnt (struct pim_nexthop *pn)
{
  PIM_TRAP (pn != NULL);
  ++pn->refcnt;
  if (IS_DEBUG_PIM_NEXTHOP(pim_top))
    PIMLOG_INFO (pim_top->zg, "Nexthop %r: Increment refcnt %d", &pn->addr, pn->refcnt); 
}

void
pim_nexthop_dec_refcnt (struct pim_nexthop *pn)
{
  struct route_node *rn;

  PIM_TRAP (pn != NULL);

  rn = pn->rn;
  if ((pn->refcnt == 0) || (--pn->refcnt == 0))
    {
      if (IS_DEBUG_PIM_NEXTHOP(pim_top))
        PIMLOG_INFO (pim_top->zg, "Nexthop %r: Decrement refcnt %d", &pn->addr, pn->refcnt); 
      pim_nexthop_free (pn);
      rn->info = NULL;
      route_unlock_node (rn);
      return;
    }
  if (IS_DEBUG_PIM_NEXTHOP(pim_top))
    PIMLOG_INFO (pim_top->zg, "Nexthop %r: Decrement refcnt %d", &pn->addr, pn->refcnt); 
}

/* Start NSM connection for nexthop lookup.  */
int
pim_nexthop_start (struct thread *t)
{
  struct pim_top *pim_top; 

  pim_top = THREAD_ARG (t);
  pim_top->t_nexthop = NULL;

  /* Start PIM nexthop scan timer. */
  PIM_TIMER_ON ( pim_top->t_nexthop, pim_nexthop_scan, pim_top,  0);

  return 0;
}

/* Initialize query client to NSM.  */
void
pim_nexthop_init (struct pim_top *pim_top)
{
  /* Initialize nexthop cache.  */
  pim_top->nexthop_cache = route_table_init ();

  if (pim_top->nexthop_cache)
    /* Register event to connect NSM.  */
    pim_top->t_nexthop = thread_add_event (pim_top->zg, pim_nexthop_start, pim_top,  0);
}

void
pim_nexthop_shutdown (struct pim_top *pim_top)
{
  struct route_node *rn;
  struct pim_nexthop *pn;

  for (rn = route_top (pim_top->nexthop_cache); rn; rn = route_next (rn))
    {
      if ((pn = rn->info) == NULL)
        continue;
      pim_nexthop_free (pn);

      rn->info = NULL;
      route_unlock_node (rn);
    }
  route_table_finish (pim_top->nexthop_cache);
  pim_top->nexthop_cache = NULL;

  PIM_TIMER_OFF (pim_top->t_nexthop);
  return;
}

/* Clean up unreachable nexthops from the cache */
void
pim_nexthop_clean_unreachable (struct pim_top *pim_top, 
    struct pal_in4_addr *dest)
{
  struct route_node *rn;
  struct pim_nexthop *pn;
  struct prefix p;

  /* Prepare prefix structure.  */
  pal_mem_set (&p, 0, sizeof (struct prefix));
  p.family = AF_INET;
  p.prefixlen = IPV4_MAX_BITLEN;
  p.u.prefix4 = *dest;

  /* First lookup cache.  */
  rn = route_node_lookup (pim_top->nexthop_cache, &p);
  if (rn == NULL)
    return;

  route_unlock_node (rn);

  if ((pn = rn->info) == NULL)
    return;

  /* Check if the nexthop is new, unreachable and refcnt = 0 */
  if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_NEW) &&
      CHECK_FLAG (pn->flag, PIM_NEXTHOP_UNR) && 
       pn->refcnt == 0)
    {
      pim_nexthop_free (pn);

      rn->info = NULL;
      route_unlock_node (rn);
    }

  return;
}
