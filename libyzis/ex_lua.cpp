/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <mikmak@yzis.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <qfileinfo.h>
#include "ex_lua.h"
#include "debug.h"
#include "session.h"
extern "C" {
#include <lauxlib.h>
}
YZExLua::YZExLua() {
	st = lua_open();
	yzDebug() << "Lua " << lua_version() << " loaded" << endl;
	lua_register(st,"text",text);
	lua_register(st,"insert",insert);
}

YZExLua::~YZExLua() {
	lua_close(st);
}

QString YZExLua::lua(YZView *view, const QString& inputs) {
	lua_pushstring(st,"text");
	lua_gettable(st,LUA_GLOBALSINDEX); //to store function name
	//now arguments in the right order
	lua_pushnumber( st, 0 ); //start col
	lua_pushnumber( st, 0 ); //start line
	lua_pushnumber( st, 0 ); //end col
	lua_pushnumber( st, 3 ); //end line

	if ( lua_pcall( st, 4, 1, 0 ) )
		yzDebug() << "YZExLua::lua " << lua_tostring(st, -1) << endl;
	else
		yzDebug() << "YZExLua::text returned " << lua_tostring( st, -1 ) << endl; //XXX remove me
	return QString::null;
}

//callers
QString YZExLua::loadFile( YZView *view, const QString& inputs ) {
	QString filename = inputs.mid( inputs.find( " " ) +1 );
	QFileInfo fi ( filename );
	filename = fi.absFilePath();
	yzDebug() << "LUA : sourcing file " << filename << endl;
	if ( fi.exists() ) {
		if ( lua_dofile( st, filename ) ) 
			yzDebug() << "YZExLua::loadFile failed : " << lua_tostring(st, -1) << endl;
		else
			yzDebug() << "YZExLua::loadFile succeded " << endl;
	}
	return QString::null;
}

//All Lua functions return the number of results returned
int YZExLua::text(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 4 ) return 0; //mis-use of the function
	
	int sCol = lua_tonumber( L, 1 );
	int sLine = lua_tonumber( L,2 );
	int eCol = lua_tonumber( L,3 );
	int eLine = lua_tonumber( L,4 );
	
	YZView* cView = YZSession::me->currentView();
	QString result,t;
	for ( int i = sLine ; i < eLine; i++ ) {
		t = cView->myBuffer()->textline( i );
		if ( i == sLine && i == eLine ) 
			result += t.mid( sCol, eCol - sCol );
		else if ( i == sLine )
			result += t.mid( sCol, t.length() - sCol ) + "\n";
		else if ( i == eLine )
			result += t.mid( 0, eCol );
		else result += t + "\n";
	}
	lua_pushstring( L, result ); // first result
	return 1; // one result
}

int YZExLua::insert(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 3 ) return 0; //mis-use of the function
	
	int sCol = lua_tonumber( L, 1 );
	int sLine = lua_tonumber( L,2 );
	QString text = lua_tostring ( L, 3 );
	
	YZView* cView = YZSession::me->currentView();
	//split new lines XXX
	cView->myBuffer()->insertChar(sCol, sLine, text);
	cView->refreshScreen();

	return 0; // no result
}

#include "ex_lua.moc"

