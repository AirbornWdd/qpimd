/*
 * Redistribution Handler
 * Copyright (C) 1999 Kunihiro Ishiguro
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

#ifndef _KROUTE_REDISTRIBUTE_H
#define _KROUTE_REDISTRIBUTE_H

#include "table.h"
#include "zserv.h"

extern void kroute_redistribute_add (int, struct zserv *, int);
extern void kroute_redistribute_delete (int, struct zserv *, int);

extern void kroute_redistribute_default_add (int, struct zserv *, int);
extern void kroute_redistribute_default_delete (int, struct zserv *, int);

extern void redistribute_add (struct prefix *, struct rib *);
extern void redistribute_delete (struct prefix *, struct rib *);

extern void kroute_interface_up_update (struct interface *);
extern void kroute_interface_down_update (struct interface *);

extern void kroute_interface_add_update (struct interface *);
extern void kroute_interface_delete_update (struct interface *);

extern void kroute_interface_address_add_update (struct interface *,
					 	struct connected *);
extern void kroute_interface_address_delete_update (struct interface *,
						   struct connected *c);

extern int kroute_check_addr (struct prefix *);

#endif /* _KROUTE_REDISTRIBUTE_H */

