
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

#	for i in $(top_srcdir)/doc/yzis-doc/en/*.xml; do \
#		xmlto pdf -o $(top_builddir)/doc/yzis-doc/en/pdf $$i; \
#	done
ADD_CUSTOM_COMMAND( 
	OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/faq.pdf
	COMMAND   xmlto
	ARGS      pdf -o ${CMAKE_CURRENT_SOURCE_DIR}/yzis-doc.xml
	DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/yzis-doc.xml
	COMMENT   "xmlto"
)


ADD_CUSTOM_COMMAND( 
	OUTPUT    ${CMAKE_CURRENT_BINARY_DIR}/faq.pdf
	COMMAND   docbook2pdf
	ARGS      ${CMAKE_CURRENT_SOURCE_DIR}/faq.docbook
	DEPENDS   ${CMAKE_CURRENT_SOURCE_DIR}/faq.docbook
	COMMENT   "docbook2pdf"
)

add_custom_target(docbook ALL echo
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/faq.pdf
)

#faq.docbook introduction.docbook reference.docbook commandref.docbook incompabilities.docbook userguide.docbook


#original Makefile.am contents follow:

#docs:
#	if test ! -d $(top_builddir)/doc/yzis-doc/en/html/; then \
#		$(mkinstalldirs) $(top_builddir)/doc/yzis-doc/en/html; \
#	fi; \
#	if test ! -d $(top_builddir)/doc/yzis-doc/en/html/pics/; then \
#		$(mkinstalldirs) $(top_builddir)/doc/yzis-doc/en/html/pics; \
#	fi; \
#	if test ! -d $(top_builddir)/doc/yzis-doc/en/pdf/pics/; then \
#		$(mkinstalldirs) $(top_builddir)/doc/yzis-doc/en/pdf/pics; \
#	fi; \
#	for i in $(top_srcdir)/doc/yzis-doc/en/*.xml; do \
#		xmlto html -o $(top_builddir)/doc/yzis-doc/en/html $$i; \
#	done ;\
#	cp $(top_srcdir)/doc/yzis-doc/en/pics/*.* $(top_builddir)/doc/yzis-doc/en/html/pics/ ; \
#	cp $(top_srcdir)/doc/yzis-doc/en/pics/*.* $(top_builddir)/doc/yzis-doc/en/pdf/pics/ ; \
#	if test ! -d $(top_builddir)/doc/yzis-doc/en/pdf/; then \
#		$(mkinstalldirs) $(top_builddir)/doc/yzis-doc/en/pdf; \
#	fi; \
#	for i in $(top_srcdir)/doc/yzis-doc/en/*.xml; do \
#		xmlto pdf -o $(top_builddir)/doc/yzis-doc/en/pdf $$i; \
#	done
#
