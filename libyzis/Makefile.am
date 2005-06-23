SUBDIRS = . libqtyzis

INCLUDES = $(YZIS_INCLUDES) $(LUA_INCLUDES) $(LIBMAGIC_INCLUDES) $(LIBGETTEXT_INCLUDES) -I$(top_srcdir)/libyzis -I$(top_builddir)/libyzis
noinst_LTLIBRARIES = libyzis.la

libyzis_la_SOURCES = session.cpp mode.cpp mode_command.cpp mode_search.cpp mode_visual.cpp mode_ex.cpp mode_insert.cpp view.cpp search.cpp mark.cpp action.cpp cursor.cpp selection.cpp line.cpp buffer.cpp linesearch.cpp debug.cpp internal_options.cpp option.cpp registers.cpp undo.cpp ex_lua.cpp qtprinter.cpp swapfile.cpp schema.cpp viewcursor.cpp printer.cpp mapping.cpp events.cpp syntaxdocument.cpp syntaxhighlight.cpp attribute.cpp yzisinfo.cpp yzisinfostartpositionrecord.cpp yzisinfojumplistrecord.cpp readtags.c tags_interface.cpp tags_stack.cpp mode_complete.cpp
libyzis_la_PCH = AUTO

libyzismimedir = $(prefix)/share/yzis
libyzismime_DATA = magic.mime

DISTCLEANFILES = translator.h

re : clean all

METASOURCES = AUTO
