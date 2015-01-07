/* Inteface addition message from kroute. */
static int
pim_interface_add (int command, struct zclient *zclient, kroute_size_t length)
{
  struct interface *ifp;

  ifp = kroute_interface_add_read (zclient->ibuf);

  if (ifp && IS_DEBUG_PIM_EVENT(pim_top))
    PIMLOG_INFO ("Kroute: interface add %s index %d flags %llx metric %d mtu %d",
               ifp->name, ifp->ifindex, (unsigned long long)ifp->flags,
               ifp->metric, ifp->mtu);
  
  return 0;
}

static int
pim_interface_delete (int command, struct zclient *zclient,
                       kroute_size_t length)
{
  struct interface *ifp;
  struct stream *s;
  struct route_node *rn;

  s = zclient->ibuf;
  /* kroute_interface_state_read() updates interface structure in iflist */
  ifp = kroute_interface_state_read (s);

  if (ifp && IS_DEBUG_PIM_EVENT(pim_top))
    PIMLOG_INFO ("Kroute: interface del %s index %d flags %llx metric %d mtu %d",
               ifp->name, ifp->ifindex, (unsigned long long)ifp->flags,
               ifp->metric, ifp->mtu);
  
  if (ifp != NULL)
    if_delete (ifp);

  return 0;
}

/* Inteface link up message processing */
int
pim_interface_up (int command, struct zclient *zclient, kroute_size_t length)
{
  struct pim_vif *vif;
  struct interface *ifp;

  /* kroute_interface_state_read () updates interface structure in
     iflist. */
  ifp = kroute_interface_state_read (zclient->ibuf);

  if (ifp == NULL)
    return 0;

  if (IS_DEBUG_PIM_EVENT(pim_top))
    PIMLOG_INFO ("Kroute: interface %s index %d flags %#llx metric %d mtu %d is up",
	       ifp->name, ifp->ifindex, (unsigned long long) ifp->flags,
	       ifp->metric, ifp->mtu);
  
  vif = ifp->info;
  if (vif)
    pim_vif_update (vif);
  if ((pim_top->bsr) && (pim_top->bsr->ifname) &&
      (!pal_strncmp (pim_top->bsr->ifname, ifp->name, IFNAMSIZ)))
    pim_bsr_candidate_update (pim_top) ;
  if (vif)
    pim_rp_candidate_update_vif (vif) ;

  return 0;
}

/* Inteface link down message processing */
int
pim_interface_down (int command, struct zclient *zclient, kroute_size_t length)
{
  struct pim_vif *vif;
  struct interface *ifp;

  /* kroute_interface_state_read () updates interface structure in
     iflist. */
  ifp = kroute_interface_state_read (zclient->ibuf);

  if (ifp == NULL)
    return 0;

  if (IS_DEBUG_PIM_EVENT(pim_top))
    PIMLOG_INFO ("Kroute: interface %s index %d flags %#llx metric %d mtu %d is up",
	       ifp->name, ifp->ifindex, (unsigned long long) ifp->flags,
	       ifp->metric, ifp->mtu);
  
  vif = ifp->info;
  if (vif)
    pim_vif_update (vif);
  if (vif)
    pim_rp_candidate_update_vif (vif) ;
  if ((pim_top->bsr) && (pim_top->bsr->ifname) &&
      (!pal_strncmp (pim_top->bsr->ifname, ifp->name, IFNAMSIZ)))
    pim_bsr_candidate_update (pim_top) ;

  return 0;
}

void
pim_kroute_init ()
{
  /* Allocate kroute structure. */
  zclient = zclient_new ();
  zclient_init (zclient, KROUTE_ROUTE_PIM);
  zclient->interface_add = pim_interface_add;
  zclient->interface_delete = pim_interface_delete;
  zclient->interface_up = pim_interface_up;
  zclient->interface_down = pim_interface_down;
  zclient->interface_address_add = pim_interface_address_add;
  zclient->interface_address_delete = pim_interface_address_delete;
  zclient->ipv4_route_add = pim_kroute_read_ipv4;
  zclient->ipv4_route_delete = pim_kroute_read_ipv4;

  access_list_add_hook (pim_filter_update);
  access_list_delete_hook (pim_filter_update);
  prefix_list_add_hook (pim_prefix_list_update);
  prefix_list_delete_hook (pim_prefix_list_update);
}

