docs:
	if test ! -d $(top_builddir)/doc/yzis-doc/en/html/; then \
		$(mkinstalldirs) $(top_builddir)/doc/yzis-doc/en/html; \
	fi; \
	if test ! -d $(top_builddir)/doc/yzis-doc/en/html/pics/; then \
		$(mkinstalldirs) $(top_builddir)/doc/yzis-doc/en/html/pics; \
	fi; \
	if test ! -d $(top_builddir)/doc/yzis-doc/en/pdf/pics/; then \
		$(mkinstalldirs) $(top_builddir)/doc/yzis-doc/en/pdf/pics; \
	fi; \
	for i in $(top_srcdir)/doc/yzis-doc/en/*.xml; do \
		docbook2html -o $(top_builddir)/doc/yzis-doc/en/html $$i; \
	done ;\
	cp $(top_srcdir)/doc/yzis-doc/en/pics/*.* $(top_builddir)/doc/yzis-doc/en/html/pics/ ; \
	cp $(top_srcdir)/doc/yzis-doc/en/pics/*.* $(top_builddir)/doc/yzis-doc/en/pdf/pics/ ; \
	if test ! -d $(top_builddir)/doc/yzis-doc/en/pdf/; then \
		$(mkinstalldirs) $(top_builddir)/doc/yzis-doc/en/pdf; \
	fi; \
	for i in $(top_srcdir)/doc/yzis-doc/en/*.xml; do \
		docbook2pdf -o $(top_builddir)/doc/yzis-doc/en/pdf $$i; \
	done

