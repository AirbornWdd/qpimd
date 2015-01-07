/*
 * Kroute debug related function
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
 * along with GNU Kroute; see the file COPYING.  If not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
 * Boston, MA 02111-1307, USA.  
 */

#ifndef _KROUTE_DEBUG_H
#define _KROUTE_DEBUG_H

/* Debug flags. */
#define KROUTE_DEBUG_EVENT   0x01

#define KROUTE_DEBUG_PACKET  0x01
#define KROUTE_DEBUG_SEND    0x20
#define KROUTE_DEBUG_RECV    0x40
#define KROUTE_DEBUG_DETAIL  0x80

#define KROUTE_DEBUG_KERNEL  0x01

#define KROUTE_DEBUG_RIB     0x01
#define KROUTE_DEBUG_RIB_Q   0x02

/* Debug related macro. */
#define IS_KROUTE_DEBUG_EVENT  (kroute_debug_event & KROUTE_DEBUG_EVENT)

#define IS_KROUTE_DEBUG_PACKET (kroute_debug_packet & KROUTE_DEBUG_PACKET)
#define IS_KROUTE_DEBUG_SEND   (kroute_debug_packet & KROUTE_DEBUG_SEND)
#define IS_KROUTE_DEBUG_RECV   (kroute_debug_packet & KROUTE_DEBUG_RECV)
#define IS_KROUTE_DEBUG_DETAIL (kroute_debug_packet & KROUTE_DEBUG_DETAIL)

#define IS_KROUTE_DEBUG_KERNEL (kroute_debug_kernel & KROUTE_DEBUG_KERNEL)

#define IS_KROUTE_DEBUG_RIB  (kroute_debug_rib & KROUTE_DEBUG_RIB)
#define IS_KROUTE_DEBUG_RIB_Q  (kroute_debug_rib & KROUTE_DEBUG_RIB_Q)

extern unsigned long kroute_debug_event;
extern unsigned long kroute_debug_packet;
extern unsigned long kroute_debug_kernel;
extern unsigned long kroute_debug_rib;

extern void kroute_debug_init (void);

#endif /* _KROUTE_DEBUG_H */
