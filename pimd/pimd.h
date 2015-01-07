/* Copyright (C) 2002-2003 IP Infusion, Inc. All Rights Reserved. */

#ifndef _ZEBOS_PIMD_H
#define _ZEBOS_PIMD_H

/* VTY PORT */
#define PIM_VTY_PORT                  2609

/* Common groups.  */
#define INADDR_MULTICAST_ADDRESS     0xe0000000U /* 224.0.0.0 */
#define INADDR_MULTICAST_PREFIXLEN   4

#ifndef INADDR_ALLHOSTS_GROUP
#define INADDR_ALLHOSTS_GROUP        0xe0000001U /* 224.0.0.1 */
#endif 
#ifndef INADDR_ALLRTRS_GROUP
#define INADDR_ALLRTRS_GROUP         0xe0000002U /* 224.0.0.2 */
#endif
#ifndef INADDR_PIM_ALLRTRS_GROUP
#define INADDR_PIM_ALLRTRS_GROUP     0xe000000DU /* 224.0.0.13 */
#endif
#ifndef INADDR_MAX_LOCAL_GROUP
#define INADDR_MAX_LOCAL_GROUP       0xe00000ffU /* 224.0.0.255 */
#endif /* INADDR_MAX_LOCAL_ADDRESS */
#ifndef INADDR_PIM_AUTORP_DISCOVERY_ADDRESS
#define INADDR_PIM_AUTORP_DISCOVERY_ADDRESS     (0xE0000128UL) /* 224.0.1.40 */
#endif
#ifndef INADDR_PIM_AUTORP_ANNOUNCE_ADDRESS
#define INADDR_PIM_AUTORP_ANNOUNCE_ADDRESS     (0xE0000127UL) /* 224.0.1.39 */
#endif

#define PIM_DEF_SSM_GRP_RNG          0xe8000000U /* 232.0.0.0 */
#define PIM_DEF_SSM_GRP_RNG_MASK     0xff000000U /* 255.0.0.0 or /8 */
#define PIM_DEF_SSM_GRP_RNG_MASKLEN  8 /* 255.0.0.0 or /8 */

/* Loopback Address */
#define LOOPBACK_ADDR                0x7f000000U /* 127.0.0.1 */

struct pim_top *pim_create(struct lib_globals *);
void pim_init (struct lib_globals *);
void pim_interface_init();
void pim_reset();
void pim_terminate();
void pim_stop (void);
void vifs_init();
void rpf_init();
void pim_rp_init();
void pim_assert_init();
void pim_nexthop_init (struct pim_top *);
void pim_nexthop_shutdown (struct pim_top *);
u_int32_t	pim_masklen_to_mask_ipv4 (u_int8_t);
enum filter_type pim_spt_al_apply (struct pim_top *, struct pal_in4_addr *);
void pim_check_switch_to_spt (struct pal_in4_addr *, struct pal_in4_addr *);
enum filter_type pim_rp_reg_filter_apply (struct pim_top *, 
    struct pal_in4_addr *);
enum filter_type pim_cisco_reg_filter_apply (struct pim_top *, 
    struct pal_in4_addr *);
void pim_mrt_start ();
void pim_mrt_stop ();

bool_t pim_is_acl_ssm_grp_rng (struct pim_top *, struct pal_in4_addr *); 
void pim_ssm_acl_filter_add (struct access_list *acl, struct filter_list *flt);
void pim_ssm_acl_filter_del (struct access_list *acl, struct filter_list *flt);
void pim_apply_ssm_semantics (struct pim_top *pim_top, struct prefix *p);
void pim_undo_ssm_semantics (struct pim_top *pim_top, struct prefix *p);

/* VTY APIs */
void pim_spt_threshold_set (struct pim_top *, char *);
void pim_spt_threshold_unset (struct pim_top *, char *);
void pim_reg_src_addr_set (struct pal_in4_addr *);
void pim_reg_src_intf_set (char *);
void pim_reg_src_unset (void);
void pim_reg_rate_limit_set (u_int16_t);
void pim_reg_rate_limit_unset (void);
void pim_reg_rp_reach_set (void);
void pim_reg_rp_reach_unset (void);
void pim_rp_reg_kat_set (u_int16_t);
void pim_rp_reg_kat_unset (void);
void pim_reg_supp_set (u_int16_t);
void pim_reg_supp_unset (void);
void pim_rp_reg_filter_set (struct pim_top *, char *);
void pim_rp_reg_filter_unset (struct pim_top *);
void pim_cisco_reg_cksum_set (struct pim_top *);
void pim_cisco_reg_cksum_unset (struct pim_top *);
void pim_cisco_reg_cksum_filter_set (struct pim_top *, char *);
void pim_cisco_reg_cksum_filter_unset (struct pim_top *, char *);
void pim_set_ignore_rpset_priority (struct pim_top *);
void pim_unset_ignore_rpset_priority (struct pim_top *);
void pim_configure_ssm_default (struct pim_top *);
void pim_configure_ssm_acl (struct pim_top *, char *);
void pim_unconfigure_ssm (struct pim_top *);

extern struct pim_top *pim_top;
extern struct prefix pim_def_ssm;

#endif /* _ZEBOS_PIMD_H */
