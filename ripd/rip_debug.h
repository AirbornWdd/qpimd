/* RIP debug routines
 * Copyright (C) 1999 Kunihiro Ishiguro <kunihiro@kroute.org>
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

#ifndef _KROUTE_RIP_DEBUG_H
#define _KROUTE_RIP_DEBUG_H

/* RIP debug event flags. */
#define RIP_DEBUG_EVENT   0x01

/* RIP debug packet flags. */
#define RIP_DEBUG_PACKET  0x01
#define RIP_DEBUG_SEND    0x20
#define RIP_DEBUG_RECV    0x40
#define RIP_DEBUG_DETAIL  0x80

/* RIP debug kroute flags. */
#define RIP_DEBUG_KROUTE   0x01

/* Debug related macro. */
#define IS_RIP_DEBUG_EVENT  (rip_debug_event & RIP_DEBUG_EVENT)

#define IS_RIP_DEBUG_PACKET (rip_debug_packet & RIP_DEBUG_PACKET)
#define IS_RIP_DEBUG_SEND   (rip_debug_packet & RIP_DEBUG_SEND)
#define IS_RIP_DEBUG_RECV   (rip_debug_packet & RIP_DEBUG_RECV)

#define IS_RIP_DEBUG_KROUTE  (rip_debug_kroute & RIP_DEBUG_KROUTE)

extern unsigned long rip_debug_event;
extern unsigned long rip_debug_packet;
extern unsigned long rip_debug_kroute;

extern void rip_debug_init (void);
extern void rip_debug_reset (void);

#endif /* _KROUTE_RIP_DEBUG_H */
