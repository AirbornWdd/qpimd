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
#include "pimd/pim_mrt.h"
#include "pimd/pim_packet.h"
#include "pimd/pim_vty.h"
#include "pimd/pim_rp.h"
#include "pimd/pim_vif.h"
#include "pimd/pim_igmp.h"
#include "pimd/pim_route.h"
#include "pimd/pim_reg.h"
#include "pimd/pim_macrochange.h"
#include "pimd/pim_kroute.h"

/* PIM instance structure.  */
struct pim_top *pim_top;

/* Common muticast address for convenience.  */
struct pal_in4_addr allhosts;
struct pal_in4_addr allrouters;
struct pal_in4_addr allpimrouters;
struct pal_in4_addr allpimautorprouters;
struct pal_in4_addr addrany;

/* Loopback addr */
struct pal_in4_addr loopback_addr;

/* Default SSM group range prefix */
struct prefix pim_def_ssm;

void pim_spt_al_change (struct pim_top*, struct pim_mrt *, 
    struct access_list *, struct filter_list *);
void pim_rp_reg_filter_change (struct pim_top*, struct pim_mrt *, 
    struct access_list *, struct filter_list *);

/* Function to convert IPv4 mask length to mask.  */
u_int32_t
pim_masklen_to_mask_ipv4 (u_int8_t masklen)
{
  if (masklen == 0)
    return 0;
  else 
    return ((u_int32_t)((s_int32_t)(0x80000000) >> (masklen - 1)));
}

/* Initialize PIM related functions such as NSM, interface, debug
   option and CLI.  This command does not create PIM instance.  So to
   crate PIM instance, please call pim_create().  */
void
pim_init (void)
{
  /* Seed the random number generator with current time */
  srand ((u_int32_t)time (NULL));
  
  /* Store common groups.  */
  allhosts.s_addr = htonl (INADDR_ALLHOSTS_GROUP);
  allrouters.s_addr = htonl (INADDR_ALLRTRS_GROUP);
  allpimrouters.s_addr = htonl (INADDR_PIM_ALLRTRS_GROUP);
  allpimautorprouters.s_addr = htonl (INADDR_PIM_AUTORP_DISCOVERY_ADDRESS);
  addrany.s_addr = 0;

  /* Loopback address */
  loopback_addr.s_addr = htonl (LOOPBACK_ADDR);

  /* Initialize default SSM prefix */
  mem_set (&pim_def_ssm, 0, sizeof (struct prefix));
  pim_def_ssm.family = AF_INET;
  pim_def_ssm.prefixlen = PIM_DEF_SSM_GRP_RNG_MASKLEN;
  pim_def_ssm.u.prefix4.s_addr = htonl (PIM_DEF_SSM_GRP_RNG);
  
  /* All initialization.  */
  pim_vty_init ();
  pim_kroute_init ();
  pim_vif_init ();

  return;
}
