/* Copyright (C) 2002-2003 IP Infusion, Inc. All Rights Reserved. */

#ifndef _ZEBOS_PIM_NEXTHOP_H
#define _ZEBOS_PIM_NEXTHOP_H

struct pim_neighbor *pim_mrib_next_hop_rp (struct pal_in4_addr *);
struct pim_neighbor *pim_mrib_next_hop_s (struct pal_in4_addr *);
struct pim_nexthop *pim_nexthop_lookup (struct pal_in4_addr *);
u_int32_t pim_mrib_metric (struct pal_in4_addr *);
u_int32_t pim_mrib_pref (struct pal_in4_addr *);
void pim_nexthop_inc_refcnt (struct pim_nexthop *);
void pim_nexthop_dec_refcnt (struct pim_nexthop *);
void pim_nexthop_clean_unreachable (struct pim_top *, struct pal_in4_addr *);

#endif
