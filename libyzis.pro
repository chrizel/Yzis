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


# you must set LUAINCLUDE to the directory containing lua headers
# and LUALIB to the directory containing lua .so or lua .lib files

TEMPLATE = lib
INCLUDEPATH += . ..  $$(LUAINCLUDE)
CONFIG    += console warn_on debug dll
CONFIG    += rtti # necessary for dynamic cast

win32-msvc {
	DESTDIR = ./
	LIBS += $$(LUALIB)/Lua.lib $$(LUALIB)/LuaLib.lib
}

# Input
HEADERS += action.h \
           attribute.h \
           buffer.h \
           commands.h \
           cursor.h \
           debug.h \
           events.h \
           ex_lua.h \
           excommands.h \
           internal_options.h \
           line.h \
           linesearch.h \
           mapping.h \
           mark.h \
           option.h \
           plugin.h \
           printer.h \
           qtprinter.h \
           registers.h \
           schema.h \
           search.h \
           selection.h \
           session.h \
           swapfile.h \
           syntaxdocument.h \
           syntaxhighlight.h \
           undo.h \
           view.h \
           viewcursor.h \
           portability.h \
           yzis.h

SOURCES += action.cpp \
           attribute.cpp \
           buffer.cpp \
           commands.cpp \
           cursor.cpp \
           debug.cpp \
           events.cpp \
           ex_lua.cpp \
           excommands.cpp \
           internal_options.cpp \
           line.cpp \
           linesearch.cpp \
           mapping.cpp \
           mark.cpp \
           option.cpp \
           printer.cpp \
           qtprinter.cpp \
           registers.cpp \
           schema.cpp \
           search.cpp \
           selection.cpp \
           session.cpp \
           swapfile.cpp \
           syntaxdocument.cpp \
           syntaxhighlight.cpp \
           undo.cpp \
           view.cpp \
           viewcursor.cpp
