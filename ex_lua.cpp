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
	lua_register(st,"replace",replace);
	lua_register(st,"wincol",wincol);
	lua_register(st,"winline",winline);
	lua_register(st,"goto",gotoxy);
	lua_register(st,"delete",delline);
}

YZExLua::~YZExLua() {
	lua_close(st);
}

QString YZExLua::lua(YZView *, const QString& ) {
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
QString YZExLua::loadFile( YZView *, const QString& inputs ) {
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

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;
	eCol = eCol ? eCol - 1 : 0;
	eLine = eLine ? eLine - 1 : 0;
	
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

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;
	
	YZView* cView = YZSession::me->currentView();
	QStringList list = QStringList::split( "\n", text );
	for ( QStringList::Iterator it = list.begin(); it != list.end(); it++ ) {
		cView->myBuffer()->insertChar(sCol, sLine, *it);
		sCol=0;
		sLine++;
	}
	//XXX should not be needed ... bug in invalidateLine ?
	cView->refreshScreen();

	return 0; // no result
}

int YZExLua::replace(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 3 ) return 0; //mis-use of the function
	
	int sCol = lua_tonumber( L, 1 );
	int sLine = lua_tonumber( L,2 );
	QString text = lua_tostring ( L, 3 );

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;
	
	YZView* cView = YZSession::me->currentView();
	QStringList list = QStringList::split( "\n", text );
	
	QString v = cView->myBuffer()->textline( sLine );
	v = v.mid( 0, sCol );
	v += list[ 0 ]; //for the first line we append at EOL
	cView->myBuffer()->replaceLine(v, sLine);
	sCol=0;
	sLine++;
	if ( list.count() > 1 )
		for ( uint i = 1 ; i < list.count() ; i++ )
			cView->myBuffer()->insertLine( list[ i ], sLine++ );
	
	//XXX should not be needed ... bug in invalidateLine ?
	cView->refreshScreen();

	return 0; // no result
}

int YZExLua::winline(lua_State *L) {
	YZView* cView = YZSession::me->currentView();
	uint result = cView->getBufferCursor()->getY() - 1;	

	lua_pushnumber( L, result ); // first result
	return 1; // one result
}

int YZExLua::wincol(lua_State *L) {
	YZView* cView = YZSession::me->currentView();
	uint result = cView->getBufferCursor()->getX() + 1;	

	lua_pushnumber( L, result ); // first result
	return 1; // one result
}

int YZExLua::gotoxy(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 2 ) return 0; //mis-use of the function
	
	int sCol = lua_tonumber( L, 1 );
	int sLine = lua_tonumber( L,2 );
	
	YZView* cView = YZSession::me->currentView();
	cView->gotoxy(sCol ? sCol - 1 : 0, sLine ? sLine - 1 : 0 );

	return 0; // one result
}

int YZExLua::delline(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 1 ) return 0; //mis-use of the function
	
	int sLine = lua_tonumber( L,1 );
	
	YZView* cView = YZSession::me->currentView();
	cView->myBuffer()->deleteLine( sLine ? sLine - 1 : 0 );

	return 0; // one result
}

#include "ex_lua.moc"

