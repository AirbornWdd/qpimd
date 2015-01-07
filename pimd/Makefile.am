## Process this file with automake to produce Makefile.in.

INCLUDES = @INCLUDES@ -I.. -I$(top_srcdir) -I$(top_srcdir)/lib @SNMP_INCLUDES@
DEFS = @DEFS@ -DSYSCONFDIR=\"$(sysconfdir)/\"
INSTALL_SDATA=@INSTALL@ -m 600

AM_CFLAGS = $(PICFLAGS)
AM_LDFLAGS = $(PILDFLAGS)

noinst_LIBRARIES = libpim.a
sbin_PROGRAMS = pimd

libpim_a_SOURCES = \
	pimd.c

noinst_HEADERS = \
	pimd.h 

pimd_SOURCES = \
	pim_main.c $(libpim_a_SOURCES)

pimd_LDADD = ../lib/libkroute.la @LIBCAP@

examplesdir = $(exampledir)
dist_examples_DATA = pimd.conf.sample

EXTRA_DIST = PIM-MIB.txt

