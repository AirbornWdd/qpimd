/* Copyright (C) 2002-2003 IP Infusion, Inc. All Rights Reserved. */

#ifndef _PAL_MCAST_H
#define _PAL_MCAST_H

#define _LINUX_IN_H
#include <linux/mroute.h>
#include <linux/udp.h>

#undef pal_vifctl
#define pal_vifctl vifctl

#undef pal_mfcctl
#define pal_mfcctl mfcctl

#undef pal_mfcctl_v2
#define pal_mfcctl_v2 mfcctl_v2

#undef pal_sioc_sg_req
#define pal_sioc_sg_req sioc_sg_req

#undef pal_sioc_vif_req
#define pal_sioc_vif_req sioc_vif_req

#undef pal_igmpmsg
#define pal_igmpmsg igmpmsg

#undef pal_udphdr
#define pal_udphdr udphdr

#undef  PAL_MAXVIFS
#define PAL_MAXVIFS               MAXVIFS

#undef  PAL_MCAST_NOCACHE
#define PAL_MCAST_NOCACHE         IGMPMSG_NOCACHE

#undef  PAL_MCAST_WRONGVIF
#define PAL_MCAST_WRONGVIF        IGMPMSG_WRONGVIF

#undef  PAL_MCAST_WHOLEPKT
#define PAL_MCAST_WHOLEPKT        IGMPMSG_WHOLEPKT

#include "pal_mcast.def"

#endif /* PAL_MCAST_H */
