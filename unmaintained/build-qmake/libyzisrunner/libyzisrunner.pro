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
QT        += xml 
CONFIG    += console warn_on debug dll
CONFIG    += rtti # necessary for dynamic cast

win32-g++ {
	LIBS += $$(LUALIB)/Lua.lib $$(LUALIB)/LuaLib.lib
    DEFINES += YZIS_WIN32_GCC
    LIBS += ../libyzis/liblibyzis.a
}

linux-g++ {
    LIBS += -L../bin/ -llibyzis
}

DESTDIR = ../bin
INCLUDEPATH += ../ ../..


# Input
HEADERS += \
    ../../libyzisrunner/NoGuiSession.h \
    ../../libyzisrunner/NoGuiView.h


SOURCES +=  \
    ../../libyzis/debug.cpp \
    ../../libyzis/option.cpp \
    ../../libyzis/internal_options.cpp \
    ../../libyzis/search.cpp \
    ../../libyzisrunner/main.cpp


