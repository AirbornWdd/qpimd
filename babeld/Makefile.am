## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib @SNMP_INCLUDES@
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600

AM_CFLAGS = $(PICFLAGS)
AM_LDFLAGS = $(PILDFLAGS)

noinst_LIBRARIES = libbabel.a
sbin_PROGRAMS = babeld

libbabel_a_SOURCES = \
	babel_kroute.c net.c kernel.c util.c source.c neighbour.c	\
	route.c xroute.c message.c resend.c babel_interface.c babeld.c	\
	babel_filter.c

noinst_HEADERS = \
	babel_kroute.h net.h kernel.h util.h source.h neighbour.h	\
	route.h xroute.h message.h resend.h babel_interface.h babeld.h	\
	babel_filter.h babel_main.h

babeld_SOURCES = \
	babel_main.c $(libbabel_a_SOURCES)

babeld_LDADD = ../lib/libkroute.la @LIBCAP@

examplesdir = $(exampledir)
dist_examples_DATA = babeld.conf.sample
