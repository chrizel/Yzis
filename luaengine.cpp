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

#include "luaengine.h"
#include "luafuncs.h"
#include "luaregexp.h"
#include "debug.h"
#include "buffer.h"
#include "session.h"
#include "yzis.h"
#include "portability.h"

#include <QDir>


class YZView;

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

void YZLuaEngine::print_lua_stack_value(lua_State*L, int index) {
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

void YZLuaEngine::print_lua_stack(lua_State *L, const char * msg) {
	printf("stack - %s\n", msg );
	for(int i=1; i<=lua_gettop(L); i++) {
		print_lua_stack_value(L,i);
	}
}

YZLuaEngine * YZLuaEngine::me = 0;

YZLuaEngine * YZLuaEngine::self() {
	if (YZLuaEngine::me == 0) { 
		YZLuaEngine::me = new YZLuaEngine();
		YZLuaEngine::self()->init();
	}
	return YZLuaEngine::me;
}

YZLuaEngine::YZLuaEngine() {
	L = lua_open();
	luaopen_base(L);
	luaopen_string( L );
	luaopen_table( L );
	luaopen_math( L );
	luaopen_io( L );
	luaopen_debug( L );
	yzDebug() << LUA_VERSION << " loaded" << endl;
}

void YZLuaEngine::init() {
	YZLuaFuncs::registerLuaFuncs( L );
	YZLuaRegexp::registerLuaRegexp( L );
}

YZLuaEngine::~YZLuaEngine() {
	lua_close(L);
}

void YZLuaEngine::cleanLuaStack( lua_State * L ) {
	lua_pop(L,lua_gettop(L));
}

QString YZLuaEngine::lua(YZView *, const QString& args) {
	execInLua( args );
	return QString::null;
}

//see Lua's PIL chapter 25.3 for how to use this :)
void YZLuaEngine::exe(const QString& function, const char* sig, ...) {
    yzDebug() << "YZLuaEngine::exe( " << function << " ) sig : " << sig << endl;
	va_list vl;
	int narg, nres;
	
	va_start(vl,sig);
	lua_getglobal(L, function.toUtf8());

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
	if (! yzpcall(narg,nres, QString("Executing function %1").arg(function))) {
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

void YZLuaEngine::execute(const QString& function, int nbArgs, int nbResults) { 
	lua_getglobal(L,function.toUtf8());
	yzpcall(nbArgs, nbResults, QString("YZLuaEngine::execute function %1").arg(function)); 
}

QString YZLuaEngine::source( const QString& filename ) {
    QString fname = filename;
	if ( !fname.endsWith( ".lua" ) ) fname += ".lua";

	yzDebug() << "source : " << fname << endl;
	fname = YZBuffer::tildeExpand( fname );
	yzDebug() << "looking filename : " << fname << endl;
	QStringList candidates;
	candidates << fname 
	           << QDir::currentPath()+"/"+fname
	           << QDir::homePath()+"/.yzis/scripts/"+fname
	           << QDir::homePath()+"/.yzis/scripts/indent/"+fname
		       << QString( PREFIX )+"/share/yzis/scripts/"+fname
		       << QString( PREFIX )+"/share/yzis/scripts/indent/"+fname;

	QString found;
	QStringList::iterator it = candidates.begin(), end = candidates.end();
	for( ; it!=end; ++it) {
		if (QFile::exists( *it )) {
            found = *it;
            break;
        }
	}

	if (found.isEmpty()) {
        YZSession::me->popupMessage(_("The file %1 could not be found in standard directories" ).arg( filename ));
		return QString::null;
	}

	lua_pushstring(L,"dofile");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L,found.toUtf8());
	yzpcall(1,1, _("Lua error when running file %1:\n").arg(found) );
	cleanLuaStack( L ); // in case sourcing the file left something on the stack
	return QString::null;
}

int YZLuaEngine::execInLua( const QString & luacode ) {
	lua_pushstring(L, "loadstring" );
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_pushstring(L, luacode.toUtf8() );
//	print_lua_stack(L, "loadstring 0");
	if (yzpcall(1,2, QString("Executing following code in lua:\n%1").arg(luacode) )) {
        // Call was successful
		return 0;
	}

    return 1;
}

bool YZLuaEngine::yzpcall( int nbArg, int nbReturn, const QString & context ) {
	int lua_err = lua_pcall(L,nbArg,nbReturn, 0);
    QString luaErrorMsg;

	if (! lua_err) return true; // call is successful

	if (lua_isstring(L,-1)) {
        // an error message on the stack
        luaErrorMsg = QString::fromUtf8( ( char * ) lua_tostring( L,lua_gettop( L ) ) );
	} else if (lua_isfunction(L,-2)) {
        // error handler function on the stack at position -2
		lua_pop(L,1);
		yzpcall(0,0, "error handling function called from within yzpcall");
	} else { 
        // big error, we do not grok what happend
		print_lua_stack(L, "loadstring returns strange things" );
        luaErrorMsg = "Unknown lua return type after loadstring";
	}

    QByteArray err = luaErrorMsg.toLatin1();
    printf("pCall error: %s\n", err.data() );

	YZSession::me->popupMessage(context + "\n" + luaErrorMsg );
	return false;
}

void YZLuaEngine::yzisprint(const QString & text) {
	QByteArray tmp = text.toUtf8();
	printf("yzisprint:%s\n", tmp.data());
}

bool YZLuaEngine::checkFunctionArguments(lua_State*L, int argNbMin, int argNbMax, const char * functionName, const char * functionArgDesc ) {
	int n = lua_gettop( L );
	if (n >= argNbMin && n <= argNbMax) return true;

	QString errorMsg = QString("%1() called with %2 arguments but %3-%4 expected: %5").arg(functionName).arg(n).arg(argNbMin).arg(argNbMax).arg(functionArgDesc);
#if 1
	QByteArray e = errorMsg.toUtf8();
	lua_pushstring(L,e.data());
	lua_error(L);
#else
	YZLuaEngine::self()->execInLua(QString("error(%1)").arg(errorMsg));
#endif
	return false;
}

QStringList YZLuaEngine::getLastResult(int nb) const {
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

