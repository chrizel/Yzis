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

void print_lua_stack_value(lua_State*L, int index)
{
	printf("stack %d ", index );
	switch(lua_type(L,index)) {
		case LUA_TNIL: printf("nil\n"); break;
		case LUA_TNUMBER: printf("number: %f\n", lua_tonumber(L,index)); break;
		case LUA_TBOOLEAN: printf("boolean: %d\n", lua_toboolean(L,index)); break;
		case LUA_TSTRING: printf("string: \"%s\"\n", lua_tostring(L,index)); break;
		case LUA_TTABLE: printf("table\n"); break;
		case LUA_TFUNCTION: printf("function\n"); break;
		case LUA_TUSERDATA: printf("userdata\n"); break;
		case LUA_TTHREAD: printf("thread\n"); break;
		case LUA_TLIGHTUSERDATA: printf("light user data:\n");break;
		default:
		printf("Unknown lua type: %d\n", lua_type(L,index) );
	}
}

void print_lua_stack(lua_State *L) {
	for(int i=1; i<=lua_gettop(L); i++) {
		print_lua_stack_value(L,i);
	}
}

YZExLua * YZExLua::_instance = 0L;

YZExLua * YZExLua::instance()
{
	if (YZExLua::_instance == NULL) YZExLua::_instance = new YZExLua();
	return YZExLua::_instance;
}

YZExLua::YZExLua() {
	L = lua_open();
	luaopen_base(L);
	luaopen_string( L );
	luaopen_table( L );
	luaopen_math( L );
	luaopen_io( L );
	luaopen_debug( L );
	yzDebug() << "Lua " << lua_version() << " loaded" << endl;
	lua_register(L,"text",text);
	lua_register(L,"line",line);
	lua_register(L,"insert",insert);
	lua_register(L,"insertline",insertline);
	lua_register(L,"appendline",appendline);
	lua_register(L,"replace",replace);
	lua_register(L,"wincol",wincol);
	lua_register(L,"winline",winline);
	lua_register(L,"goto",_goto);
	lua_register(L,"deleteline",deleteline);
	lua_register(L,"version",version);
	lua_register(L,"filename",filename);
	lua_register(L,"color",color);
	lua_register(L,"linecount",linecount);
	lua_register(L,"sendkeys",sendkeys);
}

YZExLua::~YZExLua() {
	lua_close(L);
}

QString YZExLua::lua(YZView *, const QString& args) {
	printf("calling hop lua %s\n", args.latin1() );
	execInLua( "print('called from yzis')\n" );
	lua_pushstring(L,"text");
	lua_gettable(L,LUA_GLOBALSINDEX); //to store function name
	//now arguments in the right order
	lua_pushnumber( L, 0 ); //start col
	lua_pushnumber( L, 0 ); //start line
	lua_pushnumber( L, 0 ); //end col
	lua_pushnumber( L, 3 ); //end line

	pcall( 4, 1, 0, QString("Error when executing: %1").arg(args) );
	return QString::null;
}

//callers
QString YZExLua::source( YZView *, const QString& args ) {
	printf("YZExLua::source( %s )\n", args.latin1() );
	QString filename = args.mid( args.find( " " ) +1 );
	printf("YZExLua - filneame = %s\n", filename.latin1() );
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

	lua_pushstring(L,"dofile");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L,found.latin1());
	pcall(1,1,0, tr("Lua error when running file %1:").arg(found) );
	return QString::null;
}

int YZExLua::execInLua( const QString & luacode )
{
	lua_pushstring(L, "loadstring" );
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, luacode );
	pcall(1,1,0, "");
	pcall(0,0,0, "");
	return 0;
}

bool YZExLua::pcall( int nbArg, int nbReturn, int errLevel, const QString & errorMsg )
{
	int lua_err = lua_pcall(L,nbArg,nbReturn,errLevel);
	if (! lua_err) return true;
	QString luaErrorMsg = lua_tostring(L,lua_gettop(L));
	printf("%s\n", luaErrorMsg.latin1() );
	YZSession::me->popupMessage(errorMsg + luaErrorMsg );
	return false;
}

void YZExLua::yzisprint(const QString & text)
{
	printf("yzisprint:%s\n", text.latin1());
}

// ========================================================
//
//                     Lua Commands
//
// ========================================================

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
	uint result = cView->getBufferCursor()->getY() + 1;

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

int YZExLua::color(lua_State *L) {
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

int YZExLua::myprint(lua_State * /*L*/)
{
	// fetch string from the stack
	// print it
	return 0;
}

#include "ex_lua.moc"

