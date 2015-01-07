/* Copyright (C) 2002-2003 IP Infusion, Inc. All Rights Reserved. */

#include <kroute.h>

#include "if.h"
#include "command.h"
#include "prefix.h"
#include "table.h"
#include "thread.h"
#include "memory.h"
#include "log.h"
#include "stream.h"
#include "filter.h"
#include "sockunion.h"
#include "sockopt.h"
#include "plist.h"
#include "privs.h"

#include "pimd/pimd.h"
#include "pimd/pim_vif.h"
#include "pimd/pim_igmp.h"
#include "pimd/pim_mrt.h"
#include "pimd/pim_rp.h"
#include "pimd/pim_nexthop.h"
#include "pimd/pim_nsm.h"

#define DEFUN_ACCESS_KROUTE_STR "IP Kroute access-list name"
#define DEFUN_PIMSM_STR "Sparse Mode (PIM-SM)"

extern char *pim_us_states[];
extern char *pim_us_sgrpt_states[];
extern char *pim_bsr_states[];
extern char *pim_bsr_roles[];
extern const char* pim_local_info_str [];

struct cmd_node interface_node = {
  INTERFACE_NODE,
  "%s(config-if)# ",
  1,
};

struct cmd_node pim_node = {
  PIM_NODE,
  "",
  1,
};
	
/* PIM interface commands.  */

DEFUN (ip_pim_mode,
     ip_pim_mode_cmd,
     "ip pim sparse-mode",
     IP_STR,
     "PIM interface commands",
     "Enable PIM sparse-mode operation")
{
  struct interface *ifp;

  ifp = vty->index;

  pim_vif_activate (ifp->info);

  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_mode,
     no_ip_pim_sparse_mode_cmd,
     "no ip pim sparse-mode",
     NO_STR,
     IP_STR,
     "PIM interface commands",
     "Enable PIM sparse-mode operation")
{
  struct interface *ifp;

  ifp = vty->index;

  pim_vif_deactivate (ifp->info);

  return CMD_SUCCESS;
}

DEFUN (ip_pim_mode_passive,
     ip_pim_mode_passive_cmd,
     "ip pim sparse-mode passive",
     IP_STR,
     "PIM interface commands",
     "Enable PIM sparse-mode operation",
     "PIM passive mode (local members only)")
{
  struct interface *ifp;

  ifp = vty->index;

  pim_vif_passive (ifp->info);

  return CMD_SUCCESS;
}

ALIAS   (ip_pim_mode,
       no_ip_pim_mode_passive_cmd,
       "no ip pim sparse-mode passive",
       NO_STR,
       IP_STR,
       "PIM interface commands",
       "Enable PIM sparse-mode operation",
       "PIM passive mode (local members only)");

DEFUN (ip_pim_nbr_flt,
    ip_pim_sm_nbr_flt_cmd,
    "(ip) pim neighbor-filter (<1-99>|WORD)",
    IP_STR,
    "PIM Interface commands",
    "PIM peering filter",
    "IP standard access list",
    DEFUN_ACCESS_KROUTE_STR)
{
  struct interface *ifp;

  ifp = vty->index;
  pim_vif_nbr_flt_set (ifp->info, argv[0]);

  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_nbr_flt,
    no_ip_pim_sm_nbr_flt_cmd,
    "no (ip) pim neighbor-filter (<1-99>|WORD)",
    NO_STR,
    IP_STR,
    "PIM Interface commands",
    "PIM peering filter",
    "IP standard access list",
    DEFUN_ACCESS_KROUTE_STR)
{
  struct interface *ifp;

  ifp = vty->index;
  pim_vif_nbr_flt_unset (ifp->info, argv[0]);

  return CMD_SUCCESS;
}

DEFUN (ip_pim_dr_priority,
    ip_pim_dr_priority_cmd,
    "ip pim dr-priority <0-4294967294>",
    IP_STR,
    "PIM Interface commands",
    "PIM router DR priority",
    "DR priority, preference given to larger value")
{
  struct interface *ifp;
  u_int32_t val;

  DEFUN_GET_INTEGER ("DR priority", val, argv[0]);

  ifp = vty->index;
  pim_dr_priority_set (ifp->info, val);

  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_dr_priority,
     no_ip_pim_dr_priority_cmd,
     "no ip pim dr-priority",
     NO_STR,
     IP_STR,
     "PIM Interface commands",
     "PIM router DR priority")
{
  struct interface *ifp;

  ifp = vty->index;
  pim_dr_priority_unset (ifp->info);

  return CMD_SUCCESS;
}

ALIAS (no_ip_pim_dr_priority,
     no_ip_pim_dr_priority_val_cmd,
     "no ip pim dr-priority <0-4294967294>",
     NO_STR,
     IP_STR,
     "PIM Interface commands",
     "PIM router DR priority",
     "DR priority, preference given to larger value");

DEFUN (ip_pim_hello_interval,
    ip_pim_hello_interval_cmd,
    "(ip) pim hello-interval <1-65535>",
    IP_STR,
    "PIM Interface commands",
    "PIM Hello message interval",
    "Hello message interval in seconds")
{
  struct interface *ifp;
  u_int32_t val;

  DEFUN_GET_INTEGER_RANGE ("Hello interval", val, argv[0], 1, 65535);

  ifp = vty->index;
  pim_hello_interval_set (ifp->info, (u_int16_t)val);

  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_hello_interval,
    no_ip_pim_hello_interval_cmd,
    "no (ip) pim hello-interval",
    NO_STR,
    IP_STR,
    "PIM Interface commands",
    "PIM Hello message interval")
{
  struct interface *ifp;

  ifp = vty->index;
  pim_hello_interval_unset (ifp->info);

  return CMD_SUCCESS;
}

DEFUN (ip_pim_hello_holdtime,
    ip_pim_hello_holdtime_cmd,
    "(ip) pim hello-holdtime <1-65535>",
    IP_STR,
    "PIM Interface commands",
    "PIM Hello message holdtime",
    "Hello message holdtime in seconds")
{
  struct interface *ifp;
  struct pim_vif *vif;
  u_int32_t val;
  int ret;

  DEFUN_GET_INTEGER_RANGE ("Hello holdtime", val, argv[0], 1, 65535);

  ifp = vty->index;
  vif = ifp->info;
  ret = pim_hello_holdtime_set (vif, (u_int16_t)val);
  if (ret < 0)
    {
      vty_out (vty, "Holdtime %u is less that current Hello interval %d\n",
          val, vif->hello_period);
      return CMD_WARNING;
    }

  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_hello_holdtime,
    no_ip_pim_hello_holdtime_cmd,
    "no (ip) pim hello-holdtime",
    NO_STR,
    IP_STR,
    "PIM Interface commands",
    "PIM Hello message holdtime")
{
  struct interface *ifp;

  ifp = vty->index;
  pim_hello_holdtime_unset (ifp->info);

  return CMD_SUCCESS;
}

DEFUN (ip_pim_exclude_genid,
    ip_pim_exclude_genid_cmd,
    "ip pim exclude-genid",
    IP_STR,
    "PIM Interface commands",
    "Exlcude Gen-id option from PIM Hello packets on this interface")
{
  struct interface *ifp;

  ifp = vty->index;

  pim_vif_set_exclude_genid (ifp->info);

  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_exclude_genid,
    no_ip_pim_exclude_genid_cmd,
    "no ip pim exclude-genid",
    NO_STR,
    IP_STR,
    "PIM Interface commands",
    "Exlcude Gen-id option from PIM Hello packets on this interface")
{
  struct interface *ifp;

  ifp = vty->index;

  pim_vif_unset_exclude_genid (ifp->info);

  return CMD_SUCCESS;
}

int
pim_if_config_write (struct vty *vty)
{
  struct listnode *node;
  struct interface *ifp;
  struct pim_vif *vif;
  int write = 0;

#ifdef HAVE_CUSTOM1
  for (node = LISTHEAD (vty->zg->ifg.if_list); node; NEXTNODE (node))
#else /* HAVE_CUSTOM1 */
  for (node = LISTHEAD (vty->vr->ifm.if_list); node; NEXTNODE (node))
#endif /* HAVE_CUSTOM1 */
    {
      ifp = GETDATA (node);
      vif = ifp->info;

      if (vif == NULL)
        continue;

      vty_out (vty, "interface %s\n", ifp->name);
      
      if (vif->mode == PIM_NODE_SPARSE) 
	vty_out (vty, " ip pim sparse-mode\n");

      if (vif->mode == PIM_NODE_SPARSE_PASSIVE) 
	vty_out (vty, " ip pim sparse-mode passive\n");

      if (CHECK_FLAG (vif->config, PIM_VIF_CONFIG_HELLO_INTERVAL))
	vty_out (vty, " ip pim hello-interval %d\n", vif->hello_period);

      if (CHECK_FLAG (vif->config, PIM_VIF_CONFIG_HELLO_HOLDTIME))
	vty_out (vty, " ip pim hello-holdtime %d\n", vif->hello.holdtime);

      if (CHECK_FLAG (vif->config, PIM_VIF_CONFIG_PRIORITY)
	  && vif->hello.priority != 1)
	vty_out (vty, " ip pim dr-priority %lu\n", vif->hello.priority);
      if (CHECK_FLAG (vif->config, PIM_VIF_EXCLUDE_GENID))
	vty_out (vty, " ip pim exclude-genid\n");
      
      if (vif->nbr_flt)
        vty_out (vty, " ip pim neighbor-filter %s\n", vif->nbr_flt);


      vty_out (vty, "!\n");

      write++;
    }
  return write;
}

/* PIM RP commands.  */
DEFUN (ip_pim_rp_address_default,
     ip_pim_rp_address_default_cmd,
     "ip pim rp-address A.B.C.D",
     IP_STR,
     PIM_STR,
     "PIM RP-address (Rendezvous Point)",
     "IP address of Rendezvous-point")
{
  struct pal_in4_addr rp_addr;

  if (! pim_top)
    return CMD_SUCCESS;

  DEFUN_GET_IPV4_ADDRESS ("RP Address", rp_addr, argv[0]);

  pim_add_static_rp_conf_default (&rp_addr);
  
  return CMD_SUCCESS;
}

DEFUN (ip_pim_rp_address_acl,
     ip_pim_rp_address_acl_cmd,
     "ip pim rp-address A.B.C.D (<1-99>|<1300-1999>|WORD)",
     IP_STR,
     PIM_STR,
     "PIM RP-address (Rendezvous Point)",
     "IP address of Rendezvous-point",
     "IP standard access list",
     "IP standard access list (expanded range)",
     DEFUN_ACCESS_KROUTE_STR)
{
  struct pal_in4_addr rp_addr;

  if (! pim_top)
    return CMD_SUCCESS;

  DEFUN_GET_IPV4_ADDRESS ("RP Address", rp_addr, argv[0]);

  pim_add_static_rp_conf_acl (&rp_addr, argv[1]);
  
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_rp_address_default,
     no_ip_pim_rp_address_default_cmd,
     "no ip pim rp-address A.B.C.D",
     NO_STR,
     IP_STR,
     PIM_STR,
     "PIM RP-address (Rendezvous Point)",
     "IP address of Rendezvous-point")
{
  struct pal_in4_addr rp_addr;

  if (! pim_top)
    return CMD_SUCCESS;

  DEFUN_GET_IPV4_ADDRESS ("RP Address", rp_addr, argv[0]);

  pim_del_static_rp_conf (&rp_addr);
  
  return CMD_SUCCESS;
}

ALIAS (no_ip_pim_rp_address_default,
     no_ip_pim_rp_address_acl_cmd,
     "no ip pim rp-address A.B.C.D (<1-99>|<1300-1999>|WORD)",
     NO_STR,
     IP_STR,
     PIM_STR,
     "PIM RP-address (Rendezvous Point)",
     "IP address of Rendezvous-point",
     "IP standard access list",
     "IP standard access list (expanded range)",
     DEFUN_ACCESS_KROUTE_STR);

DEFUN (ip_pim_autorp_refresh,
    ip_pim_autorp_refresh_cmd,
    "ip pim auto-rp expire-interval <1-16383>",
    IP_STR,
	PIM_STR,
    "Configure Auto-RP",
    "Auto-RP expiration",
    "Auto-RP expiration interval")
{
  u_int16_t timer;
  unsigned long val;

  DEFUN_GET_INTEGER_RANGE ("Auto-RP expiration", val, argv[0], 1, 16383);
  timer = val;

  pim_auto_rp_expire_set(timer);
  return CMD_SUCCESS;
}

DEFUN (ip_pim_jp_timer,
     ip_pim_jp_timer_cmd,
     "ip pim jp-timer <1-65535>",
     IP_STR,
     PIM_STR,
     "Join/Prune timer",
     "Join/Prune timer value")
{
  u_int16_t timer;
  unsigned long val;

  DEFUN_GET_INTEGER_RANGE ("Join/Prune timer", val, argv[0], 0, 65535);
  timer = val;

  pim_jp_timer_set (pim_top, val);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_jp_timer,
       no_ip_pim_jp_timer_cmd,
       "no ip pim jp-timer",
       NO_STR,
       IP_STR,
       PIM_STR,
       "Join/Prune timer")
{
  pim_jp_timer_unset (pim_top);
  return CMD_SUCCESS;
}

ALIAS (no_ip_pim_jp_timer,
     no_ip_pim_jp_timer_val_cmd,
     "no ip pim jp-timer <1-65535>",
     NO_STR,
     IP_STR,
     PIM_STR,
     "Join/Prune timer",
     "Join/Prune timer value");

DEFUN (ip_pim_ignore_rp_set_priority,
     ip_pim_ignore_rp_set_priority_cmd,
     "ip pim ignore-rp-set-priority",
     IP_STR,
     PIM_STR,
     "Ignore RP set priority value")
{
  if (! pim_top)
    return CMD_WARNING;

  pim_set_ignore_rpset_priority (pim_top);

  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_ignore_rp_set_priority,
     no_ip_pim_ignore_rp_set_priority_cmd,
     "no ip pim ignore-rp-set-priority",
     NO_STR,
     IP_STR,
     PIM_STR,
     "Ignore RP set priority value")
{
  if (! pim_top)
    return CMD_WARNING;

  pim_unset_ignore_rpset_priority (pim_top);

  return CMD_SUCCESS;
}


DEFUN (ip_pim_spt_switch,
      ip_pim_spt_switch_cmd,
      "ip pim spt-threshold",
      IP_STR,
      PIM_STR,
      "Source-tree switching threshold")
{
  pim_spt_threshold_set (pim_top, NULL);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_spt_switch,
      no_ip_pim_spt_switch_cmd,
      "no ip pim spt-threshold",
      NO_STR,
      IP_STR,
      PIM_STR,
      "Source-tree switching threshold")
{
  pim_spt_threshold_unset (pim_top, NULL);
  return CMD_SUCCESS;
}

DEFUN (ip_pim_spt_switch_grouplist,
      ip_pim_spt_switch_grouplist_cmd,
      "ip pim spt-threshold group-list (<1-99>|<1300-1999>|WORD)",
      IP_STR,
      PIM_STR,
      "Source-tree switching threshold",
      "Group address access list",
      "IP standard access list",
      "IP standard access list (expanded range)",
      DEFUN_ACCESS_KROUTE_STR)
{
  pim_spt_threshold_set (pim_top, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_spt_switch_grouplist,
      no_ip_pim_spt_switch_grouplist_cmd,
      "no ip pim spt-threshold group-list (<1-99>|<1300-1999>|WORD)",
      NO_STR,
      IP_STR,
      PIM_STR,
      "Source-tree switching threshold",
      "Group address access list",
      "IP standard access list",
      "IP standard access list (expanded range)",
      DEFUN_ACCESS_KROUTE_STR)
{
  pim_spt_threshold_unset (pim_top, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (ip_pim_reg_src_addr,
    ip_pim_reg_src_addr_cmd,
    "ip pim register-source A.B.C.D",
    IP_STR,
    PIM_STR,
    "Source address for PIM Register",
    "IP Address to be used as source")
{
  struct pal_in4_addr src;

  DEFUN_GET_IPV4_ADDRESS ("Source address", src, argv[0]); 
  pim_reg_src_addr_set (&src);
  return CMD_SUCCESS;
}

DEFUN (ip_pim_reg_src_intf,
      ip_pim_reg_src_intf_cmd,
      "ip pim register-source IFNAME",
      IP_STR,
      PIM_STR,
      "Source address for PIM Register",
      "Interface address to be used as source")
{
  pim_reg_src_intf_set (argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_reg_src,
      no_ip_pim_reg_src_cmd,
      "no ip pim register-source",
      NO_STR,
      IP_STR,
      PIM_STR,
      "Source address for PIM Register")
{
  pim_reg_src_unset ();
  return CMD_SUCCESS;
}

DEFUN (ip_pim_reg_rate_limit,
      ip_pim_reg_rate_limit_cmd,
      "ip pim register-rate-limit <1-65535>",
      IP_STR,
      PIM_STR,
      "Rate limit for PIM Registers",
      "Packets per second")
{
  u_int32_t lim;

  DEFUN_GET_INTEGER_RANGE ("Limit", lim, argv[0], 1, 65535); 
  pim_reg_rate_limit_set ((u_int16_t)lim);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_reg_rate_limit,
      no_ip_pim_reg_rate_limit_cmd,
      "no ip pim register-rate-limit",
      NO_STR,
      IP_STR,
      PIM_STR,
      "Rate limit for PIM Registers")
{
  pim_reg_rate_limit_unset ();
  return CMD_SUCCESS;
}


DEFUN (ip_pim_reg_rp_reach,
      ip_pim_reg_rp_reach_cmd,
      "ip pim register-rp-reachability",
      IP_STR,
      PIM_STR,
      "Enable RP reachability check for PIM Registers")
{
  pim_reg_rp_reach_set ();
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_reg_rp_reach,
      no_ip_pim_reg_rp_reach_cmd,
      "no ip pim register-rp-reachability",
      NO_STR,
      IP_STR,
      PIM_STR,
      "Enable RP reachability check for PIM Registers")
{
  pim_reg_rp_reach_unset ();
  return CMD_SUCCESS;
}

DEFUN (ip_pim_rp_reg_kat,
      ip_pim_rp_reg_kat_cmd,
      "ip pim rp-register-kat <1-65535>",
      IP_STR,
      PIM_STR,
      "KAT for (S,G) at RP from PIM Registers",
      "KAT time in secs")
{
  u_int32_t kat;

  DEFUN_GET_INTEGER_RANGE ("KAT", kat, argv[0], 1, 65535);
  pim_rp_reg_kat_set ((u_int16_t)kat);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_rp_reg_kat,
      no_ip_pim_rp_reg_kat_cmd,
      "no ip pim rp-register-kat",
      NO_STR,
      IP_STR,
      PIM_STR,
      "KAT for (S,G) at RP from PIM Registers")
{
  pim_rp_reg_kat_unset ();
  return CMD_SUCCESS;
}

DEFUN (ip_pim_reg_supp,
      ip_pim_reg_supp_cmd,
      "ip pim register-suppression <1-65535>",
      IP_STR,
      PIM_STR,
      "Register Suppression for PIM Registers",
      "Register Suppression time in secs")
{
  u_int32_t supp;

  DEFUN_GET_INTEGER_RANGE ("KAT", supp, argv[0], 1, 65535);
  pim_reg_supp_set ((u_int16_t)supp);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_reg_supp,
      no_ip_pim_reg_supp_cmd,
      "no ip pim register-suppression",
      NO_STR,
      IP_STR,
      PIM_STR,
      "Register Suppression for PIM Registers")
{
  pim_reg_supp_unset ();
  return CMD_SUCCESS;
}

DEFUN (ip_pim_rp_reg_filter,
      ip_pim_rp_reg_filter_cmd,
      "ip pim accept-register list (<100-199>|<2000-2699>|WORD)",
      IP_STR,
      PIM_STR,
      "Register accept filter at RP",
      "Access list",
      "IP extended access list",
      "IP extended access list (expanded range)",
      DEFUN_ACCESS_KROUTE_STR)
{
  pim_rp_reg_filter_set (pim_top, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_rp_reg_filter,
      no_ip_pim_rp_reg_filter_cmd,
      "no ip pim accept-register",
      NO_STR,
      IP_STR,
      PIM_STR,
      "Register accept filter at RP")
{
  pim_rp_reg_filter_unset (pim_top);
  return CMD_SUCCESS;
}

DEFUN (ip_pim_cisco_reg_cksum,
    ip_pim_reg_cksum_cmd,
    "ip pim cisco-register-checksum",
    IP_STR,
    PIM_STR,
    "Calculate Register checksum over whole packet (Cisco compatibility)")
{
  pim_cisco_reg_cksum_set (pim_top);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_cisco_reg_cksum,
    no_ip_pim_reg_cksum_cmd,
    "no ip pim cisco-register-checksum",
    NO_STR,
    IP_STR,
    PIM_STR,
    "Calculate Register checksum over whole packet (Cisco compatibility)")
{
  pim_cisco_reg_cksum_unset (pim_top);
  return CMD_SUCCESS;
}

DEFUN (ip_pim_cisco_reg_cksum_filter,
    ip_pim_reg_cksum_filter_cmd,
    "ip pim cisco-register-checksum group-list (<1-99>|<1300-1999>|WORD)",
    IP_STR,
    PIM_STR,
    "Calculate Register checksum over whole packet (Cisco compatibility)",
    "Group address access list",
    "IP standard access list",
    "IP standard access list (expanded range)",
    DEFUN_ACCESS_KROUTE_STR)
{
  pim_cisco_reg_cksum_filter_set (pim_top, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_cisco_reg_cksum_filter,
    no_ip_pim_reg_cksum_filter_cmd,
    "no ip pim cisco-register-checksum group-list (<1-99>|<1300-1999>|WORD)",
    IP_STR,
    NO_STR,
    PIM_STR,
    "Calculate Register checksum over whole packet (Cisco compatibility)",
    "Group address access list",
    "IP standard access list",
    "IP standard access list (expanded range)",
    DEFUN_ACCESS_KROUTE_STR)
{
  pim_cisco_reg_cksum_filter_unset (pim_top, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (ip_pim_ssm_default,
    ip_pim_ssm_default_cmd,
    "ip pim ssm default",
    IP_STR,
    PIM_STR,
    "Configure Source Specific multicast",
    "Use 232/8 group range for SSM")
{
  pim_configure_ssm_default (pim_top);
  return CMD_SUCCESS;
}

DEFUN (ip_pim_ssm_acl,
    ip_pim_ssm_acl_cmd,
    "ip pim ssm range (<1-99>|WORD)",
    IP_STR,
    PIM_STR,
    "Configure Source Specific multicast",
    "ACL for group range to be used for SSM",
    "IP standard access list",
    DEFUN_ACCESS_KROUTE_STR)
{
  pim_configure_ssm_acl (pim_top, argv[0]);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_ssm,
    no_ip_pim_ssm_cmd,
    "no ip pim ssm",
    NO_STR,
    IP_STR,
    PIM_STR,
    "Configure Source Specific multicast")
{
  pim_unconfigure_ssm (pim_top);
  return CMD_SUCCESS;
}

/* RP mapping show function.  This DEFUN display RP set information.  */
DEFUN (show_ip_pim_rp_mapping,
     show_ip_pim_rp_mapping_cmd,
     "show ip pim sparse-mode rp mapping",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM Rendezvous Point (RP) information",
     "Show group-to-RP mappings")
{
  struct route_node *rn;
  struct pim_rp_set *rp_set;
  struct pim_bsr *bsr;
  struct pim_rp *rp;
  char uptimebuf[TIME_BUF];
  char exptimebuf[TIME_BUF];

  if (! pim_top)
    return CMD_SUCCESS;

  bsr = pim_bsr_lookup (pim_top);

  vty_out (vty, "PIM Group-to-RP Mappings\n");

  if (bsr->role == PIM_BSR_ROLE_CANDIDATE_BSR && 
      bsr->status == PIM_BSR_STATUS_ELECTED_BSR)
    vty_out (vty, "This system is the Bootstrap Router (v2)\n");

  /* BSR originated RP information.  */
  for (rn = route_top (bsr->rp_set); rn; rn = route_next (rn))
    if ((rp_set = rn->info) != NULL)
      {
	vty_out (vty, "Group(s): %r/%d\n",
		 &rp_set->group, rp_set->masklen);

	for (rp = rp_set->head; rp; rp = rp->next)
	  {
	    timeutil_uptime (uptimebuf, TIME_BUF, pal_time_current (NULL) - rp->uptime);
            if (rp->t_holdtime)
              timeutil_uptime (exptimebuf, TIME_BUF, rp->exptime - pal_time_current (NULL));
	    vty_out (vty, "  RP: %r\n", &(rp->addr));
	    vty_out (vty, "    Info source: %r, via bootstrap, priority %d\n",
		     &(rp->from), rp->priority);
            if (rp->t_holdtime)
              vty_out (vty, "         Uptime: %s, expires: %s\n",
                  uptimebuf, exptimebuf);
            else
              vty_out (vty, "         Uptime: %s\n", uptimebuf);
	  }
      }

  /* Static RP information.  */
  for (rn = route_top (pim_top->rp_static); rn; rn = route_next (rn))
    if ((rp_set = rn->info) != NULL)
      {
	vty_out (vty, "Group(s): %r/%d\n",
		 &rp_set->group, rp_set->masklen);
        for (rp = rp_set->head; rp ; rp = rp->next)
          {
            vty_out (vty, "    RP: %r\n", &(rp->addr));
						if (IS_PIM_RP_AUTO(rp))
					    vty_out (vty, "    Info source: %r, via Auto-RP\n", &(rp->from));
            timeutil_uptime (uptimebuf, TIME_BUF, pal_time_current (NULL) - rp->uptime);
            if (rp->t_holdtime)
            {
              timeutil_uptime (exptimebuf, TIME_BUF, rp->exptime - pal_time_current (NULL));
              vty_out (vty, "         Uptime: %s, expires: %s\n",
                  uptimebuf, exptimebuf);
            }
            else
              vty_out (vty, "         Uptime: %s\n", uptimebuf);
          }
      }

  return CMD_SUCCESS;
}

/* This DEFUN display RP information.  */
DEFUN (show_ip_pim_rp_hash,
     show_ip_pim_rp_hash_cmd,
     "show ip pim sparse-mode rp-hash A.B.C.D",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "RP to be chosen based on group selected",
     "Group Address")
{
  struct pal_in4_addr group;
  struct pim_rp *rp;

  /* Group is specified.  */
  DEFUN_GET_IPV4_ADDRESS("Group address", group, argv[0]);
  if (!IN_MULTICAST (pal_ntoh32 (group.s_addr)))
    {
      vty_out (vty, "%%Group %r is not a multicast address\n", &group);
      return CMD_WARNING;
    }
  rp = pim_rp_g (&group);

  if (! rp)
    {
      vty_out (vty, "%% No RP available for this group\n");
      return CMD_WARNING;
    }

  if (IS_PIM_RP_STATIC(rp))
    {
      vty_out (vty, "    RP: %r\n", &(rp->addr));
    }
  else
    {
      vty_out (vty, "    RP: %r\n", &(rp->addr));
      vty_out (vty, "    Info source: %r, via %s\n",
	       &(rp->from), IS_PIM_RP_BSR(rp) ? "bootstrap" :
	       IS_PIM_RP_AUTO(rp) ? "Auto-RP" : "Unknown");
    }
  return CMD_SUCCESS;
}


ALIAS (show_ip_pim_rp_mapping,
     show_ip_pim_rp_mapping_simple_cmd,
     "show ip pim rp mapping",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "PIM Rendezvous Point (RP) information",
     "Show group-to-RP mappings");

DEFUN (ip_pim_bsr_candidate,
     ip_pim_bsr_candidate_cmd,
     "ip pim bsr-candidate IFNAME",
     IP_STR,
     PIM_STR,
     "Candidate bootstrap router (candidate BSR)",
     "Interface name")
{
  pim_bsr_candidate_set (pim_top, argv[0], NULL, NULL);
  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_rp_hash,
     show_ip_pim_rp_hash_simple_cmd,
     "show ip pim rp-hash A.B.C.D",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "RP to be chosen based on group selected",
     "Group Address");
     

DEFUN (ip_pim_bsr_candidate_hash,
     ip_pim_bsr_candidate_hash_cmd,
     "ip pim bsr-candidate IFNAME <0-32>",
     IP_STR,
     PIM_STR,
     "Candidate bootstrap router (candidate BSR)",
     "Interface name",
     "Hash Mask length for RP selection")
{
  u_char hash_masklen;
  unsigned long val;

  DEFUN_GET_INTEGER_RANGE ("Hash masklen", val, argv[1], 0, 32);
  hash_masklen = val;
  
  pim_bsr_candidate_set (pim_top, argv[0], NULL, &hash_masklen);
  return CMD_SUCCESS;
}


DEFUN (ip_pim_bsr_candidate_hash_priority,
     ip_pim_bsr_candidate_hash_priority_cmd,
     "ip pim bsr-candidate IFNAME <0-32> <0-255>",
     IP_STR,
     PIM_STR,
     "Candidate bootstrap router (candidate BSR)",
     "Interface name",
     "Hash Mask length for RP selection",
     "Priority value for candidate bootstrap router")
{
  u_char hash_masklen;
  u_char priority;
  unsigned long val;

  DEFUN_GET_INTEGER_RANGE ("Hash masklen", val, argv[1], 0, 32);
  hash_masklen = val;
  DEFUN_GET_INTEGER_RANGE ("priority", val, argv[2], 0, 255);
  priority = val;
  
  pim_bsr_candidate_set (pim_top, argv[0], &priority, &hash_masklen);
  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_bsr_candidate,
     no_ip_pim_bsr_candidate_cmd,
     "no ip pim bsr-candidate",
     NO_STR,
     IP_STR,
     PIM_STR,
     "Candidate bootstrap router (candidate BSR)")
{
  pim_bsr_candidate_unset (pim_top);
  return CMD_SUCCESS;
}

ALIAS (no_ip_pim_bsr_candidate,
     no_ip_pim_bsr_candidate_arg_cmd,
     "no ip pim bsr-candidate IFNAME",
     NO_STR,
     IP_STR,
     PIM_STR,
     "Candidate bootstrap router (candidate BSR)",
     "Interface name");

DEFUN (show_ip_pim_bsr_router,
     show_ip_pim_bsr_router_cmd,
     "show ip pim sparse-mode bsr-router",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "Bootstrap router (v2)")
{
  struct pim_bsr *bsr;
  char uptimebuf[TIME_BUF];
  char exptimebuf[TIME_BUF];
  pal_time_t current, exptime;
  struct pim_crp *crp;

  if (! pim_top)
    return CMD_SUCCESS;

  bsr = pim_bsr_lookup (pim_top);
  if (! bsr)
    return CMD_SUCCESS;

  /* BSR is active.  */
  if (bsr->t_bst)
    {
      current = pal_time_current (NULL);

      timeutil_uptime (uptimebuf, TIME_BUF, current - bsr->uptime);

      if (bsr->exptime < current)
        exptime = current;
      else
        exptime = bsr->exptime;

      timeutil_uptime (exptimebuf, TIME_BUF, exptime - current);

      vty_out (vty, "PIMv2 Bootstrap information\n");

      if (bsr->role == PIM_BSR_ROLE_CANDIDATE_BSR &&
	  (bsr->status == PIM_BSR_STATUS_PENDING_BSR
	   || bsr->status == PIM_BSR_STATUS_ELECTED_BSR))
	vty_out (vty, "This system is the Bootstrap Router (BSR)\n");

      vty_out (vty, "  BSR address: %r\n", &(bsr->addr));
      vty_out (vty, 
          "  Uptime:      %s, BSR Priority: %d, Hash mask length: %d\n", 
          uptimebuf, bsr->priority, bsr->hash_masklen);

      if (bsr->role == PIM_BSR_ROLE_CANDIDATE_BSR &&
	  bsr->status == PIM_BSR_STATUS_ELECTED_BSR)
	vty_out (vty, "  Next bootstrap message in %s\n", exptimebuf);
      else
	vty_out (vty, "  Expires:     %s\n", exptimebuf);
      vty_out (vty, "  Role: %s\n", pim_bsr_roles[bsr->role]);
      vty_out (vty, "  State: %s\n", pim_bsr_states[bsr->status]);
    }

  /* Display all the C-RPs */
  for (crp = bsr->crp_head; crp; crp = crp->next)
    {
      /* C-RP is active.  */
      if (crp->t_crpt)
        {
          timeutil_uptime (exptimebuf, TIME_BUF, 
              crp->crp_exptime - pal_time_current (NULL));
          vty_out (vty, "\n");
          vty_out (vty, "  Candidate RP: %r(%s)\n", &crp->crp_addr, 
              crp->crp_ifname);
          vty_out (vty, "    Advertisement interval %d seconds\n", 
              crp->crp_advinterval);
          vty_out (vty, "    Next Cand_RP_advertisement in %s\n", exptimebuf);
        }
    }

  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_bsr_router,
     show_ip_pim_bsr_router_simple_cmd,
     "show ip pim bsr-router",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "Bootstrap router (v2)");

DEFUN (ip_pim_rp_cand1,
     ip_pim_rp_cand1_cmd,
     "ip pim rp-candidate IFNAME",
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate",
     "Interface Name")
{
  char *ifname;
  u_char conf = 0;

  ifname = argv[0];

  pim_add_rp_candidate_conf (pim_top, ifname, NULL, NULL, NULL, conf);

  return CMD_SUCCESS;
}

DEFUN (ip_pim_rp_cand2,
     ip_pim_rp_cand2_cmd,
     "ip pim rp-candidate IFNAME priority <0-255> (group-list (<1-99>|WORD)|) "
     "(interval <1-16383>|)",
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate",
     "Interface Name",
     "Candidate-RP priority",
     "C-RP priority value",
     "Group Ranges for this C-RP",
     "IP standard access list",
     DEFUN_ACCESS_KROUTE_STR,
     "C-RP advertisement interval",
     "C-RP advertisement interval in seconds")
{
  unsigned long val;
  u_char pri = 0;
  u_int16_t intvl = 0;
  char *acl = NULL;
  char *ifname;
  u_char conf = 0;
  int i;

  ifname = argv[0];

  SET_FLAG (conf, PIM_CRP_CONFIG_PRIORITY);
  DEFUN_GET_INTEGER_RANGE ("C-RP Priority", val, argv[1], 0, 255);
  pri = val;

  for (i = argc - 1; i > 2; i-=2)
    {
      if (! pal_strcmp (argv[i-1], "interval"))
        {
          SET_FLAG (conf, PIM_CRP_CONFIG_INTERVAL);
          DEFUN_GET_INTEGER_RANGE ("C-RP advertisement intvl", intvl, argv[i], 
              1, 16383);
        }
      else if (! pal_strcmp (argv[i-1], "group-list"))
        {
          SET_FLAG (conf, PIM_CRP_CONFIG_ACL);
          acl = argv[i];
        }
    }

  pim_add_rp_candidate_conf (pim_top, ifname, &pri, &intvl, acl, conf);

  return CMD_SUCCESS;
}

ALIAS (ip_pim_rp_cand2,
     ip_pim_rp_cand3_cmd,
     "ip pim rp-candidate IFNAME priority <0-255> interval <1-16383> "
     "(group-list (<1-99>|WORD)|)",
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate",
     "Interface Name",
     "Candidate-RP priority",
     "C-RP priority value",
     "C-RP advertisement interval",
     "C-RP advertisement interval in seconds",
     "Group Ranges for this C-RP",
     "IP standard access list",
     DEFUN_ACCESS_KROUTE_STR);

DEFUN (ip_pim_rp_cand4,
     ip_pim_rp_cand4_cmd,
     "ip pim rp-candidate IFNAME interval <1-16383> (priority <0-255>|) "
     "(group-list (<1-99>|WORD)|)",
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate",
     "Interface Name",
     "C-RP advertisement interval",
     "C-RP advertisement interval in seconds",
     "Candidate-RP priority",
     "C-RP priority value",
     "Group Ranges for this C-RP",
     "IP standard access list",
     DEFUN_ACCESS_KROUTE_STR)
{
  unsigned long val;
  u_char pri = 0;
  u_int16_t intvl = 0;
  char *acl= NULL;
  char *ifname;
  u_char conf = 0;
  int i;
  ifname = argv[0];

  SET_FLAG (conf, PIM_CRP_CONFIG_INTERVAL);
  DEFUN_GET_INTEGER_RANGE ("C-RP advertisement intvl", intvl, argv[1], 1, 16383);

  for (i = argc - 1; i > 2; i-=2)
    {
      if (! pal_strcmp (argv[i-1], "priority"))
        {
          SET_FLAG (conf, PIM_CRP_CONFIG_PRIORITY);
          DEFUN_GET_INTEGER_RANGE ("C-RP Priority", val, argv[i], 0, 255);
          pri = val;
        }
      else if (! pal_strcmp (argv[i-1], "group-list"))
        {
          SET_FLAG (conf, PIM_CRP_CONFIG_ACL);
          acl = argv[i];
        }
    }

  pim_add_rp_candidate_conf (pim_top, ifname, &pri, &intvl, acl, conf);

  return CMD_SUCCESS;
}

ALIAS (ip_pim_rp_cand4,
     ip_pim_rp_cand5_cmd,
     "ip pim rp-candidate IFNAME interval <1-16383> group-list (<1-99>|WORD) "
     "(priority <0-255>|)",
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate",
     "Interface Name",
     "C-RP advertisement interval",
     "C-RP advertisement interval in seconds",
     "Group Ranges for this C-RP",
     "IP standard access list",
     DEFUN_ACCESS_KROUTE_STR,
     "Candidate-RP priority",
     "C-RP priority value");

DEFUN (ip_pim_rp_cand6,
     ip_pim_rp_cand6_cmd,
     "ip pim rp-candidate IFNAME group-list (<1-99>|WORD) (interval <1-16383>|) "
     "(priority <0-255>|)",
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate",
     "Interface Name",
     "Group Ranges for this C-RP",
     "IP standard access list",
     DEFUN_ACCESS_KROUTE_STR,
     "C-RP advertisement interval",
     "C-RP advertisement interval in seconds",
     "Candidate-RP priority",
     "C-RP priority value")
{
  unsigned long val;
  u_char pri = 0;
  u_int16_t intvl = 0;
  char *acl= NULL;
  char *ifname;
  u_char conf = 0;
  int i;

  ifname = argv[0];

  SET_FLAG (conf, PIM_CRP_CONFIG_ACL);
  acl = argv[1];


  for (i = argc - 1; i > 2; i-=2)
    {
      if (! pal_strcmp (argv[i-1], "priority"))
        {
          SET_FLAG (conf, PIM_CRP_CONFIG_PRIORITY);
          DEFUN_GET_INTEGER_RANGE ("C-RP Priority", val, argv[i], 0, 255);
          pri = val;
        }
      else if (! pal_strcmp (argv[i-1], "interval"))
        {
          SET_FLAG (conf, PIM_CRP_CONFIG_INTERVAL);
          DEFUN_GET_INTEGER_RANGE ("C-RP advertisement intvl", intvl, argv[i], 
              1, 16383);
        }
    }

  pim_add_rp_candidate_conf (pim_top, ifname, &pri, &intvl, acl, conf);

  return CMD_SUCCESS;
}

ALIAS (ip_pim_rp_cand6,
     ip_pim_rp_cand7_cmd,
     "ip pim rp-candidate IFNAME group-list (<1-99>|WORD) priority <0-255> "
     "(interval <1-16383>|)",
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate",
     "Interface Name",
     "Group Ranges for this C-RP",
     "IP standard access list",
     DEFUN_ACCESS_KROUTE_STR,
     "Candidate-RP priority",
     "C-RP priority value",
     "C-RP advertisement interval",
     "C-RP advertisement interval in seconds");

DEFUN (no_ip_pim_rp_cand,
     no_ip_pim_rp_cand_cmd,
     "no ip pim rp-candidate",
     NO_STR,
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate")
{
  pim_del_rp_candidate_conf (pim_top, NULL);

  return CMD_SUCCESS;
}

DEFUN (no_ip_pim_rp_cand_arg,
     no_ip_pim_rp_cand_arg_cmd,
     "no ip pim rp-candidate IFNAME",
     NO_STR,
     IP_STR,
     PIM_STR,
     "PIMv2 RP-candidate",
     "Interface name")
{
  pim_del_rp_candidate_conf (pim_top, argv[0]);

  return CMD_SUCCESS;
}

DEFUN (ip_pim_crp_cisco_prefix,
     ip_pim_crp_cisco_prefix_cmd,
     "ip pim crp-cisco-prefix",
     IP_STR,
     PIM_STR,
     "To be a C-RP working with Cisco BSR")
{
  struct pim_bsr *bsr;
  bsr = pim_bsr_lookup (pim_top);
  if (! bsr)
    return CMD_SUCCESS;
  SET_FLAG (bsr->configs, PIM_BSR_CONFIG_CRP_CISCO_PRIFIX);
  return CMD_SUCCESS;
}


DEFUN (no_ip_pim_crp_cisco_prefix,
     no_ip_pim_crp_cisco_prefix_cmd,
     "no ip pim crp-cisco-prefix",
     NO_STR,
     IP_STR,
     PIM_STR,
     "To be a C-RP working with Cisco BSR")
{
  struct pim_bsr *bsr;
  bsr = pim_bsr_lookup (pim_top);
  if (! bsr)
    return CMD_SUCCESS;
  UNSET_FLAG (bsr->configs, PIM_BSR_CONFIG_CRP_CISCO_PRIFIX);
  return CMD_SUCCESS;
}



/* PIM show commands.  */
DEFUN (show_ip_pim_nexthop,
     show_ip_pim_nexthop_cmd,
     "show ip pim sparse-mode nexthop",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM next hops")
{
  struct route_node *rn;
  struct pim_nexthop *pn;
  char type[5] = {'.', '.', '.', '.', '\0'};
  int header = 1;
  struct pal_in4_addr dummy = {0};

  if (! pim_top)
    return CMD_SUCCESS;

  if (!pim_top->nexthop_cache)
    {
      vty_out (vty, "No PIM next hops\n");
      return CMD_SUCCESS;
    }

  for (rn = route_top (pim_top->nexthop_cache); rn; rn = route_next (rn))
    {
      if ((pn = rn->info) == NULL)
	continue;

      if (header)
	{
	  vty_out (vty, "Flags: N = New, R = RP, S = Source, U = Unreachable\n");
	  vty_out (vty, "Destination     Type  Nexthop   Nexthop         Nexthop  Nexthop Metric Pref  Refcnt\n");
	  vty_out (vty, "                        Num     Addr            Ifindex  Name                       \n");
	  vty_out (vty, "____________________________________________________________________________________\n");
	  header = 0;
	}

      pal_mem_set (type, '.', sizeof (type) - 1);

      if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_NEW))
	type[0] = 'N';
      if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_RP))
	type[1] = 'R';
      if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_S))
	type[2] = 'S';
      if (CHECK_FLAG (pn->flag, PIM_NEXTHOP_UNR))
	type[3] = 'U';

      if (pn->nexthop)
        {
          vty_out (vty, "%-15r %-5s %-9d %-15r %-8d %-7s %-6d %-5d %-6d\n", 
              &pn->addr, type, pn->nexthop_num, &pn->nexthop->gate.ipv4, 
              pn->nexthop->ifindex, 
              ((pn->nexthop->ifname)?(pn->nexthop->ifname):" "), pn->metric, 
              pn->preference, pn->refcnt); 
        }
      else
        {
          vty_out (vty, "%-15r %-5s %-9d %-15r %-8d %-7s %-6d %-5d %-6d\n", 
              &pn->addr, type, pn->nexthop_num, &dummy, -1, " ", pn->metric, 
              pn->preference, pn->refcnt); 
        }

    }

  if (header)
    vty_out (vty, "No PIM next hops\n");


  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_nexthop,
     show_ip_pim_nexthop_simple_cmd,
     "show ip pim nexthop",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "PIM next hops");

void
pim_show_mrt_xxrp (struct vty *vty, struct pim_mrt *mrt)
{
  int i;
  char inherited_olist[PAL_MAXVIFS + 1];
  char joined_olist[PAL_MAXVIFS + 1];
  struct pal_in4_addr dummy_addr = {0};

  pal_mem_set (inherited_olist, 0, sizeof (inherited_olist));
  pal_mem_set (joined_olist, 0, sizeof (joined_olist));

  vty_out (vty, "(*, *, %r)\n", &(mrt->src));
  if (mrt->pim_us.xxrp_us.last_rpf_nbr)
    {
      vty_out (vty, "RPF nbr: %r\n", &(mrt->pim_us.xxrp_us.last_rpf_nbr->addr));
      vty_out (vty, "RPF idx: %s\n", mrt->pim_us.xxrp_us.last_rpf_nbr->vif->ifp->name);
    }
  else
    {
      vty_out (vty, "RPF nbr: %r\n", &dummy_addr);
      vty_out (vty, "RPF idx: %s\n", "None");
    }

  vty_out (vty, "Upstream State: %s\n", pim_us_states[mrt->pim_us.xxrp_us.state]);

  /* Not cisco way.  But useful for debugging.  */
  for (i = 0; i < PAL_MAXVIFS; i++)
    {
      inherited_olist[i] = VIFM_ISSET(i, mrt->inherited_olist) ? 'o' : '.';
      joined_olist[i] = VIFM_ISSET(i, mrt->joined_olist) ? 'j' : '.';
    }

  vty_out (vty, " Joined    %s\n", joined_olist);
  vty_out (vty, " Outgoing  %s\n", inherited_olist);

  return;
}

void
pim_show_mrt_xg (struct vty *vty, struct pim_mrt *mrt)
{
  int i;
  char inherited_olist[PAL_MAXVIFS + 1];
  char joined_olist[PAL_MAXVIFS + 1];
  char asserted_olist[PAL_MAXVIFS + 1];
  char local_olist[PAL_MAXVIFS + 1];
  struct pal_in4_addr dummy_addr = {0};
  struct ptree_node *pn;
  struct pim_xg_s_fcr *fcr;

  pal_mem_set (inherited_olist, 0, sizeof (inherited_olist));
  pal_mem_set (joined_olist, 0, sizeof (joined_olist));
  pal_mem_set (local_olist, 0, sizeof (local_olist));
  pal_mem_set (asserted_olist, 0, sizeof (asserted_olist));

  vty_out (vty, "(*, %r)\n", &(mrt->group));
  vty_out (vty, "RP: %r\n", &(mrt->pim_us.xg_us.last_rp));
  if (mrt->pim_us.xg_us.last_rpf_nbr)
    {
      vty_out (vty, "RPF nbr: %r\n", &(mrt->pim_us.xg_us.last_rpf_nbr->addr));
      vty_out (vty, "RPF idx: %s\n", mrt->pim_us.xg_us.last_rpf_nbr->vif->ifp->name);
    }
  else
    {
      vty_out (vty, "RPF nbr: %r\n", &dummy_addr);
      vty_out (vty, "RPF idx: %s\n", "None");
    }

  vty_out (vty, "Upstream State: %s\n", pim_us_states[mrt->pim_us.xg_us.state]);

  /* Not cisco way.  But useful for debugging.  */
  for (i = 0; i < PAL_MAXVIFS; i++)
    {
      local_olist[i] = VIFM_ISSET(i, mrt->local_olist) ? 'i' : '.';
      joined_olist[i] = VIFM_ISSET(i, mrt->joined_olist) ? 'j' : '.';

      if (mrt->stat[i] == NULL)
	{
	  asserted_olist[i] = '.';
	  continue;
	}

      if (mrt->stat[i]->pim_ds.xg_ds.assert.state == PIM_ASSERT_I_AM_WINNER)
	asserted_olist[i] = 'W';
      else if (mrt->stat[i]->pim_ds.xg_ds.assert.state == PIM_ASSERT_I_AM_LOSER)
	asserted_olist[i] = 'L';
      else
	asserted_olist[i] = '.';

    }

  vty_out (vty, " Local     %s\n", local_olist);
  vty_out (vty, " Joined    %s\n", joined_olist);
  vty_out (vty, " Asserted  %s\n", asserted_olist);

  /* Display the per source S FCRs */
  vty_out (vty, "FCR:\n");
  for (pn = ptree_top (mrt->fcr); pn; pn = ptree_next (pn))
    {
      if ((fcr = pn->info) == NULL)
        continue;
      vty_out (vty, "Source: %r\n", &fcr->addr);
      for (i = 0; i < PAL_MAXVIFS; i++)
        {
          inherited_olist[i] = VIFM_ISSET(i, fcr->inherited_olist) ? 'o' : '.';
        }
      vty_out (vty, " Outgoing  %s\n", inherited_olist);
      if (fcr->fcr_kat)
        vty_out (vty, " KAT timer running, %u seconds remaining\n", 
            thread_timer_remain_second (fcr->fcr_kat));
      else
        vty_out (vty, " KAT timer not running\n"); 
      vty_out (vty, " Packet count %u\n", fcr->pktcount);
    }
  return;
}

void
pim_show_mrt_sg (struct vty *vty, struct pim_mrt *mrt)
{
  int i;
  char inherited_olist[PAL_MAXVIFS + 1];
  char joined_olist[PAL_MAXVIFS + 1];
  char asserted_olist[PAL_MAXVIFS + 1];
  char local_olist[PAL_MAXVIFS + 1];
  struct pal_in4_addr dummy_addr = {0};

  pal_mem_set (inherited_olist, 0, sizeof (inherited_olist));
  pal_mem_set (joined_olist, 0, sizeof (joined_olist));
  pal_mem_set (local_olist, 0, sizeof (local_olist));
  pal_mem_set (asserted_olist, 0, sizeof (asserted_olist));

  vty_out (vty, "(%r, %r)\n", &(mrt->src), 
	   &(mrt->group));
  if (mrt->pim_us.sg_us.last_rpf_nbr)
    {
      vty_out (vty, "RPF nbr: %r\n", &(mrt->pim_us.sg_us.last_rpf_nbr->addr));
      vty_out (vty, "RPF idx: %s\n", mrt->pim_us.sg_us.last_rpf_nbr->vif->ifp->name);
    }
  else
    {
      vty_out (vty, "RPF nbr: %r\n", &dummy_addr);
      vty_out (vty, "RPF idx: %s\n", "None");
    }
  vty_out (vty, "SPT bit: %s\n", (mrt->pim_us.sg_us.spt_bit == PAL_TRUE)?"1":"0");

  vty_out (vty, "Upstream State: %s\n", pim_us_states[mrt->pim_us.sg_us.state]);

  /* Not cisco way.  But useful for debugging.  */
  for (i = 0; i < PAL_MAXVIFS; i++)
    {
      local_olist[i] = VIFM_ISSET(i, mrt->local_olist) ? 'i' : '.';
      inherited_olist[i] = VIFM_ISSET(i, mrt->inherited_olist) ? 'o' : '.';
      joined_olist[i] = VIFM_ISSET(i, mrt->joined_olist) ? 'j' : '.';

      if (mrt->stat[i] == NULL)
	{
	  asserted_olist[i] = '.';
	  continue;
	}

      if (mrt->stat[i]->pim_ds.sg_ds.assert.state == PIM_ASSERT_I_AM_WINNER)
	asserted_olist[i] = 'W';
      else if (mrt->stat[i]->pim_ds.sg_ds.assert.state == PIM_ASSERT_I_AM_LOSER)
	asserted_olist[i] = 'L';
      else
	asserted_olist[i] = '.';

    }

  vty_out (vty, " Local     %s\n", local_olist);
  vty_out (vty, " Joined    %s\n", joined_olist);
  vty_out (vty, " Asserted  %s\n", asserted_olist);
  vty_out (vty, " Outgoing  %s\n", inherited_olist);

  return;
}

void
pim_show_mrt_sgrpt (struct vty *vty, struct pim_mrt *mrt)
{
  int i;
  char inherited_olist[PAL_MAXVIFS + 1];
  char pruned_olist[PAL_MAXVIFS + 1];
  char local_olist[PAL_MAXVIFS + 1];
  struct pim_neighbor *rpf_nbr;
  struct pal_in4_addr dummy_addr = {0};

  pal_mem_set (inherited_olist, 0, sizeof (inherited_olist));
  pal_mem_set (pruned_olist, 0, sizeof (pruned_olist));
  pal_mem_set (local_olist, 0, sizeof (local_olist));

  vty_out (vty, "(%r, %r, rpt)\n", &(mrt->src), 
	   &(mrt->group));
  vty_out (vty, "RP: %r\n", &(mrt->pim_us.sgrpt_us.last_rp));

  rpf_nbr = pim_rpf_sgrpt (&mrt->src, &mrt->group);
  if (rpf_nbr)
    {
      vty_out (vty, "RPF nbr: %r\n", &(rpf_nbr->addr));
      vty_out (vty, "RPF idx: %s\n", rpf_nbr->vif->ifp->name);
    }
  else
    {
      vty_out (vty, "RPF nbr: %r\n", &dummy_addr);
      vty_out (vty, "RPF idx: %s\n", "None");
    }
  vty_out (vty, "Upstream State: %s\n", pim_us_sgrpt_states[mrt->pim_us.sgrpt_us.state]);

  /* Not cisco way.  But useful for debugging.  */
  for (i = 0; i < PAL_MAXVIFS; i++)
    {
      local_olist[i] = VIFM_ISSET(i, mrt->local_olist) ? 'e' : '.';
      pruned_olist[i] = VIFM_ISSET(i, mrt->pruned_olist) ? 'p' : '.';
      inherited_olist[i] = VIFM_ISSET(i, mrt->inherited_olist) ? 'o' : '.';
    }

  vty_out (vty, " Local     %s\n", local_olist);
  vty_out (vty, " Pruned    %s\n", pruned_olist);
  vty_out (vty, " Outgoing  %s\n", inherited_olist);

  return;
}

DEFUN (show_ip_pim_mroute,
       show_ip_pim_mroute_cmd,
       "show ip pim sparse-mode mroute",
       SHOW_STR,
       IP_STR,
       PIM_STR,
       DEFUN_PIMSM_STR,
       "PIM Tree Information Base")
{
  int header = 1;
  struct pim_mrt *mrt;
  struct route_node *rn;

  if (! pim_top)
    return CMD_SUCCESS;
  if (!pim_top->tib)
    return CMD_SUCCESS;

  for (rn = route_top (pim_top->tib); rn; rn = route_next (rn))
    if ((mrt = rn->info) != NULL)
      {
	if (header)
	  {
	    vty_out (vty, "IP Multicast Routing Table\n\n");
	    vty_out (vty, "(*,*,RP) Entries: %d\n", pim_top->no_xxrp);
	    vty_out (vty, "(*,G) Entries: %d\n", pim_top->no_xg);
	    vty_out (vty, "(S,G) Entries: %d\n", pim_top->no_sg);
	    vty_out (vty, "(S,G,rpt) Entries: %d\n", pim_top->no_sgrpt);
	    vty_out (vty, "FCR Entries: %d\n", pim_top->no_fcr);
	    vty_out (vty, "\n");
	    header = 0;
	  }

	switch (mrt->flags)
	  {
	  case PIM_MRT_FLAG_XXRP:
	    pim_show_mrt_xxrp (vty, mrt);
	    vty_out (vty, "\n");
	    break;
	  case PIM_MRT_FLAG_XG:
	    pim_show_mrt_xg (vty, mrt);
	    vty_out (vty, "\n");
	    break;
	  case PIM_MRT_FLAG_SG:
	    pim_show_mrt_sg (vty, mrt);
	    vty_out (vty, "\n");
	    break;
	  case PIM_MRT_FLAG_SGRPT:
	    pim_show_mrt_sgrpt (vty, mrt);
	    vty_out (vty, "\n");
	    break;
	  }	
      }

  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_mroute,
       show_ip_pim_mroute_simple_cmd,
       "show ip pim mroute",
       SHOW_STR,
       IP_STR,
       PIM_STR,
       "PIM Tree Information Base");



DEFUN (show_ip_pim_neighbor,
     show_ip_pim_neighbor_cmd,
     "show ip pim sparse-mode neighbor",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM neighbor information")
{
  int header = 1;
  struct listnode *nn;
  struct interface *ifp;
  struct pim_vif *vif;
  struct route_node *rn;
  struct pim_neighbor *pn = NULL;
  char uptimebuf[TIME_BUF];
  char exptimebuf[TIME_BUF];
  
  LIST_LOOP (vty->vr->ifm.if_list, ifp, nn)
    {
      vif = ifp->info;
      if (! vif)
	continue;

      for (rn = route_top (vif->neighbor); rn; rn = route_next (rn))
	if ((pn = rn->info) != NULL)
	  {	     
	    if (header)
	      {
		vty_out (vty, "Neighbor          Interface          Uptime/Expires    Ver   DR\n");
		vty_out (vty, "Address                                                      Priority/Mode\n");
		header = 0;
	      }

	    timeutil_uptime (uptimebuf, TIME_BUF, pal_time_current (NULL) - pn->uptime);

	    if (! pn->exptime)
	      pal_snprintf (exptimebuf, sizeof ("never"), "never");
	    else
	      timeutil_uptime (exptimebuf, TIME_BUF, pn->exptime - pal_time_current (NULL));

	    vty_out (vty, "%-18r%-19s%8s/%8s v2/S  ", &(pn->addr),
		     ifp->name, uptimebuf, exptimebuf);

	    if (CHECK_FLAG (pn->hello.flags, PIM_HELLO_FLAG_DR_PRIORITY))
	      vty_out (vty, "%d /", pn->hello.priority);
	    else
	      vty_out (vty, "N /");

	    if (pn->dr)
	      vty_out (vty, " DR");

	    vty_out (vty, "\n");
	  }
    }

  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_neighbor,
     show_ip_pim_neighbor_simple_cmd,
     "show ip pim neighbor",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "PIM neighbor information");


DEFUN (show_ip_pim_neighbor_detail,
     show_ip_pim_neighbor_detail_cmd,
     "show ip pim sparse-mode neighbor detail",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM neighbor information",
     "Detailed neighbor information")
{
  struct listnode *nn, *n;
  struct interface *ifp;
  struct pim_vif *vif;
  struct route_node *rn;
  struct pim_neighbor *pn = NULL;
  struct pal_in4_addr *addr;
  
  LIST_LOOP (vty->vr->ifm.if_list, ifp, nn)
    {
      vif = ifp->info;
      if (! vif)
	continue;

      for (rn = route_top (vif->neighbor); rn; rn = route_next (rn))
        {
          if ((pn = rn->info) == NULL)
            continue;

          vty_out (vty, "Nbr %r (%s)\n", &pn->addr, pn->vif->ifp->name);
          if (pn->exptime)
            vty_out (vty, " Expires in %d seconds\n", 
                thread_timer_remain_second(pn->t_hello_neighbor));
          else
            vty_out (vty, " Never Expires\n"); 
          /* Secondary addresses */
          if (LISTCOUNT (pn->hello.addr_list))
            {
              vty_out (vty, "  Secondary addresses:\n");
              LIST_LOOP (pn->hello.addr_list, addr, n)
                {
                  vty_out (vty, "   %r\n", addr);
                }
            }
          vty_out (vty, "\n");
        }
    }

  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_neighbor_detail,
     show_ip_pim_neighbor_simple_detail_cmd,
     "show ip pim neighbor detail",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "PIM neighbor information",
     "Detailed neighbor information");

     
DEFUN (show_ip_pim_interface,
     show_ip_pim_interface_cmd,
     "show ip pim sparse-mode interface",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM interface information")
{
  int header = 1;
  struct listnode *nn;
  struct interface *ifp;
  struct pim_vif *vif;

  LIST_LOOP (vty->vr->ifm.if_list, ifp, nn)
    {
      vif = ifp->info;
      if (! vif)
	continue;

      if (vif->index == -1)
	continue;

      if ((vif->mode != PIM_NODE_SPARSE) &&
          (vif->mode != PIM_NODE_SPARSE_PASSIVE))
	continue;

      if (header)
	{
	  vty_out (vty, "Address          Interface VIFindex Ver/   Nbr    DR    DR\n");
	  vty_out (vty, "                                    Mode   Count  Prior\n");
	  header = 0;
	}

      vty_out (vty, "%-16r %-10s%-8d v2/S   %-6d %-5d ",
	       &(vif->addr), vif->ifp->name, vif->index, 
	       vif->neighbor_cnt, vif->hello.priority);
      vty_out (vty, "%r",
	       CHECK_FLAG (vif->flags, PIM_VIF_FLAG_DR) 
	       ? &(vif->addr) : &(vif->dr_addr));
      vty_out (vty, "\n");
    }

  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_interface,
     show_ip_pim_interface_simple_cmd,
     "show ip pim interface",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "PIM interface information");

DEFUN (show_ip_pim_interface_detail,
     show_ip_pim_interface_detail_cmd,
     "show ip pim sparse-mode interface detail",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM interface information",
     "Detailed interface information")
{
  struct listnode *nn, *n;
  struct interface *ifp;
  struct pim_vif *vif;
  struct pal_in4_addr *addr; 
  struct route_node *rn;
  struct pim_neighbor *nbr;

  LIST_LOOP (vty->vr->ifm.if_list, ifp, nn)
    {
      vif = ifp->info;
      if (! vif)
	continue;

      if (vif->index == -1)
	continue;

      if ((vif->mode != PIM_NODE_SPARSE) &&
          (vif->mode != PIM_NODE_SPARSE_PASSIVE))
	continue;

      vty_out (vty, "%s (vif %d): %s\n", vif->ifp->name, vif->index,
          ((vif->mode == PIM_NODE_SPARSE_PASSIVE) ? "Passive mode" : ""));
      vty_out (vty, "  Address %r, DR %r\n", &vif->addr, &vif->dr_addr);
      if (vif->t_hello)
        vty_out (vty, "  Hello period %d seconds, Next Hello in %d seconds\n",
            vif->hello_period, thread_timer_remain_second (vif->t_hello));
      else
        vty_out (vty, "  Hello period %d seconds\n", vif->hello_period);
      if (vif->t_triggered_hello)
        vty_out (vty, "  Triggered Hello period %d seconds, Next Triggered Hello in %d seconds\n",
            vif->trig_hello_delay, 
            thread_timer_remain_second (vif->t_triggered_hello));
      else
        vty_out (vty, "  Triggered Hello period %d seconds\n", 
            vif->trig_hello_delay);
      /* Secondary addresses */
      if (LISTCOUNT (vif->hello.addr_list))
        {
          vty_out (vty, "  Secondary addresses:\n");
          LIST_LOOP (vif->hello.addr_list, addr, n)
            {
              vty_out (vty, "   %r\n", addr);
            }
        }
      vty_out (vty, "  Neighbors:\n");
      /* Neighbors */
      for (rn = route_top (vif->neighbor); rn; rn = route_next (rn))
        {
          if ((nbr = rn->info) == NULL)
            continue;
          vty_out (vty, "   %r\n", &nbr->addr);
        }
      vty_out (vty, "\n");
    }

  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_interface_detail,
     show_ip_pim_interface_simple_detail_cmd,
     "show ip pim interface detail",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "PIM interface information",
     "Detailed interface information");

void
pim_show_local_members (struct vty *vty, struct pim_vif *vif)
{
  struct ptree_node *pn;
  struct pim_vif_local *vlocal;

  vty_out (vty, "%s:\n", vif->ifp->name);

  for (pn = ptree_top (vif->local_info); pn; pn = ptree_next (pn))
    {
      if ((vlocal = pn->info) == NULL)
        continue;

      switch (vlocal->type)
        {
        case PIM_VIF_LOCAL_X_G:
          vty_out (vty, "  (*, %r) : %s\n", &vlocal->lcl_grp, 
              pim_local_info_str [vlocal->info]);
          break;
        case PIM_VIF_LOCAL_S_G:
          vty_out (vty, "  (%r, %r) : %s\n", &vlocal->lcl_src, &vlocal->lcl_grp, 
              pim_local_info_str [vlocal->info]);
          break;
        }
    }
  return;
}

DEFUN (show_ip_pim_localmembers,
     show_ip_pim_localmembers_cmd,
     "show ip pim sparse-mode local-members",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM local membership information")
{
  int header = 1;
  struct listnode *nn;
  struct interface *ifp;
  struct pim_vif *vif;

  LIST_LOOP (vty->vr->ifm.if_list, ifp, nn)
    {
      vif = ifp->info;
      if (! vif)
	continue;

      if (vif->index == -1)
	continue;

      if ((vif->mode != PIM_NODE_SPARSE) &&
          (vif->mode != PIM_NODE_SPARSE_PASSIVE))
	continue;

      if (header)
	{
	  vty_out (vty, "PIM Local membership information\n\n");
	  header = 0;
	}

      pim_show_local_members (vty, vif);

      vty_out (vty, "\n");
    }

  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_localmembers,
     show_ip_pim_localmembers_simple_cmd,
     "show ip pim local-members",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "PIM local membership information");
     

DEFUN (show_ip_pim_localmembers_ifname,
     show_ip_pim_localmembers_ifname_cmd,
     "show ip pim sparse-mode local-members IFNAME",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM local membership information",
     "Interface name")
{
  struct interface *ifp;
  struct pim_vif *vif;

  ifp = if_lookup_by_name (&vty->vr->ifm, argv[0]);
  if (!ifp)
    return CMD_SUCCESS;

  vif = ifp->info;
  if (! vif)
    return CMD_SUCCESS;

  if (vif->index == -1)
    return CMD_SUCCESS;

  if ((vif->mode != PIM_NODE_SPARSE) &&
      (vif->mode != PIM_NODE_SPARSE_PASSIVE))
    return CMD_SUCCESS;

  vty_out (vty, "PIM Local membership information\n\n");

  pim_show_local_members (vty, vif);

  vty_out (vty, "\n");

  return CMD_SUCCESS;
}

ALIAS (show_ip_pim_localmembers_ifname,
     show_ip_pim_localmembers_ifname_simple_cmd,
     "show ip pim local-members IFNAME",
     SHOW_STR,
     IP_STR,
     PIM_STR,
     "PIM local membership information",
     "Interface name");


DEFUN (show_debugging_pim,
     show_debugging_pim_cmd,
     "show debugging pim sparse-mode",
     SHOW_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR)
{
  vty_out (vty, "Debugging status:\n");

  if (IS_DEBUG_PIM_EVENT(pim_top))
    vty_out (vty, "  PIM event debugging is on\n");

  if (IS_DEBUG_PIM_MFC(pim_top))
    vty_out (vty, "  PIM MFC debugging is on\n");

  if (IS_DEBUG_PIM_STATE(pim_top))
    vty_out (vty, "  PIM state debugging is on\n");

  if (IS_DEBUG_PIM_PACKET_IN(pim_top) && IS_DEBUG_PIM_PACKET_OUT(pim_top))
    vty_out (vty, "  PIM packet debugging is on\n");
  else
    {
      if (IS_DEBUG_PIM_PACKET_IN(pim_top))
	vty_out (vty, "  PIM incoming packet debugging is on\n");
      if (IS_DEBUG_PIM_PACKET_OUT(pim_top))
	vty_out (vty, "  PIM outgoing packet debugging is on\n");
    }

  if (IS_DEBUG_PIM_TIMER_HELLO_HT(pim_top))
    vty_out (vty, "  PIM Hello HT timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_HELLO_NLT(pim_top))
    vty_out (vty, "  PIM Hello NLT timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_HELLO_THT(pim_top))
    vty_out (vty, "  PIM Hello THT timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_JP_JT(pim_top))
    vty_out (vty, "  PIM Join/Prune JT timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_JP_ET(pim_top))
    vty_out (vty, "  PIM Join/Prune ET timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_JP_PPT(pim_top))
    vty_out (vty, "  PIM Join/Prune PPT timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_JP_KAT(pim_top))
    vty_out (vty, "  PIM Join/Prune KAT timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_JP_OT(pim_top))
    vty_out (vty, "  PIM Join/Prune OT timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_ASSERT_AT(pim_top))
    vty_out (vty, "  PIM Assert AT timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_REG_RST(pim_top))
    vty_out (vty, "  PIM Register RST timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_BSR_BST(pim_top))
    vty_out (vty, "  PIM Bootstrap BST timer debugging is on\n");
  if (IS_DEBUG_PIM_TIMER_BSR_CRP(pim_top))
    vty_out (vty, "  PIM Bootstrap CRP timer debugging is on\n");
  if (IS_DEBUG_PIM_MIB(pim_top))
    vty_out (vty, "  PIM mib debugging is on\n");
  if (IS_DEBUG_PIM_NSM(pim_top))
    vty_out (vty, "  PIM nsm debugging is on\n");
  if (IS_DEBUG_PIM_NEXTHOP(pim_top))
    vty_out (vty, "  PIM nexthop debugging is on\n");

  return CMD_SUCCESS;
}

DEFUN (debug_pim_state,
     debug_pim_state_cmd,
     "debug pim sparse-mode state",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM state")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, STATE);
  TERM_DEBUG_ON (pim_top, pim, STATE);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_state,
     no_debug_pim_state_cmd,
     "no debug pim sparse-mode state",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM state")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, STATE);
  TERM_DEBUG_OFF (pim_top, pim, STATE);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_events,
     debug_pim_events_cmd,
     "debug pim sparse-mode events",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM events")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, EVENT);
  TERM_DEBUG_ON (pim_top, pim, EVENT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_events,
     no_debug_pim_events_cmd,
     "no debug pim sparse-mode events",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM events")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, EVENT);
  TERM_DEBUG_OFF (pim_top, pim, EVENT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_mfc,
     debug_pim_mfc_cmd,
     "debug pim sparse-mode mfc",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM MFC updates")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, MFC);
  TERM_DEBUG_ON (pim_top, pim, MFC);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_mfc,
     no_debug_pim_mfc_cmd,
     "no debug pim sparse-mode mfc",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM MFC updates")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, MFC);
  TERM_DEBUG_OFF (pim_top, pim, MFC);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer,
     debug_pim_timer_cmd,
     "debug pim sparse-mode timer",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_ALL);
  TERM_DEBUG_ON (pim_top, pim, TIMER_ALL);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_hello,
     debug_pim_timer_hello_cmd,
     "debug pim sparse-mode timer hello",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Hello Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_HELLO_ALL);
  TERM_DEBUG_ON (pim_top, pim, TIMER_HELLO_ALL);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_hello_ht,
     debug_pim_timer_hello_ht_cmd,
     "debug pim sparse-mode timer hello ht",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Hello Timers",
     "Hello timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_HELLO_HT);
  TERM_DEBUG_ON (pim_top, pim, TIMER_HELLO_HT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_hello_nlt,
     debug_pim_timer_hello_nlt_cmd,
     "debug pim sparse-mode timer hello nlt",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Hello Timers",
     "Neighbor Liveliness timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_HELLO_NLT);
  TERM_DEBUG_ON (pim_top, pim, TIMER_HELLO_NLT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_hello_tht,
     debug_pim_timer_hello_tht_cmd,
     "debug pim sparse-mode timer hello tht",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Hello Timers",
     "Triggered Hello timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_HELLO_THT);
  TERM_DEBUG_ON (pim_top, pim, TIMER_HELLO_THT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_jp,
     debug_pim_timer_jp_cmd,
     "debug pim sparse-mode timer joinprune",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_JP_ALL);
  TERM_DEBUG_ON (pim_top, pim, TIMER_JP_ALL);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_jp_jt,
     debug_pim_timer_jp_jt_cmd,
     "debug pim sparse-mode timer joinprune jt",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "JoinPrune timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_JP_JT);
  TERM_DEBUG_ON (pim_top, pim, TIMER_JP_JT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_jp_et,
     debug_pim_timer_jp_et_cmd,
     "debug pim sparse-mode timer joinprune et",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "Expiry timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_JP_ET);
  TERM_DEBUG_ON (pim_top, pim, TIMER_JP_ET);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_jp_ppt,
     debug_pim_timer_jp_ppt_cmd,
     "debug pim sparse-mode timer joinprune ppt",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "Prune Pending timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_JP_PPT);
  TERM_DEBUG_ON (pim_top, pim, TIMER_JP_PPT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_jp_kat,
     debug_pim_timer_jp_kat_cmd,
     "debug pim sparse-mode timer joinprune kat",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "Keep Alive timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_JP_KAT);
  TERM_DEBUG_ON (pim_top, pim, TIMER_JP_KAT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_jp_ot,
     debug_pim_timer_jp_ot_cmd,
     "debug pim sparse-mode timer joinprune ot",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "Override timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_JP_OT);
  TERM_DEBUG_ON (pim_top, pim, TIMER_JP_OT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_assert,
     debug_pim_timer_assert_cmd,
     "debug pim sparse-mode timer assert",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Assert Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_ASSERT_AT);
  TERM_DEBUG_ON (pim_top, pim, TIMER_ASSERT_AT);
  return CMD_SUCCESS;
}

ALIAS (debug_pim_timer_assert,
     debug_pim_timer_assert_at_cmd,
     "debug pim sparse-mode timer assert at",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Assert Timers",
     "Assert timer");

DEFUN (debug_pim_timer_reg,
     debug_pim_timer_reg_cmd,
     "debug pim sparse-mode timer register",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Register Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_REG_RST);
  TERM_DEBUG_ON (pim_top, pim, TIMER_REG_RST);
  return CMD_SUCCESS;
}

ALIAS (debug_pim_timer_reg,
     debug_pim_timer_reg_rst_cmd,
     "debug pim sparse-mode timer register rst",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Register Timers",
     "Register Stop timer");

DEFUN (debug_pim_timer_bsr,
     debug_pim_timer_bsr_cmd,
     "debug pim sparse-mode timer bsr",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "BSR Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_BSR_ALL);
  TERM_DEBUG_ON (pim_top, pim, TIMER_BSR_ALL);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_bsr_bst,
     debug_pim_timer_bsr_bst_cmd,
     "debug pim sparse-mode timer bsr bst",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "BSR Timers",
     "Bootstrap timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_BSR_BST);
  TERM_DEBUG_ON (pim_top, pim, TIMER_BSR_BST);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_timer_bsr_crp,
     debug_pim_timer_bsr_crp_cmd,
     "debug pim sparse-mode timer bsr crp",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "BSR Timers",
     "Candidate-RP timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, TIMER_BSR_CRP);
  TERM_DEBUG_ON (pim_top, pim, TIMER_BSR_CRP);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer,
     no_debug_pim_timer_cmd,
     "no debug pim sparse-mode timer",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_ALL);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_ALL);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_hello,
     no_debug_pim_timer_hello_cmd,
     "no debug pim sparse-mode timer hello",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Hello Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_HELLO_ALL);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_HELLO_ALL);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_hello_ht,
     no_debug_pim_timer_hello_ht_cmd,
     "no debug pim sparse-mode timer hello ht",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Hello Timers",
     "Hello timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_HELLO_HT);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_HELLO_HT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_hello_nlt,
     no_debug_pim_timer_hello_nlt_cmd,
     "no debug pim sparse-mode timer hello nlt",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Hello Timers",
     "Neighbor Liveliness timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_HELLO_NLT);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_HELLO_NLT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_hello_tht,
     no_debug_pim_timer_hello_tht_cmd,
     "no debug pim sparse-mode timer hello tht",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Hello Timers",
     "Triggered Hello timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_HELLO_THT);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_HELLO_THT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_jp,
     no_debug_pim_timer_jp_cmd,
     "no debug pim sparse-mode timer joinprune",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_JP_ALL);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_JP_ALL);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_jp_jt,
     no_debug_pim_timer_jp_jt_cmd,
     "no debug pim sparse-mode timer joinprune jt",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "JoinPrune timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_JP_JT);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_JP_JT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_jp_et,
     no_debug_pim_timer_jp_et_cmd,
     "no debug pim sparse-mode timer joinprune et",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "Expiry timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_JP_ET);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_JP_ET);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_jp_ppt,
     no_debug_pim_timer_jp_ppt_cmd,
     "no debug pim sparse-mode timer joinprune ppt",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "Prune Pending timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_JP_PPT);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_JP_PPT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_jp_kat,
     no_debug_pim_timer_jp_kat_cmd,
     "no debug pim sparse-mode timer joinprune kat",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "Keep Alive timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_JP_KAT);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_JP_KAT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_jp_ot,
     no_debug_pim_timer_jp_ot_cmd,
     "no debug pim sparse-mode timer joinprune ot",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "JoinPrune Timers",
     "Override timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_JP_OT);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_JP_OT);
  return CMD_SUCCESS;
}


DEFUN (no_debug_pim_timer_assert,
     no_debug_pim_timer_assert_cmd,
     "no debug pim sparse-mode timer assert",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Assert Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_ASSERT_AT);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_ASSERT_AT);
  return CMD_SUCCESS;
}

ALIAS (no_debug_pim_timer_assert,
     no_debug_pim_timer_assert_at_cmd,
     "no debug pim sparse-mode timer assert at",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Assert Timers",
     "Assert timer");

DEFUN (no_debug_pim_timer_reg,
     no_debug_pim_timer_reg_cmd,
     "no debug pim sparse-mode timer register",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Register Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_REG_RST);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_REG_RST);
  return CMD_SUCCESS;
}

ALIAS (no_debug_pim_timer_reg,
     no_debug_pim_timer_reg_rst_cmd,
     "no debug pim sparse-mode timer register rst",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "Register Timers",
     "Register Stop timer");

DEFUN (no_debug_pim_timer_bsr,
     no_debug_pim_timer_bsr_cmd,
     "no debug pim sparse-mode timer bsr",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "BSR Timers")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_BSR_ALL);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_BSR_ALL);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_bsr_bst,
     no_debug_pim_timer_bsr_bst_cmd,
     "no debug pim sparse-mode timer bsr bst",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "BSR Timers",
     "Bootstrap timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_BSR_BST);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_BSR_BST);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_timer_bsr_crp,
     no_debug_pim_timer_bsr_crp_cmd,
     "no debug pim sparse-mode timer bsr crp",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM timers",
     "BSR Timers",
     "Candidate-RP timer")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, TIMER_BSR_CRP);
  TERM_DEBUG_OFF (pim_top, pim, TIMER_BSR_CRP);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_packet,
     debug_pim_packet_cmd,
     "debug pim sparse-mode packet",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM packet")
{
  if (vty->mode == CONFIG_NODE)
    {
      CONF_DEBUG_ON (pim_top, pim, PACKET_IN);
      CONF_DEBUG_ON (pim_top, pim, PACKET_OUT);
    }
  TERM_DEBUG_ON (pim_top, pim, PACKET_IN);
  TERM_DEBUG_ON (pim_top, pim, PACKET_OUT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_packet_in,
     debug_pim_packet_in_cmd,
     "debug pim sparse-mode packet in",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM packet",
     "Incoming PIM packet")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, PACKET_IN);
  TERM_DEBUG_ON (pim_top, pim, PACKET_IN);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_packet_out,
     debug_pim_packet_out_cmd,
     "debug pim sparse-mode packet out",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM packet",
     "Outgoing PIM packet")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, PACKET_OUT);
  TERM_DEBUG_ON (pim_top, pim, PACKET_OUT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_packet,
     no_debug_pim_packet_cmd,
     "no debug pim sparse-mode packet",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM packet")
{
  if (vty->mode == CONFIG_NODE)
    {
      CONF_DEBUG_OFF (pim_top, pim, PACKET_IN);
      CONF_DEBUG_OFF (pim_top, pim, PACKET_OUT);
    }
  TERM_DEBUG_OFF (pim_top, pim, PACKET_IN);
  TERM_DEBUG_OFF (pim_top, pim, PACKET_OUT);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_packet_in,
     no_debug_pim_packet_in_cmd,
     "no debug pim sparse-mode packet in",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM packet",
     "Disable incoming packet debug")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, PACKET_IN);
  TERM_DEBUG_OFF (pim_top, pim, PACKET_IN);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_packet_out,
     no_debug_pim_packet_out_cmd,
     "no debug pim sparse-mode packet out",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM packet",
     "Disable outgoing packet debug")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, PACKET_OUT);
  TERM_DEBUG_OFF (pim_top, pim, PACKET_OUT);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_mib,
     debug_pim_mib_cmd,
     "debug pim sparse-mode mib",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM mib")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, MIB);
  TERM_DEBUG_ON (pim_top, pim, MIB);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_mib,
     no_debug_pim_mib_cmd,
     "no debug pim sparse-mode mib",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM mib")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, MIB);
  TERM_DEBUG_OFF (pim_top, pim, MIB);
  return CMD_SUCCESS;
}

DEFUN (debug_pim_nexthop,
     debug_pim_nexthop_cmd,
     "debug pim sparse-mode nexthop",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM nexthop")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, NEXTHOP);
  TERM_DEBUG_ON (pim_top, pim, NEXTHOP);
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_nexthop,
     no_debug_pim_nexthop_cmd,
     "no debug pim sparse-mode nexthop",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIM nexthop")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, NEXTHOP);
  TERM_DEBUG_OFF (pim_top, pim, NEXTHOP);
  return CMD_SUCCESS;
}


DEFUN (debug_pim_nsm,
     debug_pim_nsm_cmd,
     "debug pim sparse-mode nsm",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "NSM message")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, NSM);
  TERM_DEBUG_ON (pim_top, pim, NSM);
  pim_nsm_debug_set ();
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_nsm,
     no_debug_pim_nsm_cmd,
     "no debug pim sparse-mode nsm",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "NSM message")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, NSM);
  TERM_DEBUG_OFF (pim_top, pim, NSM);
  pim_nsm_debug_unset ();
  return CMD_SUCCESS;
}

DEFUN (debug_pim_all,
     debug_pim_all_cmd,
     "debug pim sparse-mode all",
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "All PIM debugging")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_ON (pim_top, pim, ALL);
  TERM_DEBUG_ON (pim_top, pim, ALL);
  pim_nsm_debug_set ();
  return CMD_SUCCESS;
}

DEFUN (no_debug_pim_all,
     no_debug_pim_all_cmd,
     "no debug pim sparse-mode all",
     NO_STR,
     DEBUG_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "All PIM debugging")
{
  if (vty->mode == CONFIG_NODE)
    CONF_DEBUG_OFF (pim_top, pim, ALL);
  TERM_DEBUG_OFF (pim_top, pim, ALL);
  pim_nsm_debug_unset ();
  return CMD_SUCCESS;
}

ALIAS (no_debug_pim_all,
     undebug_pim_all_cmd,
     "undebug all pim sparse-mode",
     UNDEBUG_STR,
     "Enable all debugging",
     PIM_STR,
     DEFUN_PIMSM_STR);

ALIAS (no_debug_pim_all,
     no_debug_all_pim_cmd,
     "no debug all",
     NO_STR,
     DEBUG_STR,
     "Turn off all debugging");

ALIAS (no_debug_pim_all,
     undebug_all_pim_cmd,
     "undebug all",
     UNDEBUG_STR,
     "Turn off all debugging");



DEFUN (clear_ip_pim_mroute,
     clear_ip_pim_mroute_cmd,
     "clear ip mroute * pim sparse-mode",
     CLEAR_STR,
     IP_STR,
     "Delete multicast route table entries",
     "Delete all multicast routes",
     PIM_STR,
     DEFUN_PIMSM_STR)
{
  struct pal_in4_addr zaddr = {0};

  pim_clear_tib (&zaddr, &zaddr);
  return CMD_SUCCESS;
}

ALIAS (clear_ip_pim_mroute,
     clear_ip_pim_mroute_simple_cmd,
     "clear ip mroute * pim",
     CLEAR_STR,
     IP_STR,
     "Delete multicast route table entries",
     "Delete all multicast routes",
     PIM_STR);

DEFUN (clear_ip_pim_mroute_xg,
     clear_ip_pim_mroute_xg_cmd,
     "clear ip mroute A.B.C.D pim sparse-mode",
     CLEAR_STR,
     IP_STR,
     "Delete multicast route table entries",
     "Group IP Address",
     PIM_STR,
     DEFUN_PIMSM_STR)
{
  struct pal_in4_addr zaddr = {0};
  struct pal_in4_addr gaddr;

  DEFUN_GET_IPV4_ADDRESS ("Group IP Address", gaddr, argv[0]);

  if (! IN_MULTICAST (pal_ntoh32 (gaddr.s_addr)))
    vty_out (vty, "%% Invalid group address\n");

  pim_clear_tib (&zaddr, &gaddr);
  return CMD_SUCCESS;
}

ALIAS (clear_ip_pim_mroute_xg,
     clear_ip_pim_mroute_xg_simple_cmd,
     "clear ip mroute A.B.C.D pim",
     CLEAR_STR,
     IP_STR,
     "Delete multicast route table entries",
     "Group IP Address",
     PIM_STR);
     

DEFUN (clear_ip_pim_mroute_sg,
     clear_ip_pim_mroute_sg_cmd,
     "clear ip mroute A.B.C.D A.B.C.D pim sparse-mode",
     CLEAR_STR,
     IP_STR,
     "Delete multicast route table entries",
     "Group IP Address",
     "Source IP Address",
     PIM_STR,
     DEFUN_PIMSM_STR)
{
  struct pal_in4_addr saddr;
  struct pal_in4_addr gaddr;

  DEFUN_GET_IPV4_ADDRESS ("Source IP address", saddr, argv[1]);
  DEFUN_GET_IPV4_ADDRESS ("Group IP address", gaddr, argv[0]);

  if (! IN_MULTICAST (pal_ntoh32 (gaddr.s_addr)))
    vty_out (vty, "%% Invalid group address\n");
  if (IN_MULTICAST (pal_ntoh32 (saddr.s_addr)))
    vty_out (vty, "%% Invalid source address\n");

  pim_clear_tib (&saddr, &gaddr);
  return CMD_SUCCESS;
}

ALIAS (clear_ip_pim_mroute_sg,
     clear_ip_pim_mroute_sg_simple_cmd,
     "clear ip mroute A.B.C.D A.B.C.D pim",
     CLEAR_STR,
     IP_STR,
     "Delete multicast route table entries",
     "Group IP Address",
     "Source IP Address",
     PIM_STR);

DEFUN (clear_ip_pim_bsr_rpset,
     clear_ip_pim_bsr_rpset_cmd,
     "clear ip pim sparse-mode bsr rp-set *",
     CLEAR_STR,
     IP_STR,
     PIM_STR,
     DEFUN_PIMSM_STR,
     "PIMv2 Bootstrap Router",
     "PIMv2 Bootstrap Router RP set",
     "Clear all RP sets")
{
  pim_clear_bsr_rpset ();
  return CMD_SUCCESS;
}

ALIAS (clear_ip_pim_bsr_rpset,
     clear_ip_pim_bsr_rpset_simple_cmd,
     "clear ip pim bsr rp-set *",
     CLEAR_STR,
     IP_STR,
     PIM_STR,
     "PIMv2 Bootstrap Router",
     "PIMv2 Bootstrap Router RP set",
     "Clear all RP sets");


int
pim_debug_config_write (struct vty *vty)
{
  if (IS_CONF_DEBUG_PIM_ALL(pim_top))
    vty_out (vty, "debug pim sparse-mode all\n");
  else
    {
      if (IS_CONF_DEBUG_PIM_EVENT(pim_top))
        vty_out (vty, "debug pim sparse-mode events\n");

      if (IS_CONF_DEBUG_PIM_NSM(pim_top))
        vty_out (vty, "debug pim sparse-mode nsm\n");

      if (IS_CONF_DEBUG_PIM_MFC(pim_top))
        vty_out (vty, "debug pim sparse-mode mfc\n");

      if (IS_CONF_DEBUG_PIM_PACKET_IN(pim_top)
          && IS_CONF_DEBUG_PIM_PACKET_OUT(pim_top))
        vty_out (vty, "debug pim sparse-mode packet\n");
      else
        {
          if (IS_CONF_DEBUG_PIM_PACKET_IN(pim_top))
            vty_out (vty, "debug pim sparse-mode packet in\n");
          if (IS_CONF_DEBUG_PIM_PACKET_OUT(pim_top))
            vty_out (vty, "debug pim sparse-mode packet out\n");
        }

      if (IS_CONF_DEBUG_PIM_STATE(pim_top))
        vty_out (vty, "debug pim sparse-mode state\n");

      if (IS_CONF_DEBUG_PIM_TIMER_ALL(pim_top))
        vty_out (vty, "debug pim sparse-mode timer\n"); 
      else
        {
          if (IS_CONF_DEBUG_PIM_TIMER_HELLO_ALL(pim_top))
            vty_out (vty, "debug pim sparse-mode timer hello\n");
          else
            {
              if (IS_CONF_DEBUG_PIM_TIMER_HELLO_HT(pim_top))
                vty_out (vty, "debug pim sparse-mode timer hello ht\n");
              if (IS_CONF_DEBUG_PIM_TIMER_HELLO_NLT(pim_top))
                vty_out (vty, "debug pim sparse-mode timer hello nlt\n");
              if (IS_CONF_DEBUG_PIM_TIMER_HELLO_THT(pim_top))
                vty_out (vty, "debug pim sparse-mode timer hello tht\n");
            }

          if (IS_CONF_DEBUG_PIM_TIMER_JP_ALL(pim_top))
            vty_out (vty, "debug pim sparse-mode timer joinprune\n");
          else
            {
              if (IS_CONF_DEBUG_PIM_TIMER_JP_JT(pim_top))
                vty_out (vty, "debug pim sparse-mode timer joinprune jt\n");
              if (IS_CONF_DEBUG_PIM_TIMER_JP_ET(pim_top))
                vty_out (vty, "debug pim sparse-mode timer joinprune et\n");
              if (IS_CONF_DEBUG_PIM_TIMER_JP_PPT(pim_top))
                vty_out (vty, "debug pim sparse-mode timer joinprune ppt\n");
              if (IS_CONF_DEBUG_PIM_TIMER_JP_KAT(pim_top))
                vty_out (vty, "debug pim sparse-mode timer joinprune kat\n");
              if (IS_CONF_DEBUG_PIM_TIMER_JP_OT(pim_top))
                vty_out (vty, "debug pim sparse-mode timer joinprune ot\n");
            }

          if (IS_CONF_DEBUG_PIM_TIMER_ASSERT_AT(pim_top))
            vty_out (vty, "debug pim sparse-mode timer assert at\n");
          if (IS_CONF_DEBUG_PIM_TIMER_REG_RST(pim_top))
            vty_out (vty, "debug pim sparse-mode timer register rst\n");

          if (IS_CONF_DEBUG_PIM_TIMER_BSR_ALL(pim_top))
            vty_out (vty, "debug pim sparse-mode timer bsr\n");
          else
            {
              if (IS_CONF_DEBUG_PIM_TIMER_BSR_BST(pim_top))
                vty_out (vty, "debug pim sparse-mode timer bsr bst\n");
              if (IS_CONF_DEBUG_PIM_TIMER_BSR_CRP(pim_top))
                vty_out (vty, "debug pim sparse-mode timer bsr crp\n");
            }
        }
      if (IS_CONF_DEBUG_PIM_NEXTHOP(pim_top))
        vty_out (vty, "debug pim sparse-mode nexthop\n");
      if (IS_CONF_DEBUG_PIM_MIB(pim_top))
        vty_out (vty, "debug pim sparse-mode mib\n");
    }

  vty_out (vty, "!\n");

  return 0;
}

/* Write C-RP configuration */
int
pim_crp_config_write (struct vty *vty, struct pim_crp *crp)
{
  vty_out (vty, "ip pim rp-candidate %s", crp->crp_ifname);
  if (CHECK_FLAG (crp->config, PIM_CRP_CONFIG_PRIORITY))
    vty_out (vty, " priority %u", crp->crp_priority);
  if (CHECK_FLAG (crp->config, PIM_CRP_CONFIG_INTERVAL))
    vty_out (vty, " interval %u", crp->crp_advinterval);
  if (CHECK_FLAG (crp->config, PIM_CRP_CONFIG_ACL))
    vty_out (vty, " group-list %s", crp->grp_list);
  vty_out (vty, "\n");

  return 0;
}

int
pim_config_write (struct vty *vty)
{
  int write = 0;
  struct pim_bsr *bsr;
  struct pim_st_rp_conf *conf;
  struct pim_crp *crp;

  if (! pim_top)
    return 0;

  for (conf = pim_top->st_rp_head; conf != NULL; conf = conf->next)
    {
      if (conf->grp_acl)
        vty_out (vty, "ip pim rp-address %r %s\n", &conf->rp_addr, 
            conf->grp_acl);
      else
        vty_out (vty, "ip pim rp-address %r\n", &conf->rp_addr);
    }

  if (pim_top->autorp_exp != DEF_AUTORP_EXPIRE)
    vty_out (vty, "ip pim auto-rp expire-interval %d\n", pim_top->autorp_exp);

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_JP_TIMER))
    vty_out (vty, "ip pim jp-timer %d\n", pim_top->jp_timer);

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_IGNORE_RP_SET_PRIORITY))
    vty_out (vty, "ip pim ignore-rp-set-priority\n");

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_SPT_SWITCH))
    {
      if (pim_top->spt_switch)
        vty_out (vty, "ip pim spt-threshold group-list %s\n",
            pim_top->spt_switch);
      else
        vty_out (vty, "ip pim spt-threshold\n");
    }

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_REG_SRC_ADDR))
    vty_out (vty, "ip pim register-source %r\n", &pim_top->reg_src_addr);
  else if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_REG_SRC_INTF))
    vty_out (vty, "ip pim register-source %s\n", pim_top->reg_src_intf);

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_REG_RATE_LIMIT))
    vty_out (vty, "ip pim register-rate-limit %d\n", pim_top->reg_rate_limit);

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_REG_RP_REACH))
    vty_out (vty, "ip pim register-rp-reachability\n");

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_REG_SUPP))
    vty_out (vty, "ip pim register-suppression %d\n", pim_top->reg_suppression);

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_RP_REG_KAT))
    vty_out (vty, "ip pim rp-register-kat %d\n", pim_top->rp_reg_kat);

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_RP_REG_FILTER))
    vty_out (vty, "ip pim accept-register list %s\n", pim_top->rp_reg_filter);

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_CISCO_REG_CKSUM))
    {
      if (pim_top->cisco_reg_filter)
        vty_out (vty, "ip pim cisco-register-checksum group-list %s\n", 
            pim_top->cisco_reg_filter);
      else
        vty_out (vty, "ip pim cisco-register-checksum\n");
    }

  bsr = pim_bsr_lookup (pim_top);
  if (CHECK_FLAG (bsr->configs, PIM_BSR_CONFIG_CANDIDATE))
    {
      vty_out (vty, "ip pim bsr-candidate %s", bsr->ifname);
      if (CHECK_FLAG (bsr->configs, PIM_BSR_CONFIG_HASH_MASKLEN))
	vty_out (vty, " %d", bsr->my_hash_masklen);
      if (CHECK_FLAG (bsr->configs, PIM_BSR_CONFIG_PRIORITY))
	vty_out (vty, " %d", bsr->my_priority);
      vty_out (vty, "\n");
    }
  /* Write C-RP configuration */
  for (crp = bsr->crp_head; crp; crp = crp->next)
    {
      pim_crp_config_write (vty, crp);
    }

  if (CHECK_FLAG (bsr->configs, PIM_BSR_CONFIG_CRP_CISCO_PRIFIX))
    vty_out (vty, "ip pim crp-cisco-prefix\n");

  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_SSM_DEFAULT))
    vty_out (vty, "ip pim ssm default\n");
  else if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_SSM_ACL))
    vty_out (vty, "ip pim ssm range %s\n", pim_top->ssm_grp_rng);

  /* Debug configuration */
  if (pim_top->conf_debug_pim)
  	pim_debug_config_write(vty);

  return write;
}

void
pim_vty_init (void)
{
  /* Install PIM and PIM interface node.  */
  install_node (&pim_node, pim_config_write);
  install_node (&interface_node, pim_if_config_write);

  /* Install default VTY commands. */
  install_default (PIM_NODE);
  install_default (INTERFACE_NODE);

  /* PIM global commands.  */
  install_element (CONFIG_NODE, &ip_pim_rp_address_default_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_address_acl_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_rp_address_default_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_rp_address_acl_cmd);
  install_element (CONFIG_NODE, &ip_pim_jp_timer_cmd);
  install_element (CONFIG_NODE, &ip_pim_autorp_refresh_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_jp_timer_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_jp_timer_val_cmd);
  install_element (CONFIG_NODE, &ip_pim_ignore_rp_set_priority_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_ignore_rp_set_priority_cmd);

  install_element (CONFIG_NODE, &ip_pim_spt_switch_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_spt_switch_cmd);
  install_element (CONFIG_NODE, &ip_pim_spt_switch_grouplist_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_spt_switch_grouplist_cmd);
  install_element (CONFIG_NODE, &ip_pim_reg_src_addr_cmd);
  install_element (CONFIG_NODE, &ip_pim_reg_src_intf_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_reg_src_cmd);
  install_element (CONFIG_NODE, &ip_pim_reg_rate_limit_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_reg_rate_limit_cmd);
  install_element (CONFIG_NODE, &ip_pim_reg_rp_reach_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_reg_rp_reach_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_reg_kat_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_rp_reg_kat_cmd);
  install_element (CONFIG_NODE, &ip_pim_reg_supp_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_reg_supp_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_reg_filter_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_rp_reg_filter_cmd);
  install_element (CONFIG_NODE, &ip_pim_reg_cksum_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_reg_cksum_cmd);
  install_element (CONFIG_NODE, &ip_pim_reg_cksum_filter_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_reg_cksum_filter_cmd);
  install_element (CONFIG_NODE, &ip_pim_ssm_default_cmd);
  install_element (CONFIG_NODE, &ip_pim_ssm_acl_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_ssm_cmd);

  /* PIM interface commands. */
  install_element (INTERFACE_NODE, &ip_pim_mode_cmd);
  install_element (INTERFACE_NODE, &no_ip_pim_sparse_mode_cmd);
  
  install_element (INTERFACE_NODE, &ip_pim_mode_passive_cmd);
  install_element (INTERFACE_NODE, &no_ip_pim_mode_passive_cmd);

  install_element (INTERFACE_NODE, &ip_pim_sm_nbr_flt_cmd);
  install_element (INTERFACE_NODE, &no_ip_pim_sm_nbr_flt_cmd);

  install_element (INTERFACE_NODE, &ip_pim_dr_priority_cmd);
  install_element (INTERFACE_NODE, &no_ip_pim_dr_priority_cmd);
  install_element (INTERFACE_NODE, &no_ip_pim_dr_priority_val_cmd);

  install_element (INTERFACE_NODE, &ip_pim_hello_interval_cmd);
  install_element (INTERFACE_NODE, &no_ip_pim_hello_interval_cmd);

  install_element (INTERFACE_NODE, &ip_pim_hello_holdtime_cmd);
  install_element (INTERFACE_NODE, &no_ip_pim_hello_holdtime_cmd);

  install_element (INTERFACE_NODE, &ip_pim_exclude_genid_cmd);
  install_element (INTERFACE_NODE, &no_ip_pim_exclude_genid_cmd);

  /* PIM show commands. */
  install_element (ENABLE_NODE, &show_ip_pim_mroute_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_mroute_simple_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_neighbor_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_neighbor_simple_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_neighbor_detail_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_neighbor_simple_detail_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_interface_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_interface_simple_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_interface_detail_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_interface_simple_detail_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_nexthop_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_nexthop_simple_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_localmembers_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_localmembers_simple_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_localmembers_ifname_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_localmembers_ifname_simple_cmd);

  /* PIM BSR commands.  */
  install_element (CONFIG_NODE, &ip_pim_bsr_candidate_cmd);
  install_element (CONFIG_NODE, &ip_pim_bsr_candidate_hash_cmd);
  install_element (CONFIG_NODE, &ip_pim_bsr_candidate_hash_priority_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_bsr_candidate_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_bsr_candidate_arg_cmd);

  install_element (ENABLE_NODE, &show_ip_pim_bsr_router_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_bsr_router_simple_cmd);

  /* PIM RP commands.  */
  install_element (CONFIG_NODE, &ip_pim_rp_cand1_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_cand2_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_cand3_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_cand4_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_cand5_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_cand6_cmd);
  install_element (CONFIG_NODE, &ip_pim_rp_cand7_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_rp_cand_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_rp_cand_arg_cmd);
  install_element (CONFIG_NODE, &ip_pim_crp_cisco_prefix_cmd);
  install_element (CONFIG_NODE, &no_ip_pim_crp_cisco_prefix_cmd);

  install_element (ENABLE_NODE, &show_ip_pim_rp_mapping_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_rp_mapping_simple_cmd);

  install_element (ENABLE_NODE, &show_ip_pim_rp_hash_cmd);
  install_element (ENABLE_NODE, &show_ip_pim_rp_hash_simple_cmd);

  /* DEBUG commands.  */
  install_element (ENABLE_NODE, &show_debugging_pim_cmd);
  install_element (ENABLE_NODE, &debug_pim_state_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_state_cmd);
  install_element (ENABLE_NODE, &debug_pim_events_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_events_cmd);
  install_element (ENABLE_NODE, &debug_pim_mfc_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_mfc_cmd);
  install_element (ENABLE_NODE, &debug_pim_packet_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_packet_cmd);
  install_element (ENABLE_NODE, &debug_pim_packet_in_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_packet_in_cmd);
  install_element (ENABLE_NODE, &debug_pim_packet_out_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_packet_out_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_hello_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_hello_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_hello_ht_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_hello_ht_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_hello_nlt_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_hello_nlt_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_hello_tht_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_hello_tht_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_jp_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_jp_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_jp_jt_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_jp_jt_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_jp_et_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_jp_et_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_jp_ppt_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_jp_ppt_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_jp_kat_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_jp_kat_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_jp_ot_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_jp_ot_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_assert_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_assert_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_assert_at_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_assert_at_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_reg_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_reg_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_reg_rst_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_reg_rst_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_bsr_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_bsr_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_bsr_bst_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_bsr_bst_cmd);
  install_element (ENABLE_NODE, &debug_pim_timer_bsr_crp_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_timer_bsr_crp_cmd);
  install_element (ENABLE_NODE, &debug_pim_mib_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_mib_cmd);
  install_element (ENABLE_NODE, &debug_pim_nsm_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_nsm_cmd);
  install_element (ENABLE_NODE, &debug_pim_nexthop_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_nexthop_cmd);
  install_element (ENABLE_NODE, &debug_pim_all_cmd);
  install_element (ENABLE_NODE, &no_debug_pim_all_cmd);
  install_element (ENABLE_NODE, &undebug_pim_all_cmd);
  install_element (ENABLE_NODE, &no_debug_all_pim_cmd);
  install_element (ENABLE_NODE, &undebug_all_pim_cmd);

  install_element (CONFIG_NODE, &debug_pim_state_cmd);  
  install_element (CONFIG_NODE, &no_debug_pim_state_cmd);
  install_element (CONFIG_NODE, &debug_pim_events_cmd);  
  install_element (CONFIG_NODE, &no_debug_pim_events_cmd);
  install_element (CONFIG_NODE, &debug_pim_mfc_cmd);  
  install_element (CONFIG_NODE, &no_debug_pim_mfc_cmd);
  install_element (CONFIG_NODE, &debug_pim_packet_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_packet_cmd);
  install_element (CONFIG_NODE, &debug_pim_packet_in_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_packet_in_cmd);
  install_element (CONFIG_NODE, &debug_pim_packet_out_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_packet_out_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_hello_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_hello_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_hello_ht_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_hello_ht_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_hello_nlt_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_hello_nlt_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_hello_tht_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_hello_tht_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_jp_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_jp_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_jp_jt_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_jp_jt_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_jp_et_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_jp_et_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_jp_ppt_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_jp_ppt_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_jp_kat_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_jp_kat_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_jp_ot_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_jp_ot_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_assert_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_assert_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_assert_at_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_assert_at_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_reg_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_reg_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_reg_rst_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_reg_rst_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_bsr_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_bsr_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_bsr_bst_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_bsr_bst_cmd);
  install_element (CONFIG_NODE, &debug_pim_timer_bsr_crp_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_timer_bsr_crp_cmd);
  install_element (CONFIG_NODE, &debug_pim_mib_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_mib_cmd);
  install_element (CONFIG_NODE, &debug_pim_nsm_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_nsm_cmd);
  install_element (CONFIG_NODE, &debug_pim_nexthop_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_nexthop_cmd);
  install_element (CONFIG_NODE, &debug_pim_all_cmd);
  install_element (CONFIG_NODE, &no_debug_pim_all_cmd);
  install_element (CONFIG_NODE, &undebug_pim_all_cmd);
  install_element (CONFIG_NODE, &no_debug_all_pim_cmd);
  install_element (CONFIG_NODE, &undebug_all_pim_cmd);

  install_element (ENABLE_NODE, &clear_ip_pim_mroute_cmd);
  install_element (ENABLE_NODE, &clear_ip_pim_mroute_simple_cmd);
  install_element (ENABLE_NODE, &clear_ip_pim_mroute_xg_cmd);
  install_element (ENABLE_NODE, &clear_ip_pim_mroute_xg_simple_cmd);
  install_element (ENABLE_NODE, &clear_ip_pim_mroute_sg_cmd);
  install_element (ENABLE_NODE, &clear_ip_pim_mroute_sg_simple_cmd);
  install_element (ENABLE_NODE, &clear_ip_pim_bsr_rpset_cmd);
  install_element (ENABLE_NODE, &clear_ip_pim_bsr_rpset_simple_cmd);
}
