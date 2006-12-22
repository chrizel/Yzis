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
INCLUDEPATH += ../ ../.. $$(LUAINCLUDE)
QT        += xml 
CONFIG    += console warn_on debug dll
CONFIG    += rtti # necessary for dynamic cast

win32-msvc {
	DESTDIR = ./
	LIBS += $$(LUALIB)/Lua.lib $$(LUALIB)/LuaLib.lib
}

win32-g++ {
	DESTDIR = ./
	LIBS += $$(LUALIB)/Lua.lib $$(LUALIB)/LuaLib.lib
    DEFINES += YZIS_WIN32_GCC
}

# Input
HEADERS += \
    ../../libyzis/action.h \
    ../../libyzis/attribute.h \
    ../../libyzis/buffer.h \
    ../../libyzis/color.h \
    ../../libyzis/cursor.h \
    ../../libyzis/debug.h \
    ../../libyzis/drawbuffer.h \
    ../../libyzis/events.h \
	../../libyzis/luaengine.h \
	../../libyzis/luafuncs.h \
	../../libyzis/luaregexp.h \
    ../../libyzis/folding.h \
    ../../libyzis/font.h \
    ../../libyzis/history.h \
    ../../libyzis/internal_options.h \
    ../../libyzis/line.h \
    ../../libyzis/linesearch.h \
    ../../libyzis/mapping.h \
    ../../libyzis/mark.h \
    ../../libyzis/mode.h \
    ../../libyzis/mode_command.h \
    ../../libyzis/mode_complete.h \
    ../../libyzis/mode_ex.h \
    ../../libyzis/mode_insert.h \
    ../../libyzis/mode_search.h \
    ../../libyzis/mode_visual.h \
    ../../libyzis/option.h \
    ../../libyzis/portability.h \
    ../../libyzis/printer.h \
    ../../libyzis/qtprinter.h \
    ../../libyzis/readtags.c \
    ../../libyzis/readtags.h \
    ../../libyzis/registers.h \
    ../../libyzis/schema.h \
    ../../libyzis/search.h \
    ../../libyzis/selection.h \
    ../../libyzis/session.h \
    ../../libyzis/swapfile.h \
    ../../libyzis/syntaxdocument.h \
    ../../libyzis/syntaxhighlight.h \
    ../../libyzis/tags_interface.h \
    ../../libyzis/tags_stack.h \
    ../../libyzis/translator.h.in \
    ../../libyzis/undo.h \
    ../../libyzis/view.h \
    ../../libyzis/viewcursor.h \
    ../../libyzis/viewid.h \
    ../../libyzis/yzis.h \
    ../../libyzis/yzisinfo.h \
    ../../libyzis/yzisinfojumplistrecord.h \
    ../../libyzis/yzisinfostartpositionrecord.h



SOURCES +=  \
    ../../libyzis/action.cpp \
    ../../libyzis/attribute.cpp \
    ../../libyzis/buffer.cpp \
    ../../libyzis/color.cpp \
    ../../libyzis/cursor.cpp \
    ../../libyzis/debug.cpp \
    ../../libyzis/drawbuffer.cpp \
    ../../libyzis/events.cpp \
	../../libyzis/luaengine.cpp \
	../../libyzis/luafuncs.cpp \
	../../libyzis/luaregexp.cpp \
    ../../libyzis/folding.cpp \
    ../../libyzis/font.cpp \
    ../../libyzis/history.cpp \
    ../../libyzis/internal_options.cpp \
    ../../libyzis/line.cpp \
    ../../libyzis/linesearch.cpp \
    ../../libyzis/mapping.cpp \
    ../../libyzis/mark.cpp \
    ../../libyzis/mode.cpp \
    ../../libyzis/mode_command.cpp \
    ../../libyzis/mode_complete.cpp \
    ../../libyzis/mode_ex.cpp \
    ../../libyzis/mode_insert.cpp \
    ../../libyzis/mode_search.cpp \
    ../../libyzis/mode_visual.cpp \
    ../../libyzis/option.cpp \
    ../../libyzis/printer.cpp \
    ../../libyzis/qtprinter.cpp \
    ../../libyzis/readtags.c \
    ../../libyzis/registers.cpp \
    ../../libyzis/schema.cpp \
    ../../libyzis/search.cpp \
    ../../libyzis/selection.cpp \
    ../../libyzis/session.cpp \
    ../../libyzis/swapfile.cpp \
    ../../libyzis/syntaxdocument.cpp \
    ../../libyzis/syntaxhighlight.cpp \
    ../../libyzis/tags_interface.cpp \
    ../../libyzis/tags_stack.cpp \
    ../../libyzis/undo.cpp \
    ../../libyzis/view.cpp \
    ../../libyzis/viewcursor.cpp \
    ../../libyzis/viewid.cpp \
    ../../libyzis/yzisinfo.cpp \
    ../../libyzis/yzisinfojumplistrecord.cpp \
    ../../libyzis/yzisinfostartpositionrecord.cpp



