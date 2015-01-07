## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib @SNMP_INCLUDES@
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\" -DMULTIPATH_NUM=@MULTIPATH_NUM@
INSTALL_SDATA=@INSTALL@ -m 600

LIB_IPV6 = @LIB_IPV6@
LIBCAP = @LIBCAP@

ipforward = @IPFORWARD@
if_method = @IF_METHOD@
if_proc = @IF_PROC@
rt_method = @RT_METHOD@
rtread_method = @RTREAD_METHOD@
kernel_method = @KERNEL_METHOD@
other_method = @OTHER_METHOD@
ioctl_method = @IOCTL_METHOD@

otherobj = $(ioctl_method) $(ipforward) $(if_method) $(if_proc) \
	$(rt_method) $(rtread_method) $(kernel_method) $(other_method)

AM_CFLAGS = $(PICFLAGS)
AM_LDFLAGS = $(PILDFLAGS)

sbin_PROGRAMS = kroute

noinst_PROGRAMS = testkroute

kroute_SOURCES = \
	zserv.c main.c interface.c connected.c kroute_rib.c kroute_routemap.c \
	redistribute.c debug.c rtadv.c kroute_snmp.c kroute_vty.c \
	irdp_main.c irdp_interface.c irdp_packet.c router-id.c

testkroute_SOURCES = test_main.c kroute_rib.c interface.c connected.c debug.c \
	kroute_vty.c \
	kernel_null.c  redistribute_null.c ioctl_null.c misc_null.c

noinst_HEADERS = \
	connected.h ioctl.h rib.h rt.h zserv.h redistribute.h debug.h rtadv.h \
	interface.h ipforward.h irdp.h router-id.h kernel_socket.h

kroute_LDADD = $(otherobj) $(LIBCAP) $(LIB_IPV6) ../lib/libkroute.la

testkroute_LDADD = $(LIBCAP) $(LIB_IPV6) ../lib/libkroute.la

kroute_DEPENDENCIES = $(otherobj)

EXTRA_DIST = if_ioctl.c if_ioctl_solaris.c if_netlink.c if_proc.c \
        if_sysctl.c ipforward_aix.c ipforward_ews.c ipforward_proc.c \
	ipforward_solaris.c ipforward_sysctl.c rt_ioctl.c rt_netlink.c \
	rt_socket.c rtread_netlink.c rtread_proc.c rtread_sysctl.c \
	rtread_getmsg.c kernel_socket.c kernel_netlink.c mtu_kvm.c \
	ioctl.c ioctl_solaris.c \
	GNOME-SMI GNOME-PRODUCT-KROUTE-MIB

#client : client_main.o ../lib/libkroute.la
#	$(CC) -g -o client client_main.o ../liblkroute.la $(LIBS) $(LIB_IPV6)

baneconfdir = $(sysconfdir)

examplesdir = $(exampledir)
dist_examples_DATA = kroute.conf.sample
