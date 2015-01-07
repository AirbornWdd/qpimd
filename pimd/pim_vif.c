/* Allocate new VIF structure.  */
struct pim_vif *
pim_vif_new (struct interface *ifp)
{
  struct pim_vif *vif;
  float tmp_holdtime;


  if (! pim_top)
    return NULL;

  /* Alloc a VIF.  */
  vif = XCALLOC (MTYPE_PIM_VIF, sizeof (struct pim_vif));
  if (vif == NULL)
    return NULL;

  /* Pre-initialize a link list to be used for secondary address list
   * on this vif
   */
  vif->hello.addr_list = list_new ();
  if (vif->hello.addr_list == NULL)
    {
      XFREE (MTYPE_PIM_VIF, vif);
      return NULL;
    }

  /* Initialize local information P-Trie */
  vif->local_info = ptree_init (PIM_VIF_LOCAL_MAX_KEYLEN);
  if (vif->local_info == NULL)
    {
      list_delete (vif->hello.addr_list);
      XFREE (MTYPE_PIM_VIF, vif);
      return NULL;
    }

  /* Set index -1.  */
  vif->index = -1;
  vif->neighbor = route_table_init ();
  if (vif->neighbor == NULL)
    {
      ptree_finish (vif->local_info);
      list_delete (vif->hello.addr_list);
      XFREE (MTYPE_PIM_VIF, vif);
      return NULL;
    }
  vif->nbr_flt = NULL;


  /* Set PIM default values.  */
  vif->hello_period = PIM_HELLO_PERIOD_DEFAULT;
  vif->trig_hello_delay = PIM_TRIGGERED_HELLO_DELAY; 

  /* Join/Prune timer.  */
  if (CHECK_FLAG (pim_top->configs, PIM_CONFIG_JP_TIMER))
    vif->t_periodic = pim_top->jp_timer;
  else
    vif->t_periodic = PIM_T_PERIODIC_DEFAULT;

  /* Join/Prune header holdtime */
  tmp_holdtime = 3.5 * vif->t_periodic;
  vif->jp_holdtime = (u_int16_t) tmp_holdtime;
  
  vif->t_override_int = PIM_T_OVERRIDE_DEFAULT;
  vif->t_propagation_delay = PIM_T_LAN_DELAY_DEFAULT;
  vif->t_assert_override_int = PIM_T_ASSERT_OVERRIDE_DEFAULT;
  vif->t_assert_time = PIM_T_ASSERT_TIME_DEFAULT;

  /* Set the default TTL */
  vif->ttl = PIM_VIF_DEFAULT_TTL;

  /* Default Hello parameter.  */
  SET_FLAG (vif->hello.flags, PIM_HELLO_FLAG_DR_PRIORITY);
  vif->hello.priority = 1;
  SET_FLAG (vif->hello.flags, PIM_HELLO_FLAG_HOLDTIME);
  vif->hello.holdtime = vif->hello_period * 3.5;
  UNSET_FLAG (vif->hello.flags, PIM_HELLO_FLAG_TRACKING_SUPPORT);
  vif->hello.lan_delay = PIM_T_LAN_DELAY_DEFAULT;
  vif->hello.override_interval = PIM_T_OVERRIDE_DEFAULT;
  vif->genid_yes = PAL_TRUE;
  /* Initialize the comparision function for sorting and deletion function */
  vif->hello.addr_list->cmp = pim_hello_addr_list_cmp;
  vif->hello.addr_list->del = pim_hello_addr_list_del_func;
  SET_FLAG (vif->hello.flags, PIM_HELLO_FLAG_ADDR_LIST);


  /* Bind vif to interface when it is physical interface.  When this
     is a kernel register interface, ifp is NULL.  */
  if (ifp)
    {
      ifp->info = vif;
      vif->ifp = ifp;
    }
  return vif;
}

/* Free VIF structure.  */
void
pim_vif_free (struct pim_vif *vif)
{
  struct interface *ifp;

  ifp = vif->ifp;
  route_table_finish (vif->neighbor);

  /* Free the neigbor filter access list name */
  if (vif->nbr_flt)
    XFREE (MTYPE_TMP, vif->nbr_flt);

  /* Free the list of secondary addresses in hello */
  if (vif->hello.addr_list)
    list_delete (vif->hello.addr_list);

  /* Free the local information */
  if (vif->local_info)
    ptree_finish (vif->local_info);

  XFREE (MTYPE_PIM_VIF, vif);
  if (ifp)
    ifp->info = NULL;
}

int
pim_interface_new_hook (struct interface *ifp)
{
  pim_vif_new (ifp);
  return 0;
}

int
pim_interface_delete_hook (struct interface *ifp)
{
  struct pim_vif *vif;

  if ((vif = ifp->info) == NULL)
    return -1;
  pim_vif_free (vif);

  return 0;
}

void
pim_vif_init ()
{
  /* Default initial size of interface vector. */
  if_init();
  if_add_hook (IF_NEW_HOOK, pim_interface_new_hook);
  if_add_hook (IF_DELETE_HOOK, pim_interface_delete_hook);
}

