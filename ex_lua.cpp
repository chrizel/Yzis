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

#include <iostream>
#include <qfileinfo.h>
#include <qdir.h>
#include "ex_lua.h"
#include "debug.h"
#include "view.h"
#include "buffer.h"
#include "action.h"
#include "cursor.h"
#include "session.h"
#include "yzis.h"
#include "translator.h"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}
YZExLua::YZExLua() {
	st = lua_open();
	luaopen_base(st);
	luaopen_string( st );
	luaopen_table( st );
	luaopen_math( st );
	luaopen_io( st );
	luaopen_debug( st );
	yzDebug() << "Lua " << lua_version() << " loaded" << endl;
	lua_register(st,"text",text);
	lua_register(st,"line",line);
	lua_register(st,"insert",insert);
	lua_register(st,"insertline",insertline);
	lua_register(st,"appendline",appendline);
	lua_register(st,"replace",replace);
	lua_register(st,"wincol",wincol);
	lua_register(st,"winline",winline);
	lua_register(st,"goto",_goto);
	lua_register(st,"deleteline",deleteline);
	lua_register(st,"version",version);
	lua_register(st,"filename",filename);
	lua_register(st,"getcolor",getcolor);
	lua_register(st,"linecount",linecount);
	lua_register(st,"sendkeys",sendkeys);
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
	QStringList candidates;
	candidates << filename 
	           << QDir::currentDirPath()+"/"+filename
	           << QDir::homeDirPath()+"/.yzis/scripts/"+filename
		       << QString( PREFIX )+"/share/yzis/scripts/"+filename;
	QString found;
	for( QStringList::iterator it = candidates.begin(); it!=candidates.end(); ++it) {
		found = *it;
		if (QFile::exists( found )) break;

		if ((found.right(4) != ".lua")) {
			found += ".lua";
			if (QFile::exists(found)) break;
		}
		found = "";
	}

	if (found.isEmpty()) {
		YZSession::me->popupMessage(tr("The file %1 could not be found in standard directories" ).arg( filename ));
		return QString::null;
	}

	lua_pushstring(st,"dofile");
	lua_gettable(st, LUA_GLOBALSINDEX);
	lua_pushstring(st,found.latin1());
	int lua_err = lua_pcall(st,1,1,0);
	if (lua_err) {
		QString errorMsg = lua_tostring(st,lua_gettop(st));
		printf("%s\n", errorMsg.latin1() );
		YZSession::me->popupMessage(tr("Lua error when running file %1:\n%2").arg(found).arg(errorMsg));
	}

	return QString::null;
}

//All Lua functions return the number of results returned
int YZExLua::text(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 4 ) return 0; //mis-use of the function

	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	int eCol = ( int )lua_tonumber( L,3 );
	int eLine = ( int )lua_tonumber( L,4 );

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

int YZExLua::line(lua_State *L) {
	int n = lua_gettop( L );
	if ( n != 1 ) return 0; //mis-use of the function

	int line = ( int )lua_tonumber( L,1 );

	line = line ? line - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	QString	t = cView->myBuffer()->textline( line );
	lua_pushstring( L, t ); // first result
	return 1; // one result
}

int YZExLua::insert(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 3 ) return 0; //mis-use of the function

	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	QString text = ( char * )lua_tostring ( L, 3 );

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	QStringList list = QStringList::split( "\n", text );
	for ( QStringList::Iterator it = list.begin(); it != list.end(); it++ ) {
		if ( ( unsigned int )sLine >= cView->myBuffer()->lineCount() ) cView->myBuffer()->action()->insertNewLine( cView, 0, sLine );
		cView->myBuffer()->action()->insertChar( cView, sCol, sLine, *it );
		sCol=0;
		sLine++;
	}

	return 0; // no result
}

int YZExLua::insertline(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 2 ) return 0; //mis-use of the function

	int sLine = ( int )lua_tonumber( L,1 );
	QString text = ( char * )lua_tostring ( L, 2 );

	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	QStringList list = QStringList::split( "\n", text );
	for ( QStringList::Iterator it = list.begin(); it != list.end(); it++ ) {
		YZBuffer * cBuffer = cView->myBuffer();
		YZAction * cAction = cBuffer->action();
		if (!(cBuffer->isEmpty() && sLine == 0)) {
			cAction->insertNewLine( cView, 0, sLine );
		}
		cAction->insertChar( cView, 0, sLine, *it );
		sLine++;
	}

	return 0; // no result
}

int YZExLua::appendline(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 1 ) return 0; //mis-use of the function

	QString text = ( char * )lua_tostring ( L, 1 );

	YZView* cView = YZSession::me->currentView();
	YZBuffer * cBuffer = cView->myBuffer();
	YZAction * cAction = cBuffer->action();
	QStringList list = QStringList::split( "\n", text );
	for ( QStringList::Iterator it = list.begin(); it != list.end(); it++ ) {
		if (cBuffer->isEmpty()) {
			cAction->insertChar( cView, 0, 0, *it );
		} else {
			cAction->insertLine( cView, cBuffer->lineCount(), *it );
		}
	}

	return 0; // no result
}

int YZExLua::replace(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 3 ) return 0; //mis-use of the function

	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	QString text = ( char * )lua_tostring ( L, 3 );

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	if (text.find('\n') != -1) {
		// replace does not accept multiline strings, it is too strange
		return 0;
	}

	YZView* cView = YZSession::me->currentView();
	if ( ( unsigned int )sLine >= cView->myBuffer()->lineCount() ) {
		cView->myBuffer()->action()->insertNewLine( cView, 0, sLine );
		sCol = 0;
	}
	cView->myBuffer()->action()->replaceChar( cView, sCol, sLine, text );

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

int YZExLua::_goto(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 2 ) return 0; //mis-use of the function

	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );

	YZView* cView = YZSession::me->currentView();
	cView->gotoxy(sCol ? sCol - 1 : 0, sLine ? sLine - 1 : 0 );

	return 0; // one result
}

int YZExLua::deleteline(lua_State *L) {
	int n = lua_gettop( L );
	if ( n < 1 ) return 0; //mis-use of the function

	int sLine = ( int )lua_tonumber( L,1 );

	YZView* cView = YZSession::me->currentView();
	QValueList<QChar> regs;
	regs << QChar( '"' ) ;
	cView->myBuffer()->action()->deleteLine( cView, sLine ? sLine - 1 : 0, 1, regs );

	return 0; // one result
}

int YZExLua::filename(lua_State *L) {
	YZView* cView = YZSession::me->currentView();
	const char *filename = cView->myBuffer()->fileName();

	lua_pushstring( L, filename ); // first result
	return 1; // one result
}

int YZExLua::getcolor(lua_State *L) {
	int n = lua_gettop( L );
	if ( n != 2 ) return 0; //mis-use of the function

	int sCol = ( int )lua_tonumber( L,1 );
	int sLine = ( int )lua_tonumber( L,2 );
	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	QString color = cView->drawColor( sCol, sLine ).name();

//	yzDebug() << "Asked color : " << color.latin1() << endl;
	lua_pushstring( L, color ); // first result

	return 1; // one result
}

int YZExLua::linecount(lua_State *L) {
	YZView* cView = YZSession::me->currentView();
	lua_pushnumber( L, cView->myBuffer()->lineCount()); // first result
	return 1; // one result
}

int YZExLua::version( lua_State *L ) {
	lua_pushstring( L, VERSION_CHAR );
	return 1;
}

int YZExLua::sendkeys( lua_State *L ) {
	YZView* cView = YZSession::me->currentView();
	int n = lua_gettop( L );
	if ( n != 1 ) return 0; //mis-use of the function
	QString text = ( char * )lua_tostring ( L, 3 );
	cView->sendMultipleKey( text );
	// nothing to return
	return 0;
}

#include "ex_lua.moc"

