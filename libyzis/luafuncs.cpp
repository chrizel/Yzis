/* This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <mikmak@yzis.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "luafuncs.h"
#include "debug.h"
#include "view.h"
#include "buffer.h"
#include "action.h"
#include "cursor.h"
#include "session.h"
#include "yzis.h"
#include "mapping.h"
#include "portability.h"
#include "internal_options.h"
#include "events.h"

#include <QDir>

#include "mode_ex.h"

#include "luaengine.h"

#define dbg()    yzDebug("YZLuaFuncs")
#define err()    yzError("YZLuaFuncs")

/*
 * TODO:
 * - invert line/col arguments
 * - test every argument of the functions
 * - find how to add file:line info to error messages
 * - override print() in lua for yzis
 * - clear the lua stack properly
 * - arguments to :source must be passed as argv
 * - add missing function from vim
 * - clean up all the lua calling functions
 * - yzpcall does not need to take nresult argument
 * - use correct printf api
 * - yzpcall should display the type of lua error when error occurs
 * - understand and document the highlight command
 *
 */

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

using namespace yzis;


// ========================================================
//
//                     Lua Commands
//
// ========================================================

void YZLuaFuncs::registerLuaFuncs(lua_State *L)
{
	lua_register(L,"line",line);
	lua_register(L,"setline",setline);
	lua_register(L,"insert",insert);
	lua_register(L,"remove",remove);
	lua_register(L,"insertline",insertline);
	lua_register(L,"appendline",appendline);
	lua_register(L,"replace",replace);
	lua_register(L,"wincol",wincol);
	lua_register(L,"winline",winline);
	lua_register(L,"winpos",winpos);
	lua_register(L,"goto",_goto);
	lua_register(L,"scrcol",scrcol);
	lua_register(L,"scrline",scrline);
	lua_register(L,"scrgoto",scrgoto);
	lua_register(L,"deleteline",deleteline);
	lua_register(L,"version",version);
	lua_register(L,"filename",filename);
	lua_register(L,"color",color);
	lua_register(L,"linecount",linecount);
	lua_register(L,"sendkeys",sendkeys);
	lua_register(L,"highlight",highlight);
	lua_register(L,"connect",connect);
	lua_register(L,"source",source);
	lua_register(L,"yzdebug",yzdebug);
	lua_register(L,"setlocal",setlocal);
	lua_register(L,"newoption",newoption);
	lua_register(L,"set",set);
	lua_register(L,"map",map);
	lua_register(L,"unmap",unmap);
	lua_register(L,"imap",imap);
	lua_register(L,"iunmap",iunmap);
	lua_register(L,"nmap",nmap);
	lua_register(L,"nunmap",nunmap);
	lua_register(L,"omap",omap);
	lua_register(L,"ounmap",ounmap);
	lua_register(L,"vmap",vmap);
	lua_register(L,"vunmap",vunmap);
	lua_register(L,"cmap",cmap);
	lua_register(L,"cunmap",cunmap);
	lua_register(L,"noremap",noremap);
	lua_register(L,"nnoremap",nnoremap);
	lua_register(L,"vnoremap",vnoremap);
	lua_register(L,"onoremap",onoremap);
	lua_register(L,"inoremap",inoremap);
	lua_register(L,"cnoremap",cnoremap);
	lua_register(L,"matchpair",matchpair);
	lua_register(L,"mode",mode);
	lua_register(L,"edit",edit);
    dbg() << HERE() << " done." << endl;
}


int YZLuaFuncs::line(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "line", "line")) return 0;
	int line = ( int )lua_tonumber( L,1 );
	lua_pop(L,1);

	line = line ? line - 1 : 0;

	YZView* cView = YZSession::self()->currentView();
	QString	t = cView->myBuffer()->textline( line );

	lua_pushstring( L, t.toUtf8() ); // first result
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ; 
}

int YZLuaFuncs::setline(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "setline", "line, text")) return 0;
	int sLine = ( int )lua_tonumber( L,1 );
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	sLine = sLine ? sLine - 1 : 0;
	if (text.indexOf("\n") != -1) {
		printf("setline with line containing \n");
		YZASSERT_EQUALS( lua_gettop(L),  0  );
		return 0;
	}
	YZView* cView = YZSession::self()->currentView();
	cView->myBuffer()->action()->replaceLine(cView, sLine, text );

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return 0; // no result
}

int YZLuaFuncs::insert(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 3, 3, "insert", "line, col, text")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 3 ) );
	lua_pop(L,3);

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::self()->currentView();
	QStringList list = text.split( "\n" );
	QStringList::Iterator it = list.begin(), end = list.end();
	for ( ; it != end; ++it ) {
		if ( sLine >= cView->myBuffer()->lineCount() ) cView->myBuffer()->action()->insertNewLine( cView, 0, sLine );
		cView->myBuffer()->action()->insertChar( cView, sCol, sLine, *it );
		sCol=0;
		sLine++;
	}

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ; // no result
}

int YZLuaFuncs::remove(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 3, 3, "remove", "line, col, nb")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	int sNb = ( int )lua_tonumber( L,3 );
	lua_pop(L,3);

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::self()->currentView();
	YZBuffer * cBuffer = cView->myBuffer();
	YZAction * cAction = cBuffer->action();
	cAction->deleteChar(cView, sCol, sLine, sNb);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ; // no result
}

int YZLuaFuncs::insertline(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "insertline", "line, text")) return 0;
	int sLine = ( int )lua_tonumber( L,1 );
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::self()->currentView();
	QStringList list = text.split( "\n" );
	QStringList::Iterator it = list.begin(), end = list.end();
	for ( ; it != end; ++it ) {
		YZBuffer * cBuffer = cView->myBuffer();
		YZAction * cAction = cBuffer->action();
		if (!(cBuffer->isEmpty() && sLine == 0)) {
			cAction->insertNewLine( cView, 0, sLine );
		}
		cAction->insertChar( cView, 0, sLine, *it );
		sLine++;
	}

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ; // no result
}

int YZLuaFuncs::appendline(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "appendline", "text")) return 0;
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZView* cView = YZSession::self()->currentView();
	YZBuffer * cBuffer = cView->myBuffer();
	YZAction * cAction = cBuffer->action();
	QStringList list = text.split( "\n" );
	QStringList::Iterator it = list.begin(), end = list.end();
	for ( ; it != end; ++it ) {
		if (cBuffer->isEmpty()) {
			cAction->insertChar( cView, 0, 0, *it );
		} else {
			cAction->insertLine( cView, cBuffer->lineCount(), *it );
		}
	}

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ; // no result
}

int YZLuaFuncs::replace(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 3, 3, "replace", "line, col, text")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 3 ) );
	lua_pop(L,3);

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	if (text.indexOf('\n') != -1) {
		// replace does not accept multiline strings, it is too strange
        // XXX raises an error
		YZASSERT_EQUALS( lua_gettop(L),  0  );
		return  0 ;
	}

	YZView* cView = YZSession::self()->currentView();
	if ( sLine >= cView->myBuffer()->lineCount() ) {
		cView->myBuffer()->action()->insertNewLine( cView, 0, sLine );
		sCol = 0;
	}
	cView->myBuffer()->action()->replaceChar( cView, sCol, sLine, text );

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ; // no result
}

int YZLuaFuncs::winline(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "winline", "")) return 0;

	YZView* cView = YZSession::self()->currentView();
	uint result = cView->getBufferCursor().y() + 1;

	lua_pushnumber( L, result ); // first result
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ; // one result
}

int YZLuaFuncs::wincol(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "wincol", "")) return 0;
	YZView* cView = YZSession::self()->currentView();
	uint result = cView->getBufferCursor().x() + 1;

	lua_pushnumber( L, result ); // first result
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ; // one result
}

int YZLuaFuncs::scrline(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "scrline", "")) return 0;
	YZView* cView = YZSession::self()->currentView();
	uint result = cView->getCursor().y() + 1;

	lua_pushnumber( L, result ); // first result
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ; // one result
}

int YZLuaFuncs::scrcol(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "scrcol", "")) return 0;
	YZView* cView = YZSession::self()->currentView();
	uint result = cView->getCursor().x() + 1;

	lua_pushnumber( L, result ); // first result
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ; // one result
}

int YZLuaFuncs::winpos(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "winpos", "")) return 0;

	YZView* cView = YZSession::self()->currentView();
	uint line = cView->getBufferCursor().y() + 1;
	uint col = cView->getBufferCursor().x() + 1;

	lua_pushnumber( L, col ); 
	lua_pushnumber( L, line ); 
	YZASSERT_EQUALS( lua_gettop(L),  2  );
	return  2 ; // two results
}

int YZLuaFuncs::_goto(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "goto", "line, col")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	lua_pop(L,2);

	YZView* cView = YZSession::self()->currentView();
	cView->gotoxy(sCol ? sCol - 1 : 0, sLine ? sLine - 1 : 0 );

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::scrgoto(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "scrgoto", "line, col")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	lua_pop(L,2);

	YZView* cView = YZSession::self()->currentView();
	cView->gotodxdy(sCol ? sCol - 1 : 0, sLine ? sLine - 1 : 0 );

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::deleteline(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "deleteline", "line")) return 0;
	int sLine = ( int )lua_tonumber( L,1 );
	lua_pop(L,1);

	YZView* cView = YZSession::self()->currentView();
	QList<QChar> regs;
	regs << QChar( '"' ) ;
	cView->myBuffer()->action()->deleteLine( cView, sLine ? sLine - 1 : 0, 1, regs );

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::filename(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "filename", "")) return 0;
	YZView* cView = YZSession::self()->currentView();
	QByteArray fn = cView->myBuffer()->fileName().toUtf8();
	const char *filename = fn.data();

	lua_pushstring( L, filename );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}

int YZLuaFuncs::color(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "color", "line, col")) return 0;
	int sCol = ( int )lua_tonumber( L,1 );
	int sLine = ( int )lua_tonumber( L,2 );
	lua_pop(L,2);
	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::self()->currentView();
	QByteArray c = cView->drawColor(  sCol, sLine ).name().toUtf8();
	const char *color = c.data();

//	dbg() << "Asked color: " << color.latin1() << endl;
	lua_pushstring( L, color );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ; // one result
}

int YZLuaFuncs::linecount(lua_State *L) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "linecount", "")) return 0;

	YZView* cView = YZSession::self()->currentView();

	lua_pushnumber( L, cView->myBuffer()->lineCount()); // first result
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ; // one result
}

int YZLuaFuncs::version( lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "version", "")) return 0;

	lua_pushstring( L, VERSION_CHAR );
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}

int YZLuaFuncs::sendkeys( lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "sendkeys", "text")) return 0;
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZSession::self()->sendMultipleKeys(YZSession::self()->currentView(), text);

	// nothing to return
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::highlight( lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 3, 100, "highlight", "type, style, ...")) return 0;

	QStringList arg;
	int n = lua_gettop(L);
	for ( int i = 1; i <= n ; i++ ) {
		arg << ( char * )lua_tostring( L, i );
	}
	lua_pop(L,n);

	YZExCommandArgs args(NULL, QString(), QString(), arg.join(" "), 0, 0, true);
	YZSession::self()->getExPool()->highlight(args);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::yzdebug( lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "yzdebug", "text")) return 0;
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	dbg() << text << endl;	

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::yzprint(lua_State * /*L*/) {
	// fetch string from the stack
	// print it
	return 0;
}

int YZLuaFuncs::connect(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "connect", "event (string), function (string)")) return 0;
	QString event = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString function = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	YZSession::self()->eventConnect(event,function);
	
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::source(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "source", "filename")) return 0;
	QString filename = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );

	YZLuaEngine::self()->source(filename);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::setlocal(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "setlocal", "option name")) return 0;
	QString option = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZExCommandArgs ex (YZSession::self()->currentView(), QString(), "setlocal", option, 0, 0, true);
	YZSession::self()->getExPool()->set(ex);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;	
}

int YZLuaFuncs::newoption(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 6, 6, "newoption", "option name, group name, default value, value, visibility (number), type (number)")) return 0;
	QString option = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString group = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	QString defaultvalue = QString::fromUtf8( (  char * )lua_tostring (  L, 3 ) );
	QString value = QString::fromUtf8( (  char * )lua_tostring (  L, 4 ) );
	OptContext visibility = (OptContext)(int)lua_tonumber ( L, 5 );
	OptType type = (OptType)(int)lua_tonumber ( L, 6 );
	lua_pop(L,6);

	YZSession::self()->getOptions()->createOption(option, group, defaultvalue, value, visibility, type );

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::map(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "map", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring ( L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	YZMapping::self()->addGlobalMapping(key, mapp);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::unmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "unmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZMapping::self()->deleteGlobalMapping(key);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::imap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "imap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	YZMapping::self()->addInsertMapping(key, mapp);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::iunmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "iunmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZMapping::self()->deleteInsertMapping(key);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::omap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "omap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	YZMapping::self()->addPendingOpMapping(key, mapp);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::ounmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "ounmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZMapping::self()->deletePendingOpMapping(key);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::vmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "vmap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	YZMapping::self()->addVisualMapping(key, mapp);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::vunmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "vunmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZMapping::self()->deleteVisualMapping(key);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::cmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "cmap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	YZMapping::self()->addCmdLineMapping(key, mapp);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::cunmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "cunmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZMapping::self()->deleteCmdLineMapping(key);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::nmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "nmap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	YZMapping::self()->addNormalMapping(key, mapp);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::nunmap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "nunmap", "key (string)")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZMapping::self()->deleteNormalMapping(key);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::noremap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "noremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	lua_pop(L,2);
	
	YZMapping::self()->addGlobalNoreMapping(key, mapp);
	
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::nnoremap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "nnoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	lua_pop(L,2);
	
	YZMapping::self()->addNormalNoreMapping(key, mapp);
	
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::vnoremap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "vnoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	lua_pop(L,2);
	
	YZMapping::self()->addVisualNoreMapping(key, mapp);
	
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::onoremap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "onoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	lua_pop(L,2);
	
	YZMapping::self()->addPendingOpNoreMapping(key, mapp);
	
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::inoremap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "inoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	lua_pop(L,2);
	
	YZMapping::self()->addInsertNoreMapping(key, mapp);
	
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::cnoremap(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 2, 2, "cnoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	lua_pop(L,2);
	
	YZMapping::self()->addCmdLineNoreMapping(key, mapp);
	
	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::matchpair(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "matchpair", "")) return 0;

	bool found = false;
	YZView *v = YZSession::self()->currentView();
	YZCursor s = v->getBufferCursor();
	YZCursor c = v->myBuffer()->action()->match(v, s, &found);

	lua_pushboolean(L , found);
	lua_pushnumber(L, c.x());
	lua_pushnumber(L, c.y());
	YZASSERT_EQUALS( lua_gettop(L),  3  );
	return  3 ;
}

int YZLuaFuncs::mode(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 0, 0, "mode", "")) return 0;

	YZView *v = YZSession::self()->currentView();
	QString mode = v->mode();

	lua_pushstring(L,mode.toUtf8());
	YZASSERT_EQUALS( lua_gettop(L),  1  );
	return  1 ;
}

int YZLuaFuncs::edit(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "edit", "filename")) return 0;
	QString fname = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	lua_pop(L,1);

	if (!fname.isEmpty())
		YZSession::self()->createBufferAndView(fname);

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;
}

int YZLuaFuncs::set(lua_State *L ) {
	if (!YZLuaEngine::checkFunctionArguments(L, 1, 1, "set", "option (string)")) return 0;
	QString option = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZSession::self()->getExPool()->set(YZExCommandArgs(YZSession::self()->currentView(), QString(), QString(), option, 0, 0, true));

	YZASSERT_EQUALS( lua_gettop(L),  0  );
	return  0 ;	
}

