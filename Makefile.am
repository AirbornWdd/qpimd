## Process this file with automake to produce Makefile.in.

SUBDIRS = lib @KROUTE@ @BGPD@ @RIPD@ @RIPNGD@ @PIMD@ @OSPFD@ @OSPF6D@ @BABELD@ \
         @ISISD@ @WATCHBANE@ @VTYSH@ @OSPFCLIENT@ @DOC@ m4 @pkgsrcdir@ \
         redhat @SOLARIS@

DIST_SUBDIRS = lib kroute bgpd ripd ripngd pimd ospfd ospf6d babeld \
	  isisd watchbane vtysh ospfclient doc m4 pkgsrc redhat tests \
	  solaris

EXTRA_DIST = aclocal.m4 SERVICES TODO REPORTING-BUGS INSTALL.bane.txt \
	update-autotools \
	vtysh/Makefile.in vtysh/Makefile.am \
	tools/mrlg.cgi tools/rrcheck.pl tools/rrlookup.pl tools/zc.pl \
	tools/kroute.el tools/multiple-bgpd.sh

if HAVE_LATEX

HACKING.pdf: HACKING.tex
	$(LATEXMK) -pdf $<

clean-local:
	-$(LATEXMK) -C HACKING.tex

endif

ACLOCAL_AMFLAGS = -I m4
