#! /usr/bin/env python

Import('env')

obj = env.qt4obj('shlib', env)
obj.vnum = '1.0.0'
obj.target = 'libyzis'
obj.source = """
session.cpp mode.cpp mode_command.cpp mode_search.cpp 
mode_visual.cpp mode_ex.cpp mode_insert.cpp view.cpp search.cpp mark.cpp action.cpp 
cursor.cpp selection.cpp line.cpp buffer.cpp linesearch.cpp debug.cpp 
internal_options.cpp option.cpp registers.cpp undo.cpp ex_lua.cpp swapfile.cpp 
schema.cpp viewcursor.cpp mapping.cpp events.cpp syntaxdocument.cpp 
syntaxhighlight.cpp attribute.cpp yzisinfo.cpp yzisinfostartpositionrecord.cpp 
yzisinfojumplistrecord.cpp readtags.c tags_interface.cpp tags_stack.cpp mode_complete.cpp 
viewid.cpp history.cpp color.cpp folding.cpp  font.cpp
"""
obj.uselib = 'QT QTCORE QT3SUPPORT LUA'
obj.execute()

def build_translator_h( target = None, source = None, env = None ) :
	dest = open(str(target[0]),"w")
	dest.write( '#define PREFIX "%s"\n' % env['PREFIX'])
	dest.close()
act = env.Action(build_translator_h, varlist=['PREFIX'])
env.Command('translator.h','', act)
