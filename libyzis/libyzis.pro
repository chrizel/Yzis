TEMPLATE = lib
TARGET = libqtyzis
SOURCES = action.cpp excommands.cpp printer.cpp swapfile.cpp attribute.cpp internal_options.cpp qtprinter.cpp syntaxdocument.cpp buffer.cpp line.cpp registers.cpp syntaxhighlight.cpp commands.cpp linesearch.cpp schema.cpp undo.cpp cursor.cpp mapping.cpp search.cpp view.cpp debug.cpp mark.cpp selection.cpp viewcursor.cpp ex_lua.cpp option.cpp session.cpp
CONFIG += warn_on debug thread qt dll
INCLUDEPATH = d:\msys\1.0\local\include d:\msys\1.0\usr\include ..
LIBS += -lLua -lLuaLib -L/usr/local/lib


