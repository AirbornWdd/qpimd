/*
 * RIPng debug output routines
 * Copyright (C) 1998, 1999 Kunihiro Ishiguro
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

#ifndef _KROUTE_RIPNG_DEBUG_H
#define _KROUTE_RIPNG_DEBUG_H

/* Debug flags. */
#define RIPNG_DEBUG_EVENT   0x01

#define RIPNG_DEBUG_PACKET  0x01
#define RIPNG_DEBUG_SEND    0x20
#define RIPNG_DEBUG_RECV    0x40

#define RIPNG_DEBUG_KROUTE   0x01

/* Debug related macro. */
#define IS_RIPNG_DEBUG_EVENT  (ripng_debug_event & RIPNG_DEBUG_EVENT)

#define IS_RIPNG_DEBUG_PACKET (ripng_debug_packet & RIPNG_DEBUG_PACKET)
#define IS_RIPNG_DEBUG_SEND   (ripng_debug_packet & RIPNG_DEBUG_SEND)
#define IS_RIPNG_DEBUG_RECV   (ripng_debug_packet & RIPNG_DEBUG_RECV)

#define IS_RIPNG_DEBUG_KROUTE  (ripng_debug_kroute & RIPNG_DEBUG_KROUTE)

extern unsigned long ripng_debug_event;
extern unsigned long ripng_debug_packet;
extern unsigned long ripng_debug_kroute;

extern void ripng_debug_init (void);
extern void ripng_debug_reset (void);

#endif /* _KROUTE_RIPNG_DEBUG_H */
