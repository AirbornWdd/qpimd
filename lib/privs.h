/* 
 * Kroute privileges header.
 *
 * Copyright (C) 2003 Paul Jakma.
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

#ifndef _KROUTE_PRIVS_H
#define _KROUTE_PRIVS_H

/* list of kroute capabilities */
typedef enum 
{
  ZCAP_SETID,
  ZCAP_BIND,
  ZCAP_NET_ADMIN,
  ZCAP_SYS_ADMIN,
  ZCAP_NET_RAW,
  ZCAP_CHROOT,
  ZCAP_NICE,
  ZCAP_PTRACE,
  ZCAP_DAC_OVERRIDE,
  ZCAP_READ_SEARCH,
  ZCAP_FOWNER,
  ZCAP_MAX
} kroute_capabilities_t;

typedef enum
{
  ZPRIVS_LOWERED,
  ZPRIVS_RAISED,
  ZPRIVS_UNKNOWN,
} kroute_privs_current_t;

typedef enum
{
  ZPRIVS_RAISE,
  ZPRIVS_LOWER,
} kroute_privs_ops_t;

struct kroute_privs_t
{
  kroute_capabilities_t *caps_p;       /* caps required for operation */
  kroute_capabilities_t *caps_i;       /* caps to allow inheritance of */
  int cap_num_p;                      /* number of caps in arrays */
  int cap_num_i;                    
  const char *user;                   /* user and group to run as */
  const char *group;
  const char *vty_group;              /* group to chown vty socket to */
  /* methods */
  int 
    (*change) (kroute_privs_ops_t);    /* change privileges, 0 on success */
  kroute_privs_current_t 
    (*current_state) (void);          /* current privilege state */
};

struct zprivs_ids_t
{
  /* -1 is undefined */
  uid_t uid_priv;                     /* privileged uid */
  uid_t uid_normal;                   /* normal uid */
  gid_t gid_priv;                     /* privileged uid */
  gid_t gid_normal;                   /* normal uid */
  gid_t gid_vty;                      /* vty gid */
};

  /* initialise kroute privileges */
extern void zprivs_init (struct kroute_privs_t *zprivs);
  /* drop all and terminate privileges */ 
extern void zprivs_terminate (struct kroute_privs_t *);
  /* query for runtime uid's and gid's, eg vty needs this */
extern void zprivs_get_ids(struct zprivs_ids_t *);

#endif /* _KROUTE_PRIVS_H */
