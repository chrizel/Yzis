SUBDIRS = . $(YZIS_SUBDIRS)

INCLUDES = $(YZIS_INCLUDES) $(LUA_INCLUDES)
noinst_LTLIBRARIES = libyzis.la

libyzis_la_SOURCES = mark.cpp action.cpp cursor.cpp selection.cpp line.cpp buffer.cpp view.cpp session.cpp commands.cpp ex_executor.cpp debug.cpp internal_options.cpp option.cpp optionpool.cpp registers.cpp undo.cpp translator.h syntaxdocument.cpp syntaxhighlight.cpp attribute.cpp ex_lua.cpp printer.cpp swapfile.cpp schema.cpp viewcursor.cpp macros.cpp
#libyzis_la_LIBADD = $(LUA_LIBS) -lmagic -lps

libyzismimedir = $(prefix)/share/yzis
libyzismime_DATA = magic.mime

DISTCLEANFILES = translator.h

re : clean all

METASOURCES = AUTO
