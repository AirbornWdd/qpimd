/* Copyright (C) 2002-2003 IP Infusion, Inc. All Rights Reserved. */

#ifndef _PLAT_INCL_H
#define _PLAT_INCL_H

#define _BSD_SOURCE		1
#define _ISOC99_SOURCE		1
#define _POSIX_SOURCE		1
#define _POSIX_C_SOURCE		199506L
#define _XOPEN_SOURCE		600
#define _XOPEN_SOURCE_EXTENDED	1

#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdarg.h>
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif /* HAVE_STROPTS_H */
#include <sys/fcntl.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif /* HAVE_SYS_SELECT_H */

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#ifndef __KERNEL__
#include <sys/param.h>
#endif /* __KERNEL__ */
#include <sys/wait.h>

#ifdef HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif /* HAVE_SYS_SYSCTL_H */
#include <sys/ioctl.h>
#ifdef HAVE_SYS_CONF_H
#include <sys/conf.h>
#endif /* HAVE_SYS_CONF_H */
#ifdef HAVE_SYS_KSYM_H
#include <sys/ksym.h>
#endif /* HAVE_SYS_KSYM_H */

#ifdef HAVE_ASM_TYPES_H
#include <asm/types.h>
#endif /* HAVE_ASM_TYPES_H */

/* network include group */

#ifndef __KERNEL__
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/uio.h>

#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif /* HAVE_SYS_SOCKIO_H */

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif /* HAVE_NETINET_IN_H */
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/igmp.h>
#ifdef HAVE_NETINET_IP6_H
#include <netinet/ip6.h>
#endif /* HAVE_NETINET_IP6_H */
#include <netinet/tcp.h>

#ifdef HAVE_NET_NETOPT_H
#include <net/netopt.h>
#endif /* HAVE_NET_NETOPT_H */

#include <net/if.h>

#endif /* __KERNEL__ */

#ifdef HAVE_NETPACKET_PACKET_H
#include <netpacket/packet.h>
#endif /* HAVE_NETPACKET_PACKET_H */

#ifdef HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif /* HAVE_NET_ETHERNET_H */

#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif /* HAVE_NET_IF_DL_H */

#ifdef HAVE_NET_IF_VAR_H
#include <net/if_var.h>
#endif /* HAVE_NET_IF_VAR_H */

#include <net/route.h>

#ifndef __KERNEL__
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif /* HAVE_NETDB_H */

#include <arpa/inet.h>
#include <arpa/telnet.h>

#ifdef HAVE_INET_ND_H
#include <inet/nd.h>
#endif /* HAVE_INET_ND_H */

#ifdef HAVE_NETINET_IN_VAR_H
#include <netinet/in_var.h>
#endif /* HAVE_NETINET_IN_VAR_H */

#ifdef HAVE_NETINET_IN6_VAR_H
#include <netinet/in6_var.h>
#endif /* HAVE_NETINET_IN6_VAR_H */

#ifdef HAVE_NETINET_ICMP6_H
#include <netinet/icmp6.h>
#endif /* HAVE_NETINET_ICMP6_H */

#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif /* HAVE_LIBUTIL_H */

#ifdef HAVE_SYS_MD5_H
#include <sys/md5.h>

#define HAVE_MD5
#define md5_init_ctx(CTX)               MD5Init((CTX))
#define md5_process_bytes(PTR,LEN,CTX)  MD5Update((CTX),(PTR),(LEN))
#define md5_finish_ctx(CTX,DIGEST)      MD5Final((DIGEST),(CTX))
#endif /* HAVE_SYS_MD5_H */

#ifdef HAVE_OPENSSL_MD5_H
#include <openssl/md5.h>

#define HAVE_MD5
#define md5_init_ctx(CTX)               MD5_Init((CTX))
#define md5_process_bytes(PTR,LEN,CTX)  MD5_Update((CTX),(PTR),(LEN))
#define md5_finish_ctx(CTX,DIGEST)      MD5_Final((DIGEST),(CTX))
#endif /* HAVE_OPENSSL_MD5_H */

#ifdef HAVE_OPENSSL_HMAC_H
#include <openssl/hmac.h>

#define HAVE_HMAC
#define hmac_init_ctx(CTXPTR,KEY,LEN,EVP)    HMAC_Init((CTXPTR), (KEY), (LEN), (EVP))
#define hmac_process_bytes(CTXPTR,DPTR,LEN)  HMAC_Update((CTXPTR),(DPTR),(LEN))
#define hmac_finish_ctx(CTXPTR,CPTR,LENPTR)  HMAC_Final((CTXPTR),(CPTR),(LENPTR))
#endif /* HAVE_OPENSSL_MD5_H */

#endif /* __KERNEL__ */

/* machine dependent includes */
#ifdef HAVE_LINUX_VERSION_H
#include <linux/version.h>
#endif /* HAVE_LINUX_VERSION_H */

#include <linux/sockios.h>

#ifdef HAVE_NETLINK
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#else
#define RT_TABLE_MAIN           0
#define RT_TABLE_LOCAL  	0
#endif /* HAVE_LINUX_RTNETLINK_H */

/* Define this if the kernel supports interface label.  */
#define HAVE_LABEL

/* Define this to install NSM RIB metric to the kernel FIB.  */
#undef HAVE_UPDATE_FIB_METRIC

/* Define this if kernel requires to have address on unnumbered interfaces.  */
#define HAVE_UPDATE_ADDRESS_ON_UNNUMBERED

#ifdef HAVE_IPNET
#define IP_SO_X_RTAB 	0x1020
#endif /* HAVE_IPNET */

#endif /* _PLAT_INCL_H */
