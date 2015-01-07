/* OSPFv2 SNMP support
 * Copyright (C) 2000 IP Infusion Inc.
 *
 * Written by Kunihiro Ishiguro <kunihiro@kroute.org>
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

#ifndef _KROUTE_OSPF_SNMP_H
#define _KROUTE_OSPF_SNMP_H

extern void ospf_snmp_if_update (struct interface *);
extern void ospf_snmp_if_delete (struct interface *);

extern void ospf_snmp_vl_add (struct ospf_vl_data *);
extern void ospf_snmp_vl_delete (struct ospf_vl_data *);

extern void ospfTrapIfStateChange (struct ospf_interface *);
extern void ospfTrapVirtIfStateChange (struct ospf_interface *);
extern void ospfTrapNbrStateChange (struct ospf_neighbor *);
extern void ospfTrapVirtNbrStateChange (struct ospf_neighbor *);

#endif /* _KROUTE_OSPF_SNMP_H */
