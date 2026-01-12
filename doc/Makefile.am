## Process this file with automake to produce Makefile.in

DOCINPUTS = $(top_srcdir)/include/oac.h \
            $(top_srcdir)/include/oac_multistream.h \
            $(top_srcdir)/include/oac_defines.h \
            $(top_srcdir)/include/oac_types.h \
            $(top_srcdir)/include/oac_custom.h \
            $(top_srcdir)/doc/header.html \
            $(top_srcdir)/doc/footer.html \
            $(top_srcdir)/doc/customdoxygen.css

EXTRA_DIST = customdoxygen.css Doxyfile.in footer.html header.html \
             trivial_example.c


if HAVE_DOXYGEN

all-local: doxygen-build.stamp

doxygen-build.stamp: Doxyfile $(DOCINPUTS)
	doxygen
	touch $@

install-data-local:
	$(INSTALL) -d $(DESTDIR)$(docdir)/html/search
	for f in `find html -type f \! -name "installdox"`; do	\
		$(INSTALL_DATA) $$f $(DESTDIR)$(docdir)/$$f;	\
	done

	$(INSTALL) -d $(DESTDIR)$(mandir)/man3
	cd man && find man3 -type f -name oac_*.3 \
		  -exec $(INSTALL_DATA) \{} $(DESTDIR)$(mandir)/man3 \;

clean-local:
	$(RM) -r html
	$(RM) -r latex
	$(RM) -r man
	$(RM) doxygen-build.stamp
	$(RM) doxygen_sqlite3.db

uninstall-local:
	$(RM) -r $(DESTDIR)$(docdir)/html
	$(RM) $(DESTDIR)$(mandir)/man3/oac_*.3 $(DESTDIR)$(mandir)/man3/oac.h.3

endif
