COMPILE_FIRST = libyzis
SUBDIRS = $(TOPSUBDIRS)

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

EXTRA_DIST = admin 

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs

apidox:
	$(mkinstalldirs) doc/api; \
	if test ! -x doc/common; then \
	$(LN_S) $(kde_libs_htmldir)/en/common doc/common ; \
	fi; \
	cp $(top_srcdir)/admin/Doxyfile.global yzis.doxyfile; \
	echo "PROJECT_NAME        = Yzis" >> yzis.doxyfile; \
	echo "PROJECT_NUMBER      = \"Version M1\"" >> yzis.doxyfile; \
	echo "INPUT               = $(srcdir)" >> yzis.doxyfile; \
	echo "OUTPUT_DIRECTORY    = doc/api" >> yzis.doxyfile; \
	echo "HTML_OUTPUT         = html" >> yzis.doxyfile; \
	echo "GENERATE_HTML       = YES" >> yzis.doxyfile ; \
	echo "GENERATE_MAN        = NO" >> yzis.doxyfile ; \
	echo "GENERATE_LATEX      = NO" >> yzis.doxyfile ; \
	echo "HTML_HEADER         = doc/common/header.html" >> yzis.doxyfile ; \
	echo "HTML_FOOTER         = " >> yzis.doxyfile ; \
	echo "HTML_STYLESHEET     = doc/common/doxygen.css" >> yzis.doxyfile ; \
	echo "FILE_PATTERNS       = *.h" >> yzis.doxyfile ; \
	echo "PREDEFINED          = DO_NOT_DOCUMENT" >> yzis.doxyfile ; \
	echo "EXTRACT_ALL         = YES" >> yzis.doxyfile ; \
	doxygen yzis.doxyfile

tests/cppunit/Makefile:
	cd tests/cppunit && qmake cppunit.pro -o Makefile
cppunit: tests/cppunit/Makefile
	cd tests/cppunit && make
tests/Makefile:
	cd tests && qmake  tests.pro -o Makefile
tests/runTests: cppunit tests/Makefile
	cd tests && make
tests: tests/runTests
	cd tests && ./runTests

