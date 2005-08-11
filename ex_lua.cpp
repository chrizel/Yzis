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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

#include <iostream>
#include "ex_lua.h"
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

#include <stdarg.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "mode_ex.h"

/*
 * TODO:
 * - invert line/col arguments
 * - test every argument of the functions
 * - find how to add file:line info to error messages
 * - override print() in lua for yzis
 * - clear the lua stack properly
 * - arguments to :source must be passed as argv
 * - add missing function from vim
 *
 */

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#ifdef _MSC_VER
#pragma warning (disable : 4390)
#endif

#define RET_LUA( n )  \
	YZASSERT_EQUALS( lua_gettop(L), n ); \
	return n;

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

void print_lua_stack(lua_State *L, const char * msg="") {
	printf("stack - %s\n", msg );
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
   /* 
	* Flush the stack, by setting the top to 0, in order to
	* ignore any result given by the library load function.
	*/
	lua_settop(L, 0);

	luaopen_string( L );
	lua_settop(L, 0);
	luaopen_table( L );
	lua_settop(L, 0);
	luaopen_math( L );
	lua_settop(L, 0);
	luaopen_io( L );
	lua_settop(L, 0);
	luaopen_debug( L );
	lua_settop(L, 0);

	yzDebug() << lua_version() << " loaded" << endl;
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

	registerRegexp(L);
}

YZExLua::~YZExLua() {
	lua_close(L);
}

QString YZExLua::lua(YZView *, const QString& args) {
	execInLua( args );
	return QString::null;
}

//see Lua's PIL chapter 25.3 for how to use this :)
void YZExLua::exe(const QString& function, const char* sig, ...) {
    yzDebug() << "YZExLua::exe( " << function << " ) sig : " << sig << endl;
	va_list vl;
	int narg, nres;
	
	va_start(vl,sig);
	lua_getglobal(L, function.utf8());

	narg=0;
	while (*sig) {
		switch(*sig++) {
			case 'd' :
				lua_pushnumber(L, va_arg(vl, double));
				break;
			case 'i' :
				lua_pushnumber(L, va_arg(vl, int));
				break;
			case 's' :
				lua_pushstring(L, va_arg(vl, char*));
				break;
			case '>' :
				goto endwhile;
				break;
			default:
				//error
				break;
		}
		narg++;
		luaL_checkstack(L, 1, "too many arguments" );
	} endwhile:

	nres = strlen(sig);
	if (! yzpcall(narg,nres,0, QString("Executing function %1").arg(function))) {
		va_end(vl);
		return;
	}
	
	nres = -nres;
	while (*sig) {
		switch(*sig++) {
			case 'd' :
				if (!lua_isnumber(L,nres))
					;//error
				*va_arg(vl,double*) = lua_tonumber(L,nres);
				break;
			case 'i' :
				if (!lua_isnumber(L,nres))
					;//error
				*va_arg(vl,int*) = (int)lua_tonumber(L,nres);
				break;
			case 's' :
				if (!lua_isstring(L,nres))
					;//error
				*va_arg(vl,const char**) = lua_tostring(L,nres);
				break;
			default:
				//error
				break;
		}
		nres++;
	}
	va_end(vl);
}

void YZExLua::execute(const QString& function, int nbArgs, int nbResults) { 
	lua_getglobal(L,function);
	yzpcall(nbArgs, nbResults, 0, QString("YZExLua::execute function %1").arg(function)); 
}

//callers
QString YZExLua::source( YZView *v, const QString& args ) {
	return source(v,args,true);
}

QString YZExLua::source( YZView *, const QString& args, bool canPopup ) {
	yzDebug() << "source : " << args << endl;
	QString filename = args.left( args.find( " " ));
	if ( !filename.endsWith( ".lua" ) )
		filename += ".lua";
	filename = YZBuffer::tildeExpand( filename );
	yzDebug() << "looking filename : " << filename << endl;
	QStringList candidates;
	candidates << filename 
	           << QDir::currentDirPath()+"/"+filename
	           << QDir::homeDirPath()+"/.yzis/scripts/"+filename
	           << QDir::homeDirPath()+"/.yzis/scripts/indent/"+filename
		       << QString( PREFIX )+"/share/yzis/scripts/"+filename
		       << QString( PREFIX )+"/share/yzis/scripts/indent/"+filename;
	QString found;
	QStringList::iterator it = candidates.begin(), end = candidates.end();
	for( ; it!=end; ++it) {
		found = *it;
		if (QFile::exists( found )) break;

		if ((found.right(4) != ".lua")) {
			found += ".lua";
			if (QFile::exists(found)) break;
		}
		found = "";
	}

	if (found.isEmpty()) {
		if ( canPopup )
			YZSession::me->popupMessage(_("The file %1 could not be found in standard directories" ).arg( filename ));
		return QString::null;
	}

	lua_pushstring(L,"dofile");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L,found.latin1());
	yzpcall(1,1,0, _("Lua error when running file %1:\n").arg(found) );
	return QString::null;
}

int YZExLua::execInLua( const QString & luacode ) {
	lua_pushstring(L, "loadstring" );
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, luacode );
//	print_lua_stack(L, "loadstring 0");
	if (! yzpcall(1,2,0, QString("Executing following code in lua:\n%1").arg(luacode) )) {
		return 0;
	}
//	print_lua_stack(L, "loadstring 1");
	if (lua_isnil(L,-2) && lua_isstring(L,-1)) {
		QString luaErrorMsg = QString::fromUtf8( (  char * ) lua_tostring( L,-1 ) );
		lua_pop(L,2);
		YZSession::me->popupMessage(luaErrorMsg );
//		printf("execInLua - %s\n", luaErrorMsg.latin1() );
		return 0;
	} else if (lua_isfunction(L,-2)) {
		lua_pop(L,1);
		yzpcall(0,0,0, "");
	} else { // big errror
//		print_lua_stack(L, "loadstring returns strange things" );
		YZSession::me->popupMessage("Unknown lua return type");
	}
	return 0;
}

bool YZExLua::yzpcall( int nbArg, int nbReturn, int errLevel, const QString & errorMsg ) {
	int lua_err = lua_pcall(L,nbArg,nbReturn,errLevel);
	if (! lua_err) return true;
	QString luaErrorMsg = QString::fromUtf8( ( char * ) lua_tostring( L,lua_gettop( L ) ) );
	printf("pCall error: %s\n", luaErrorMsg.latin1() );
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

int YZExLua::line(lua_State *L) {
	if (!checkFunctionArguments(L, 1, 1, "line", "line")) return 0;
	int line = ( int )lua_tonumber( L,1 );
	lua_pop(L,1);

	line = line ? line - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	QString	t = cView->myBuffer()->textline( line );
	lua_pushstring( L, t ); // first result
	RET_LUA( 1 ); // one result
}

int YZExLua::setline(lua_State *L) {
	if (!checkFunctionArguments(L, 2, 2, "setline", "line, text")) return 0;
	int sLine = ( int )lua_tonumber( L,1 );
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	sLine = sLine ? sLine - 1 : 0;

	if (text.find("\n") != -1) {
		printf("setline with line containing \n");
		return 0;
	}
	YZView* cView = YZSession::me->currentView();
	cView->myBuffer()->action()->replaceLine(cView, sLine, text );
	return 0; // no result
}

int YZExLua::insert(lua_State *L) {
	if (!checkFunctionArguments(L, 3, 3, "insert", "line, col, text")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 3 ) );
	lua_pop(L,3);

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	QStringList list = QStringList::split( "\n", text );
	QStringList::Iterator it = list.begin(), end = list.end();
	for ( ; it != end; ++it ) {
		if ( ( unsigned int )sLine >= cView->myBuffer()->lineCount() ) cView->myBuffer()->action()->insertNewLine( cView, 0, sLine );
		cView->myBuffer()->action()->insertChar( cView, sCol, sLine, *it );
		sCol=0;
		sLine++;
	}

	RET_LUA( 0 ); // no result
}

int YZExLua::remove(lua_State *L) {
	if (!checkFunctionArguments(L, 3, 3, "remove", "line, col, nb")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	int sNb = ( int )lua_tonumber( L,3 );
	lua_pop(L,3);

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	YZBuffer * cBuffer = cView->myBuffer();
	YZAction * cAction = cBuffer->action();
	cAction->deleteChar(cView, sCol, sLine, sNb);

	RET_LUA( 0 ); // no result
}

int YZExLua::insertline(lua_State *L) {
	if (!checkFunctionArguments(L, 2, 2, "insertline", "line, text")) return 0;
	int sLine = ( int )lua_tonumber( L,1 );
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	lua_pop(L,2);

	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	QStringList list = QStringList::split( "\n", text );
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

	RET_LUA( 0 ); // no result
}

int YZExLua::appendline(lua_State *L) {
	if (!checkFunctionArguments(L, 1, 1, "appendline", "text")) return 0;
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZView* cView = YZSession::me->currentView();
	YZBuffer * cBuffer = cView->myBuffer();
	YZAction * cAction = cBuffer->action();
	QStringList list = QStringList::split( "\n", text );
	QStringList::Iterator it = list.begin(), end = list.end();
	for ( ; it != end; ++it ) {
		if (cBuffer->isEmpty()) {
			cAction->insertChar( cView, 0, 0, *it );
		} else {
			cAction->insertLine( cView, cBuffer->lineCount(), *it );
		}
	}

	RET_LUA( 0 ); // no result
}

int YZExLua::replace(lua_State *L) {
	if (!checkFunctionArguments(L, 3, 3, "replace", "line, col, text")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 3 ) );
	lua_pop(L,3);

	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	if (text.find('\n') != -1) {
		// replace does not accept multiline strings, it is too strange
		RET_LUA( 0 );
	}

	YZView* cView = YZSession::me->currentView();
	if ( ( unsigned int )sLine >= cView->myBuffer()->lineCount() ) {
		cView->myBuffer()->action()->insertNewLine( cView, 0, sLine );
		sCol = 0;
	}
	cView->myBuffer()->action()->replaceChar( cView, sCol, sLine, text );

	RET_LUA( 0 ); // no result
}

int YZExLua::winline(lua_State *L) {
	if (!checkFunctionArguments(L, 0, 0, "winline", "")) return 0;
	YZView* cView = YZSession::me->currentView();
	uint result = cView->getBufferCursor()->y() + 1;

	lua_pushnumber( L, result ); // first result
	RET_LUA( 1 ); // one result
}

int YZExLua::wincol(lua_State *L) {
	if (!checkFunctionArguments(L, 0, 0, "wincol", "")) return 0;
	YZView* cView = YZSession::me->currentView();
	uint result = cView->getBufferCursor()->x() + 1;

	lua_pushnumber( L, result ); // first result
	RET_LUA( 1 ); // one result
}

int YZExLua::scrline(lua_State *L) {
	if (!checkFunctionArguments(L, 0, 0, "scrline", "")) return 0;
	YZView* cView = YZSession::me->currentView();
	uint result = cView->getCursor()->y() + 1;

	lua_pushnumber( L, result ); // first result
	RET_LUA( 1 ); // one result
}

int YZExLua::scrcol(lua_State *L) {
	if (!checkFunctionArguments(L, 0, 0, "scrcol", "")) return 0;
	YZView* cView = YZSession::me->currentView();
	uint result = cView->getCursor()->x() + 1;

	lua_pushnumber( L, result ); // first result
	RET_LUA( 1 ); // one result
}

int YZExLua::winpos(lua_State *L) {
	if (!checkFunctionArguments(L, 0, 0, "winpos", "")) return 0;
	YZView* cView = YZSession::me->currentView();
	uint line = cView->getBufferCursor()->y() + 1;
	uint col = cView->getBufferCursor()->x() + 1;
	lua_pushnumber( L, col ); 
	lua_pushnumber( L, line ); 
	RET_LUA( 2 );
}

int YZExLua::_goto(lua_State *L) {
	if (!checkFunctionArguments(L, 2, 2, "goto", "line, col")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	lua_pop(L,2);

	YZView* cView = YZSession::me->currentView();
	cView->gotoxy(sCol ? sCol - 1 : 0, sLine ? sLine - 1 : 0 );

	RET_LUA( 0 ); // one result
}

int YZExLua::scrgoto(lua_State *L) {
	if (!checkFunctionArguments(L, 2, 2, "scrgoto", "line, col")) return 0;
	int sCol = ( int )lua_tonumber( L, 1 );
	int sLine = ( int )lua_tonumber( L,2 );
	lua_pop(L,2);

	YZView* cView = YZSession::me->currentView();
	cView->gotodxdy(sCol ? sCol - 1 : 0, sLine ? sLine - 1 : 0 );

	RET_LUA( 0 ); // one result
}

int YZExLua::deleteline(lua_State *L) {
	if (!checkFunctionArguments(L, 1, 1, "deleteline", "line")) return 0;
	int sLine = ( int )lua_tonumber( L,1 );
	lua_pop(L,1);

	YZView* cView = YZSession::me->currentView();
	QValueList<QChar> regs;
	regs << QChar( '"' ) ;
	cView->myBuffer()->action()->deleteLine( cView, sLine ? sLine - 1 : 0, 1, regs );

	RET_LUA( 0 ); // one result
}

int YZExLua::filename(lua_State *L) {
	if (!checkFunctionArguments(L, 0, 0, "filename", "")) return 0;
	YZView* cView = YZSession::me->currentView();
	const char *filename = cView->myBuffer()->fileName();

	lua_pushstring( L, filename ); // first result
	RET_LUA( 1 ); // one result
}

int YZExLua::color(lua_State *L) {
	if (!checkFunctionArguments(L, 2, 2, "color", "line, col")) return 0;
	int sCol = ( int )lua_tonumber( L,1 );
	int sLine = ( int )lua_tonumber( L,2 );
	lua_pop(L,2);
	sCol = sCol ? sCol - 1 : 0;
	sLine = sLine ? sLine - 1 : 0;

	YZView* cView = YZSession::me->currentView();
	const char *color = cView->drawColor( sCol, sLine ).name();

//	yzDebug() << "Asked color : " << color.latin1() << endl;
	lua_pushstring( L, color ); // first result

	RET_LUA( 1 ); // one result
}

int YZExLua::linecount(lua_State *L) {
	if (!checkFunctionArguments(L, 0, 0, "linecount", "")) return 0;
	YZView* cView = YZSession::me->currentView();
	lua_pushnumber( L, cView->myBuffer()->lineCount()); // first result
	RET_LUA( 1 ); // one result
}

int YZExLua::version( lua_State *L ) {
	if (!checkFunctionArguments(L, 0, 0, "version", "")) return 0;
	lua_pushstring( L, VERSION_CHAR );
	RET_LUA( 1 );
}

int YZExLua::sendkeys( lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "sendkeys", "text")) return 0;
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);
	YZSession::me->sendMultipleKeys(text);
	// nothing to return
	RET_LUA( 0 );
}

int YZExLua::highlight( lua_State *L ) {
	int n = lua_gettop( L );
	if ( n < 3 ) return 0;
	QStringList arg;
	for ( int i = 1; i <= n ; i++ ) {
		arg << ( char * )lua_tostring( L, i );
	}
	YZExCommandArgs args(NULL,QString::null,QString::null,arg.join(" "),0,0,true);
	YZSession::me->getExPool()->highlight(args);
	RET_LUA( 0 );
}

int YZExLua::yzdebug( lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "yzdebug", "text")) return 0;
	QString text = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	yzDebug() << "Lua debug : " << text << endl;	
	RET_LUA( 0 );
}

int YZExLua::yzprint(lua_State * /*L*/) {
	// fetch string from the stack
	// print it
	return 0;
}

int YZExLua::connect(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "connect", "event (string), function (string)")) return 0;
	QString event = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString function = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );

	YZSession::me->eventConnect(event,function);
	
	RET_LUA( 0 );
}

int YZExLua::source(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "source", "filename")) return 0;
	QString filename = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );

	YZExLua::instance()->source(NULL, filename);

	RET_LUA( 0 );
}

int YZExLua::setlocal(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "setlocal", "option name")) return 0;
	QString option = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );

	YZExCommandArgs ex (YZSession::me->currentView(), QString::null, "setlocal", option, 0, 0, true);
	YZSession::me->getExPool()->set(ex);

	RET_LUA( 0 );	
}

int YZExLua::newoption(lua_State *L ) {
	if (!checkFunctionArguments(L, 6, 6, "newoption", "option name, group name, default value, value, visibility (number), type (number)")) return 0;
	QString option = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString group = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );
	QString defaultvalue = QString::fromUtf8( (  char * )lua_tostring (  L, 3 ) );
	QString value = QString::fromUtf8( (  char * )lua_tostring (  L, 4 ) );
	context_t visibility = (context_t)(int)lua_tonumber ( L, 5 );
	value_t type = (value_t)(int)lua_tonumber ( L, 6 );

	YZSession::me->getOptions()->createOption(option, group, defaultvalue, value, visibility, type );

	RET_LUA( 0 );
}

int YZExLua::map(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "map", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring ( L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );

	YZMapping::self()->addGlobalMapping(key, mapp);

	RET_LUA( 0 );
}

int YZExLua::unmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "unmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	YZMapping::self()->deleteGlobalMapping(key);
	RET_LUA( 0 );
}

int YZExLua::imap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "imap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );

	YZMapping::self()->addInsertMapping(key, mapp);

	RET_LUA( 0 );
}

int YZExLua::iunmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "iunmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	YZMapping::self()->deleteInsertMapping(key);
	RET_LUA( 0 );
}

int YZExLua::omap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "omap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );

	YZMapping::self()->addPendingOpMapping(key, mapp);
	RET_LUA( 0 );
}

int YZExLua::ounmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "ounmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	YZMapping::self()->deletePendingOpMapping(key);
	RET_LUA( 0 );
}

int YZExLua::vmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "vmap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );

	YZMapping::self()->addVisualMapping(key, mapp);
	RET_LUA( 0 );
}

int YZExLua::vunmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "vunmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	YZMapping::self()->deleteVisualMapping(key);
	RET_LUA( 0 );
}

int YZExLua::cmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "cmap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );

	YZMapping::self()->addCmdLineMapping(key, mapp);
	RET_LUA( 0 );
}

int YZExLua::cunmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "cunmap", "key")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	YZMapping::self()->deleteCmdLineMapping(key);
	RET_LUA( 0 );
}

int YZExLua::nmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "nmap", "key, text")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	QString mapp = QString::fromUtf8( (  char * )lua_tostring (  L, 2 ) );

	YZMapping::self()->addNormalMapping(key, mapp);
	RET_LUA( 0 );
}

int YZExLua::nunmap(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "nunmap", "key (string)")) return 0;
	QString key = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	YZMapping::self()->deleteNormalMapping(key);
	RET_LUA( 0 );
}

int YZExLua::noremap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "noremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	
	YZMapping::self()->addGlobalNoreMapping(key, mapp);
	
	RET_LUA( 0 );
}

int YZExLua::nnoremap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "nnoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	
	YZMapping::self()->addNormalNoreMapping(key, mapp);
	
	RET_LUA( 0 );
}

int YZExLua::vnoremap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "vnoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	
	YZMapping::self()->addVisualNoreMapping(key, mapp);
	
	RET_LUA( 0 );
}

int YZExLua::onoremap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "onoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	
	YZMapping::self()->addPendingOpNoreMapping(key, mapp);
	
	RET_LUA( 0 );
}

int YZExLua::inoremap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "inoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	
	YZMapping::self()->addInsertNoreMapping(key, mapp);
	
	RET_LUA( 0 );
}

int YZExLua::cnoremap(lua_State *L ) {
	if (!checkFunctionArguments(L, 2, 2, "cnoremap", "key, text")) return 0;
	QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
	QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
	
	YZMapping::self()->addCmdLineNoreMapping(key, mapp);
	
	RET_LUA( 0 );
}

int YZExLua::matchpair(lua_State *L ) {
	if (!checkFunctionArguments(L, 0, 0, "matchpair", "")) return 0;
	bool found = false;
	YZView *v = YZSession::me->currentView();
	YZCursor s (v->getBufferCursor());
	YZCursor c = v->myBuffer()->action()->match(v, s, &found);
	lua_pushboolean(L , found);
	lua_pushnumber(L, c.x());
	lua_pushnumber(L, c.y());
	RET_LUA( 3 );
}

int YZExLua::mode(lua_State *L ) {
	if (!checkFunctionArguments(L, 0, 0, "mode", "")) return 0;
	YZView *v = YZSession::me->currentView();
	QString mode = v->mode();
	lua_pushstring(L,mode.latin1());
	RET_LUA( 1 );
}

int YZExLua::set(lua_State *L ) {
	if (!checkFunctionArguments(L, 1, 1, "set", "option (string)")) return 0;
	QString option = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );
	lua_pop(L,1);

	YZSession::me->getExPool()->set(YZExCommandArgs(YZSession::me->currentView(), QString::null, QString::null, option, 0, 0, true));

	RET_LUA( 0 );	
}

bool YZExLua::checkFunctionArguments(lua_State*L, int argNbMin, int argNbMax, const char * functionName, const char * functionArgDesc ) {
	int n = lua_gettop( L );
	if (n >= argNbMin && n <= argNbMax) return true;

	QString errorMsg = QString("%1() called with %2 arguments but %3-%4 expected: %5").arg(functionName).arg(n).arg(argNbMin).arg(argNbMax).arg(functionArgDesc);
#if 1
	lua_pushstring(L,errorMsg.latin1());
	lua_error(L);
#else
	YZExLua::instance()->execInLua(QString("error(%1)").arg(errorMsg));
#endif
	return false;
}

QStringList YZExLua::getLastResult(int nb) {
	int n = lua_gettop( L );
	yzDebug() << "LUA: Stack has " << n << " entries" << endl;
	QStringList list;
	for (int i = - nb ; i < 0 ; ++i ) {
		int type = lua_type(L,i);
		yzDebug() << "Type for index " << i << " : " << type << endl;
		switch (type) {
			case LUA_TNUMBER:
				list << QString::number(lua_tonumber(L,i));
				break;
			case LUA_TBOOLEAN:
				list << QString((lua_toboolean(L,i)!=0) ? "true" : "false");
				break;
			case LUA_TSTRING:
				list << QString::fromUtf8((char*)lua_tostring(L,i));
				break;
			default:
				break;
		}
		//cleanup
		lua_pop(L,1);
	}
	yzDebug() << "LUA: Result " << list << endl;
	return list;
}

void YZExLua::registerRegexp(lua_State * L)
{
	lua_register(L,"Regexp_create", Regexp_create);
	lua_register(L,"Regexp_matchIndex", Regexp_matchIndex);
	lua_register(L,"Regexp_match", Regexp_match);
	lua_register(L,"Regexp_setCaseSensitive", Regexp_setCaseSensitive);
	lua_register(L,"Regexp_setMinimal", Regexp_setMinimal);
	lua_register(L,"Regexp_pos", Regexp_pos);
	lua_register(L,"Regexp_numCaptures", Regexp_numCaptures);
	lua_register(L,"Regexp_captured", Regexp_captured);
	lua_register(L,"Regexp_replace", Regexp_replace);
	lua_register(L,"Regexp_pattern", Regexp_pattern);
	lua_register(L,"Regexp_userdata_finalize", Regexp_userdata_finalize);

	QString regexpLuaCode = ""
"Regexp = { 					\n"
"    setCaseSensitive = Regexp_setCaseSensitive		\n"
"    ,setMinimal = Regexp_setMinimal		\n"
"    ,match = Regexp_match		\n"
"    ,replace = Regexp_replace		\n"
"    ,pattern = Regexp_pattern		\n"
"    ,matchIndex = Regexp_matchIndex		\n"
"	 ,pos = Regexp_pos			\n"
"	 ,numCaptures = Regexp_numCaptures	\n"
"	 ,captured = Regexp_captured			\n"
"}								\n"

"Regexp_Class_mt = { 			\n"
"    __call  = Regexp_create	\n"
"}								\n"
" 								\n"
"setmetatable( Regexp, Regexp_Class_mt )	\n"
" 								\n"
"Regexp_Object_mt = { 			\n"
"    __index = Regexp,			\n"
"}								\n"
" 								\n"
" 								\n"
" 								\n"
;
	execInLua( regexpLuaCode );
}

int YZExLua::Regexp_create(lua_State *L)
{
	if (! checkFunctionArguments(L, 2, 2, "Regexp.create", "Regexp table, pattern")) return 0;
	// discard the Regexp table
	lua_remove(L, 1);
	QString re = QString::fromUtf8( (  char * )lua_tostring (  L, 1 ) );

	// create table
	lua_newtable( L );
	// stack: table

		// store ud
	lua_pushstring( L, "qregexp*" );
	// stack: table, "qregexp*"
	QRegExp **pRegExp = (QRegExp **) lua_newuserdata(L, sizeof( QRegExp * ) ); // store the pointer as userdata
	*pRegExp = new QRegExp(re);
	// stack: table, "qregexp*", userdata

	// create userdata metatable and fill it
	lua_newtable( L ); 
	// stack: table, "qregexp*", userdata, table
	lua_pushstring( L, "__gc" ); 
	// stack: table, "qregexp*", ud, table, "__gc"
	lua_pushstring(L,"Regexp_userdata_finalize"); 
	// stack: table, "qregexp*",ud, table, "__gc", "Regexp_userdata_finalize"
	lua_gettable(L, LUA_GLOBALSINDEX);
	// stack: table, "qregexp*",ud, table, "__gc", function
	lua_rawset(L, -3 );
	// stack: table, "qregexp*",ud, table
	lua_setmetatable(L, -2);
	// stack: table, "qregexp*",ud
	lua_rawset(L, -3 );
	// stack: table

	// set Regexp_mt as metatable
	lua_pushstring(L, "Regexp_Object_mt" );
	// stack: table, "Regexp_mt"
	lua_gettable( L, LUA_GLOBALSINDEX );
	// stack: table, table Regexp_mt
	lua_setmetatable(L, -2);
	// stack: table

	return 1;
}

int YZExLua::Regexp_userdata_finalize(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 1, 1, "Regexp.finalize", "Regexp object")) return 0;

	QRegExp ** pRegexp = (QRegExp **) lua_touserdata(L, -1);
	QRegExp * regexp = *pRegexp;
	lua_pop(L, 1);

	//	printf("Finalizing %s\n", pRegexp->pattern().latin1() );

	delete regexp;
	*pRegexp = NULL;
	
	RET_LUA( 0 );
}

int YZExLua::Regexp_match(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 2, 2, "Regexp.match", "Regexp object, string")) return 0;

	QString s = QString::fromUtf8( (  char * ) lua_tostring (  L, 2 ) );

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));

	lua_pop(L, 3);

	lua_pushboolean( L, regexp->search( s ) != -1);

	RET_LUA( 1 );
}


int YZExLua::Regexp_matchIndex(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 2, 2, "Regexp.matchIndex", "Regexp object, string")) return 0;

	QString s = QString::fromUtf8( (  char * ) lua_tostring (  L, 2 ) );

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 3 );

	lua_pushnumber( L, regexp->search( s ) );
	RET_LUA( 1 );
}


int YZExLua::Regexp_setMinimal(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 2, 2, "Regexp.setMinimal", "Regexp object, boolean")) return 0;

	bool b = lua_toboolean ( L, 2 );

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 3);

	regexp->setMinimal( b );
	RET_LUA( 0 );
}


int YZExLua::Regexp_setCaseSensitive(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 2, 2, "Regexp.setCaseSensitive", "Regexp object, boolean")) return 0;

	bool b = lua_toboolean ( L, 2 );

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 3);

	regexp->setCaseSensitive( b );
	RET_LUA( 0 );
}


int YZExLua::Regexp_pos(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 2, 2, "Regexp.pos", "Regexp object, index")) return 0;

	int index = lua_tonumber( L, 2 );

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 3);

	lua_pushnumber( L, regexp->pos( index ) );
	RET_LUA( 1 );
}

int YZExLua::Regexp_numCaptures(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 1, 1, "Regexp.numCaptures", "Regexp object")) return 0;

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 2);

	lua_pushnumber( L, regexp->numCaptures() );
	RET_LUA( 1 );
}

int YZExLua::Regexp_captured(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 2, 2, "Regexp.captured", "Regexp object, index")) return 0;

	int index = lua_tonumber( L, 2 );

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 3);

	lua_pushstring( L, regexp->cap( index ) );
	RET_LUA( 1 );
}

int YZExLua::Regexp_replace(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 3, 4, "Regexp.replace", "Regexp object, string, string, number (optional)")) return 0;

	QString s  = lua_tostring( L, 2 );
	QString replacement  = lua_tostring( L, 3 );

	int nb = -1;
	if ( lua_gettop(L) >= 4) {
		nb = lua_tonumber(L, 4);
		lua_pop(L, 1);
	}
	
	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 4);


	QString sRet = "", matched;
	int matchLen=0, matchIdx, idx=0;

	while( nb == -1 || nb-- > 0) {
		// look for regexp inside s starting at index idx
		if (regexp->search(s, idx) == -1) break;
		// if regexp match, we have an index and a length
		matched = regexp->cap( 0 );
		matchIdx = regexp->pos( 0 );
		matchLen = matched.length();
		sRet += s.mid( idx, matchIdx-idx );
		sRet += matched.replace( *regexp, replacement );
		idx = matchIdx + matchLen;
	}

	sRet += s.mid( idx );

	lua_pushstring( L, sRet.latin1() );
	RET_LUA( 1 );
}

int YZExLua::Regexp_pattern(lua_State *L)
{
	if (! YZExLua::checkFunctionArguments(L, 1, 1, "Regexp.pattern", "Regexp object")) return 0;

	// extract userdata from table Regexp
	lua_pushstring(L, "qregexp*" );
	lua_gettable( L, 1);
	QRegExp * regexp = *((QRegExp **) lua_touserdata(L, -1));
	lua_pop(L, 2);

	lua_pushstring( L, regexp->pattern().latin1() );
	RET_LUA( 1 );
}
