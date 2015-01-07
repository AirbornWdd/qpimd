/* 
 * Bane Signal handling header.
 *
 * Copyright (C) 2004 Paul Jakma.
 *
 * This file is part of Bane.
 *
 * Bane is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * Bane is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bane; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef _BANE_SIGNAL_H
#define _BANE_SIGNAL_H

#include <thread.h>

#define BANE_SIGNAL_TIMER_INTERVAL 2L
#define Q_SIGC(sig) (sizeof(sig)/sizeof(sig[0]))

struct bane_signal_t
{
  int signal;                     /* signal number    */
  void (*handler) (void);         /* handler to call  */

  volatile sig_atomic_t caught;   /* private member   */
};

/* initialise sigevent system
 * takes:
 * - pointer to valid struct thread_master
 * - number of elements in passed in signals array
 * - array of bane_signal_t's describing signals to handle
 *   and handlers to use for each signal
 */
extern void signal_init (struct thread_master *m, int sigc, 
                         struct bane_signal_t *signals);

/* check whether there are signals to handle, process any found */
extern int bane_sigevent_process (void);

#endif /* _BANE_SIGNAL_H */
