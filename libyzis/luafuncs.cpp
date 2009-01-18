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
#include "internal_options.h"
#include "events.h"
#include "resourcemgr.h"

#include "mode_ex.h"

#include "luaengine.h"

#define dbg()    yzDebug("YLuaFuncs")
#define err()    yzError("YLuaFuncs")

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

extern "C"
{
#include <lauxlib.h>
#include <lualib.h>
}

using namespace yzis;


// ========================================================
//
//                     Lua Commands
//
// ========================================================

void YLuaFuncs::registerLuaFuncs(lua_State *L)
{
    lua_register(L, "line", line);
    lua_register(L, "setline", setline);
    lua_register(L, "insert", insert);
    lua_register(L, "remove", remove);
    lua_register(L, "insertline", insertline);
    lua_register(L, "appendline", appendline);
    lua_register(L, "replace", replace);
    lua_register(L, "wincol", wincol);
    lua_register(L, "winline", winline);
    lua_register(L, "winpos", winpos);
    lua_register(L, "goto", _goto);
    lua_register(L, "scrcol", scrcol);
    lua_register(L, "screenwidth", scrscreenwidth);
    lua_register(L, "scrline", scrline);
    lua_register(L, "scrgoto", scrgoto);
    lua_register(L, "deleteline", deleteline);
    lua_register(L, "version", version);
    lua_register(L, "filename", filename);
    lua_register(L, "color", color);
    lua_register(L, "linecount", linecount);
    lua_register(L, "sendkeys", sendkeys);
    lua_register(L, "highlight", highlight);
    lua_register(L, "connect", connect);
    lua_register(L, "source", source);
    lua_register(L, "yzdebug", yzdebug);
    lua_register(L, "setlocal", setlocal);
    lua_register(L, "newoption", newoption);
    lua_register(L, "set", set);
    lua_register(L, "map", map);
    lua_register(L, "unmap", unmap);
    lua_register(L, "imap", imap);
    lua_register(L, "iunmap", iunmap);
    lua_register(L, "nmap", nmap);
    lua_register(L, "nunmap", nunmap);
    lua_register(L, "omap", omap);
    lua_register(L, "ounmap", ounmap);
    lua_register(L, "vmap", vmap);
    lua_register(L, "vunmap", vunmap);
    lua_register(L, "cmap", cmap);
    lua_register(L, "cunmap", cunmap);
    lua_register(L, "noremap", noremap);
    lua_register(L, "nnoremap", nnoremap);
    lua_register(L, "vnoremap", vnoremap);
    lua_register(L, "onoremap", onoremap);
    lua_register(L, "inoremap", inoremap);
    lua_register(L, "cnoremap", cnoremap);
    lua_register(L, "matchpair", matchpair);
    lua_register(L, "mode", mode);
    lua_register(L, "edit", edit);
    lua_register(L, "loadplugin", loadplugin);
    lua_register(L, "setLuaReturnValue", setLuaReturnValue);
    dbg() << HERE() << " done." << endl;
}


int YLuaFuncs::line(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "line", "line")) return 0;
    int line = ( int )lua_tonumber( L, 1 );
    lua_pop(L, 1);

    line = line ? line - 1 : 0;

    YView* cView = YSession::self()->currentView();
    QString t = cView->buffer()->textline( line );

    lua_pushstring( L, t.toUtf8() ); // first result
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ;
}

int YLuaFuncs::setline(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "setline", "line, text")) return 0;
    int sLine = ( int )lua_tonumber( L, 1 );
    QString text = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    sLine = sLine ? sLine - 1 : 0;
    if (text.indexOf("\n") != -1) {
        printf("setline with line containing \n");
        YASSERT_EQUALS( lua_gettop(L), 0 );
        return 0;
    }
    YView* cView = YSession::self()->currentView();
	if ( sLine >= cView->buffer()->lineCount() ) {
		// replace only existing lines
		return 0;
	}
    cView->buffer()->action()->replaceLine(cView, sLine, text );

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0; // no result
}

int YLuaFuncs::insert(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 3, 3, "insert", "line, col, text")) return 0;
    int sCol = ( int )lua_tonumber( L, 1 );
    int sLine = ( int )lua_tonumber( L, 2 );
    QString text = QString::fromUtf8( ( char * )lua_tostring ( L, 3 ) );
    lua_pop(L, 3);

    sCol = sCol ? sCol - 1 : 0;
    sLine = sLine ? sLine - 1 : 0;

    YView* cView = YSession::self()->currentView();

	if ( sLine > cView->buffer()->lineCount() ) {
		// do not accept creating lines beyond the last
		return 0;
	}

    QStringList list = text.split( "\n" );
    QStringList::Iterator it = list.begin(), end = list.end();
    for ( ; it != end; ++it ) {
		cView->buffer()->action()->ensureLineExists(sLine);
        cView->buffer()->action()->insertChar( cView, sCol, sLine, *it );
        sCol = 0;
        sLine++;
    }

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ; // no result
}

int YLuaFuncs::remove(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 3, 3, "remove", "line, col, nb")) return 0;
    int sCol = ( int )lua_tonumber( L, 1 );
    int sLine = ( int )lua_tonumber( L, 2 );
    int sNb = ( int )lua_tonumber( L, 3 );
    lua_pop(L, 3);

    sCol = sCol ? sCol - 1 : 0;
    sLine = sLine ? sLine - 1 : 0;

    YView* cView = YSession::self()->currentView();
    YBuffer * cBuffer = cView->buffer();
    YZAction * cAction = cBuffer->action();
    cAction->deleteChar(cView, sCol, sLine, sNb);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ; // no result
}

int YLuaFuncs::insertline(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "insertline", "line, text")) return 0;
    int sLine = ( int )lua_tonumber( L, 1 );
    QString text = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    sLine = sLine ? sLine - 1 : 0;
	

    YView* cView = YSession::self()->currentView();
	YBuffer* cBuffer = cView->buffer();
	if ( sLine > cBuffer->lineCount() ) {
		// do not accept creating lines beyond the last
		return 0;
	}
	YZAction* cAction = cBuffer->action();
    QStringList list = text.split( "\n" );
    QStringList::Iterator it = list.begin(), end = list.end();
    for ( ; it != end; ++it ) {
		if ( sLine >= cBuffer->lineCount() ) {
			cAction->ensureLineExists(sLine);
		} else if ( !cBuffer->isEmpty() ) {
			cAction->insertNewLine(cView, YCursor(0,sLine));
		}
        cAction->insertChar( cView, 0, sLine, *it );
        sLine++;
    }

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ; // no result
}

int YLuaFuncs::appendline(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "appendline", "text")) return 0;
    QString text = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YView* cView = YSession::self()->currentView();
    YBuffer * cBuffer = cView->buffer();
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

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ; // no result
}

int YLuaFuncs::replace(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 3, 3, "replace", "line, col, text")) return 0;
    int sCol = ( int )lua_tonumber( L, 1 );
    int sLine = ( int )lua_tonumber( L, 2 );
    QString text = QString::fromUtf8( ( char * )lua_tostring ( L, 3 ) );
    lua_pop(L, 3);

    sCol = sCol ? sCol - 1 : 0;
    sLine = sLine ? sLine - 1 : 0;

    if (text.indexOf('\n') != -1) {
        // replace does not accept multiline strings, it is too strange
        // XXX raises an error
        YASSERT_EQUALS( lua_gettop(L), 0 );
        return 0 ;
    }

    YView* cView = YSession::self()->currentView();
	YBuffer* cBuffer = cView->buffer();
	if ( sLine > cBuffer->lineCount() ) {
		// do not accept creating lines beyond the last
		return 0;
	}
	if ( sLine > 0 && sLine >= cBuffer->lineCount() ) {
		cBuffer->action()->ensureLineExists(sLine);
		sCol = 0;
	}
    cView->buffer()->action()->replaceChar( cView, sCol, sLine, text );

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ; // no result
}

int YLuaFuncs::winline(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "winline", "")) return 0;

    YView* cView = YSession::self()->currentView();
    uint result = cView->getLineColumnCursor().y() + 1;

    lua_pushnumber( L, result ); // first result
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ; // one result
}

int YLuaFuncs::wincol(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "wincol", "")) return 0;
    YView* cView = YSession::self()->currentView();
    uint result = cView->currentPosition() + 1;

    lua_pushnumber( L, result ); // first result
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ; // one result
}

int YLuaFuncs::scrline(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "scrline", "")) return 0;
    YView* cView = YSession::self()->currentView();
    uint result = cView->getRowColumnCursor().y() + 1;

    lua_pushnumber( L, result ); // first result
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ; // one result
}

int YLuaFuncs::scrcol(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "scrcol", "")) return 0;
    YView* cView = YSession::self()->currentView();
    uint result = cView->getRowColumnCursor().x() + 1;

    lua_pushnumber( L, result ); // first result
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ; // one result
}

int YLuaFuncs::winpos(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "winpos", "")) return 0;

    YView* cView = YSession::self()->currentView();
    uint line = cView->getLineColumnCursor().y() + 1;
    uint col = cView->getLineColumnCursor().x() + 1;

    lua_pushnumber( L, col );
    lua_pushnumber( L, line );
    YASSERT_EQUALS( lua_gettop(L), 2 );
    return 2 ; // two results
}

int YLuaFuncs::_goto(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "goto", "line, col")) return 0;
    int sCol = ( int )lua_tonumber( L, 1 );
    int sLine = ( int )lua_tonumber( L, 2 );
    lua_pop(L, 2);

    YView* cView = YSession::self()->currentView();
	sLine = qMin(qMax(sLine-1,0), cView->buffer()->lineCount()-1);
    cView->gotoLinePosition(sLine, sCol ? sCol - 1 : 0);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::scrgoto(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "scrgoto", "line, col")) return 0;
    int sCol = ( int )lua_tonumber( L, 1 );
    int sLine = ( int )lua_tonumber( L, 2 );
    lua_pop(L, 2);

    YView* cView = YSession::self()->currentView();
    cView->gotoRowColumn( sLine ? sLine - 1 : 0, sCol ? sCol - 1 : 0 );

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}
int YLuaFuncs::scrscreenwidth(lua_State *L)
{
    if(!YLuaEngine::checkFunctionArguments(L, 0, 0, "scrscreenwidth", "")) return 0;
    YView *cView = YSession::self()->currentView();
    uint column = cView->getColumnsVisible();
    lua_pushnumber( L, column);
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1;
}

int YLuaFuncs::deleteline(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "deleteline", "line")) return 0;
    int sLine = ( int )lua_tonumber( L, 1 );
    lua_pop(L, 1);

    YView* cView = YSession::self()->currentView();
    QList<QChar> regs;
    regs << QChar( '"' ) ;
    cView->buffer()->action()->deleteLine( cView, sLine ? sLine - 1 : 0, 1, regs );

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::filename(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "filename", "")) return 0;
    YView* cView = YSession::self()->currentView();
    QByteArray fn = cView->buffer()->fileName().toUtf8();
    const char *filename = fn.data();

    lua_pushstring( L, filename );
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ;
}

int YLuaFuncs::color(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "color", "line, col")) return 0;
    int sCol = ( int )lua_tonumber( L, 1 );
    int sLine = ( int )lua_tonumber( L, 2 );
    lua_pop(L, 2);
    sCol = sCol ? sCol - 1 : 0;
    sLine = sLine ? sLine - 1 : 0;

    YView* cView = YSession::self()->currentView();
    QByteArray c = cView->drawColor( sCol, sLine ).name().toUtf8();
    const char *color = c.data();

    // dbg() << "Asked color: " << color.latin1() << endl;
    lua_pushstring( L, color );
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ; // one result
}

int YLuaFuncs::linecount(lua_State *L)
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "linecount", "")) return 0;

    YView* cView = YSession::self()->currentView();

    lua_pushnumber( L, cView->buffer()->lineCount()); // first result
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ; // one result
}

int YLuaFuncs::version( lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "version", "")) return 0;

    lua_pushstring( L, VERSION_CHAR );
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ;
}

int YLuaFuncs::sendkeys( lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "sendkeys", "text")) return 0;
    QString text = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YKeySequence inputs(text); // Performs the lexical analysis here
    YSession::self()->sendMultipleKeys(YSession::self()->currentView(), inputs);

    // nothing to return
    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::highlight( lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 3, 100, "highlight", "type, style, ...")) return 0;

    QStringList arg;
    int n = lua_gettop(L);
    for ( int i = 1; i <= n ; i++ ) {
        arg << ( char * )lua_tostring( L, i );
    }
    lua_pop(L, n);

    YExCommandArgs args(NULL, QString(), QString(), arg.join(" "), 0, 0, true);
    YSession::self()->getExPool()->highlight(args);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::yzdebug( lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "yzdebug", "text")) return 0;
    QString text = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    dbg() << text << endl;

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::yzprint(lua_State * /*L*/)
{
    // fetch string from the stack
    // print it
    return 0;
}

int YLuaFuncs::connect(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "connect", "event (string), function (string)")) return 0;
    QString event = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    QString function = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    YSession::self()->eventConnect(event, function);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::source(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "source", "filename")) return 0;
    QString filename = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YLuaEngine::self()->source(filename);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::setlocal(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "setlocal", "option name")) return 0;
    QString option = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YExCommandArgs ex (YSession::self()->currentView(), QString(), "setlocal", option, 0, 0, true);
    YSession::self()->getExPool()->set(ex);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::newoption(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 6, 6, "newoption", "option name, group name, default value, value, visibility (number), type (number)")) return 0;
    QString option = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    QString group = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    QString defaultvalue = QString::fromUtf8( ( char * )lua_tostring ( L, 3 ) );
    QString value = QString::fromUtf8( ( char * )lua_tostring ( L, 4 ) );
    OptContext visibility = (OptContext)(int)lua_tonumber ( L, 5 );
    OptType type = (OptType)(int)lua_tonumber ( L, 6 );
    lua_pop(L, 6);

    YSession::self()->getOptions()->createOption(option, group, defaultvalue, value, visibility, type );

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::map(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "map", "key, text")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    QString mapp = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    YZMapping::self()->addGlobalMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::unmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "unmap", "key")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YZMapping::self()->deleteGlobalMapping(key);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::imap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "imap", "key, text")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    QString mapp = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    YZMapping::self()->addInsertMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::iunmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "iunmap", "key")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YZMapping::self()->deleteInsertMapping(key);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::omap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "omap", "key, text")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    QString mapp = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    YZMapping::self()->addPendingOpMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::ounmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "ounmap", "key")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YZMapping::self()->deletePendingOpMapping(key);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::vmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "vmap", "key, text")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    QString mapp = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    YZMapping::self()->addVisualMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::vunmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "vunmap", "key")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YZMapping::self()->deleteVisualMapping(key);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::cmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "cmap", "key, text")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    QString mapp = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    YZMapping::self()->addCmdLineMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::cunmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "cunmap", "key")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YZMapping::self()->deleteCmdLineMapping(key);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::nmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "nmap", "key, text")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    QString mapp = QString::fromUtf8( ( char * )lua_tostring ( L, 2 ) );
    lua_pop(L, 2);

    YZMapping::self()->addNormalMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::nunmap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "nunmap", "key (string)")) return 0;
    QString key = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YZMapping::self()->deleteNormalMapping(key);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::noremap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "noremap", "key, text")) return 0;
    QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
    QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
    lua_pop(L, 2);

    YZMapping::self()->addGlobalNoreMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::nnoremap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "nnoremap", "key, text")) return 0;
    QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
    QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
    lua_pop(L, 2);

    YZMapping::self()->addNormalNoreMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::vnoremap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "vnoremap", "key, text")) return 0;
    QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
    QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
    lua_pop(L, 2);

    YZMapping::self()->addVisualNoreMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::onoremap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "onoremap", "key, text")) return 0;
    QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
    QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
    lua_pop(L, 2);

    YZMapping::self()->addPendingOpNoreMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::inoremap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "inoremap", "key, text")) return 0;
    QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
    QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
    lua_pop(L, 2);

    YZMapping::self()->addInsertNoreMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::cnoremap(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 2, 2, "cnoremap", "key, text")) return 0;
    QString key = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
    QString mapp = QString::fromUtf8(( char * )lua_tostring ( L, 2 ));
    lua_pop(L, 2);

    YZMapping::self()->addCmdLineNoreMapping(key, mapp);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::matchpair(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "matchpair", "")) return 0;

    bool found = false;
    YView *v = YSession::self()->currentView();
    YCursor s = v->getLineColumnCursor();
    YCursor c = v->buffer()->action()->match(v, s, &found);

    lua_pushboolean(L , found);
    lua_pushnumber(L, c.x());
    lua_pushnumber(L, c.y());
    YASSERT_EQUALS( lua_gettop(L), 3 );
    return 3 ;
}

int YLuaFuncs::mode(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 0, 0, "mode", "")) return 0;

    YView *v = YSession::self()->currentView();
    QString mode = v->currentMode()->toString();

    lua_pushstring(L, mode.toUtf8());
    YASSERT_EQUALS( lua_gettop(L), 1 );
    return 1 ;
}

int YLuaFuncs::edit(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "edit", "filename")) return 0;
    QString fname = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
    lua_pop(L, 1);

    if (!fname.isEmpty())
        YSession::self()->createBufferAndView(fname);

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::set(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "set", "option (string)")) return 0;
    QString option = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);

    YSession::self()->getExPool()->set(YExCommandArgs(YSession::self()->currentView(), QString(), QString(), option, 0, 0, true));

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::loadplugin(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "loadplugin", "plugin name")) return 0;
    QString pname = QString::fromUtf8(( char * )lua_tostring ( L, 1 ));
    lua_pop(L, 1);

    if (!pname.isEmpty()) {
        QString resource = YSession::self()->resourceMgr()->findResource( UserScriptResource, pname );
        if (! resource.isEmpty()) {
            YLuaEngine::self()->source( resource );
        }
    }

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}

int YLuaFuncs::setLuaReturnValue(lua_State *L )
{
    if (!YLuaEngine::checkFunctionArguments(L, 1, 1, "setLuaReturnValue", "return value as string")) return 0;
    QString luaReturnValue = QString::fromUtf8( ( char * )lua_tostring ( L, 1 ) );
    lua_pop(L, 1);
    YLuaEngine::self()->setLuaReturnValue( luaReturnValue );

    YASSERT_EQUALS( lua_gettop(L), 0 );
    return 0 ;
}
