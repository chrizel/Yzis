# This file is part of the Yzis libraries
#  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Library General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public License
#  along with this library; see the file COPYING.LIB.  If not, write to
#  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
#  Boston, MA 02111-1307, USA.

TEMPLATE = app
INCLUDEPATH += . .. ../libyzis
CONFIG    += console warn_on debug
CONFIG    += rtti # necessary for dynamic cast
#LIB  += ../libyzis.lib

win32-msvc {
	DESTDIR = ./
}

HEADERS += qyzview.h \
		   qyzbuffer.h \
		   qyzsession.h

SOURCES += main.cpp \
			qyzview.cpp \
			qyzbuffer.cpp \
			qyzsession.cpp

####################### copy from libyzis

# you must set LUAINCLUDE to the directory containing lua headers
# and LUALIB to the directory containing lua .so or lua .lib files

INCLUDEPATH += $$(LUAINCLUDE)

win32-msvc {
	DESTDIR = ./
	LIBS += $$(LUALIB)/Lua.lib $$(LUALIB)/LuaLib.lib
}

# Input
HEADERS += ../libyzis/action.h \
           ../libyzis/attribute.h \
           ../libyzis/buffer.h \
           ../libyzis/commands.h \
           ../libyzis/cursor.h \
           ../libyzis/debug.h \
           ../libyzis/events.h \
           ../libyzis/ex_lua.h \
           ../libyzis/excommands.h \
           ../libyzis/internal_options.h \
           ../libyzis/line.h \
           ../libyzis/linesearch.h \
           ../libyzis/mapping.h \
           ../libyzis/mark.h \
           ../libyzis/option.h \
           ../libyzis/plugin.h \
           ../libyzis/printer.h \
           ../libyzis/qtprinter.h \
           ../libyzis/registers.h \
           ../libyzis/schema.h \
           ../libyzis/search.h \
           ../libyzis/selection.h \
           ../libyzis/session.h \
           ../libyzis/swapfile.h \
           ../libyzis/syntaxdocument.h \
           ../libyzis/syntaxhighlight.h \
           ../libyzis/undo.h \
           ../libyzis/view.h \
           ../libyzis/viewcursor.h \
           ../libyzis/portability.h \
           ../libyzis/yzis.h

SOURCES += ../libyzis/action.cpp \
           ../libyzis/attribute.cpp \
           ../libyzis/buffer.cpp \
           ../libyzis/commands.cpp \
           ../libyzis/cursor.cpp \
           ../libyzis/debug.cpp \
           ../libyzis/events.cpp \
           ../libyzis/ex_lua.cpp \
           ../libyzis/excommands.cpp \
           ../libyzis/internal_options.cpp \
           ../libyzis/line.cpp \
           ../libyzis/linesearch.cpp \
           ../libyzis/mapping.cpp \
           ../libyzis/mark.cpp \
           ../libyzis/option.cpp \
           ../libyzis/printer.cpp \
           ../libyzis/qtprinter.cpp \
           ../libyzis/registers.cpp \
           ../libyzis/schema.cpp \
           ../libyzis/search.cpp \
           ../libyzis/selection.cpp \
           ../libyzis/session.cpp \
           ../libyzis/swapfile.cpp \
           ../libyzis/syntaxdocument.cpp \
           ../libyzis/syntaxhighlight.cpp \
           ../libyzis/undo.cpp \
           ../libyzis/view.cpp \
           ../libyzis/viewcursor.cpp
