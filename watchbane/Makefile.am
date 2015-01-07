## Process this file with Automake to create Makefile.in

INCLUDES = @INCLUDES@ -I$(top_srcdir) -I$(top_srcdir)/lib
DEFS = @DEFS@ -DSTATEDIR=\"$(localstatedir)/\"

AM_CFLAGS = $(PICFLAGS)
AM_LDFLAGS = $(PILDFLAGS)

sbin_PROGRAMS = watchbane

watchbane_SOURCES = watchbane.c
watchbane_LDADD = ../lib/libkroute.la @LIBCAP@
