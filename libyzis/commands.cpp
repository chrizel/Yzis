/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>,
 *  Thomas Capricelli <orzel@freehackers.org>,
 *  Philippe Fremy <phil@freehackers.org>
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

/**
 * $Id$
 */

#include "commands.h"
#include "view.h"
#include "debug.h"
#include <qregexp.h>

/**
 * Here is a simple example to add a new command and how to use it :
 * This binds the keystroke "test", to the function YZCommandPool::test()
 * Note that you have to precise on which kind of objects you intend to operate ( here
 * it's the pool)
 * The third parameter specify whether your command will be overwritable or not ( not working
 * yet )
 * NEW_POOL_COMMAND("test", &YZCommandPool::test(),true);
 * Update: "test" can be a string regexp too
 *
 * This calls the test function with an empty arg list using the 'this' instance :
 *	QString result = ( *this.*( globalCommands[ "test" ].function ) )( QStringList() );
 *
 * Thanks to this you can access commands on any view/buffer.
 * This should also allow us to 'remap' commands, and dynamically add new ones :)
 */
YZCommandPool::YZCommandPool() {
	executor = new YZExExecutor();
}

YZCommandPool::~YZCommandPool() {
	globalCommands.clear();
	globalExCommands.clear();
	delete executor;
}

/**
 * NORMAL MODE COMMANDS
 */
void YZCommandPool::initPool() {
	//normal stuff
	NEW_VIEW_COMMAND("([0-9]*)(j)",&YZView::moveDown,true,true,false,false);
	NEW_VIEW_COMMAND("([0-9]*)(k)",&YZView::moveUp,true,true,false,false);
	NEW_VIEW_COMMAND("([0-9]*)(h)",&YZView::moveLeft,true,true,false,false);
	NEW_VIEW_COMMAND("([0-9]*)(l)",&YZView::moveRight,true,true,false,false);
	NEW_VIEW_COMMAND("\\^",&YZView::moveToFirstNonBlankOfLine,true,false,false,false);
	NEW_VIEW_COMMAND("0",&YZView::moveToStartOfLine,true,false,false,false);
	NEW_VIEW_COMMAND("\\$",&YZView::moveToEndOfLine,true,false,false,false);
	NEW_VIEW_COMMAND("(\".)?([0-9]*)(x|X)",&YZView::deleteCharacter,true,true,false,false);
	NEW_VIEW_COMMAND("i",&YZView::gotoInsertMode,true,false,false,false);
	NEW_VIEW_COMMAND(":",&YZView::gotoExMode,true,false,false,false);
	NEW_VIEW_COMMAND("R",&YZView::gotoReplaceMode,true,false,false,false);
	NEW_VIEW_COMMAND("([0-9]*)(gg|G)",&YZView::gotoLine,true,true,false,false);
	NEW_VIEW_COMMAND("(\".)?([0-9]*)(d.|D)",&YZView::deleteLine,true,true,true,true);
	NEW_VIEW_COMMAND("o",&YZView::openNewLineAfter,true,false,false,false);
	NEW_VIEW_COMMAND("O",&YZView::openNewLineBefore,true,false,false,false);
	NEW_VIEW_COMMAND("a",&YZView::append,true,false,false,false);
	NEW_VIEW_COMMAND("A",&YZView::appendAtEOL,true,false,false,false);
	NEW_SESS_COMMAND("ZZ",&YZSession::saveBufferExit,true,false,false,false);
	NEW_VIEW_COMMAND("(\".)?([0-9]*)(y.|Y)",&YZView::copy,true,true,false,true);
	NEW_VIEW_COMMAND("(\".)?(p|P)",&YZView::paste,true,false,false,true);
}

void YZCommandPool::execCommand(YZView *view, const QString& inputs, int * /* error */ ) {
	QString result;
	QString command=QString::null;
	YZCommandArgs args;

	QMap<QString, YZCommand>::Iterator it = globalCommands.end();
	for ( it = globalCommands.begin(); it!=globalCommands.end(); ++it ) {
		QString t = it.key();
		QRegExp ex ( t );
		if ( ex.exactMatch( inputs ) ) { //command found
			command = it.key(); 
			//fill args now
			int ag=1;
			if ( it.data().hasRegister ) {
				args.registr = ex.cap( ag++ )[ 1 ];
				if ( args.registr.isNull() ) args.registr = '"'; //default register to use
				yzDebug() << "hasRegister : " << QString( args.registr ) << endl;
			}
			if ( it.data().hasCounter ) {
				args.count = ( ex.cap( ag ).isNull() || ex.cap( ag ).isEmpty() ) ? 1 : ex.cap( ag ).toUInt();
				ag++;
				yzDebug() << "hasCounter : " << args.count << endl;
			}
			args.command = ex.cap( ag++ );
			yzDebug() << "Command : " << args.command << endl;
			//at last there is the motion
			if ( it.data().hasMotion ) {//TODO
				yzDebug() << "hasMotion : " << endl;
			}
			break; //leave now
		}
	}

	if ( command.isNull() ) {
		return; //not found :/
	}
	switch ( globalCommands[ command ].obj ) {
		case VIEW :
			result = ( *view.*(globalCommands[ command ].viewFunc )) (inputs,args) ;
			break;
		case BUFF :
			result = ( *( view->myBuffer() ).*(globalCommands[ command ].buffFunc )) (inputs,args) ;
			break;
		case SESS :
			result = ( *( view->mySession() ).*(globalCommands[ command ].sessFunc )) (inputs,args) ;
			break;
		case POOL :
			result = ( *( view->mySession()->getPool() ).*(globalCommands[ command ].poolFunc )) (inputs,args) ;
			break;
			/**		case PLUG :
			  result = ( *this.*(globalCommands[ command ].viewFunc )) (inputs,args) ;
			  break;*/
		default:
			break;
	}
}

/**
 * EX MODE COMMANDS
 */

void YZCommandPool::initExPool() {
	NEW_EX_COMMAND("write", &YZExExecutor::write,true);
	NEW_EX_COMMAND("bnext", &YZExExecutor::buffernext,true);
	NEW_EX_COMMAND("bprevious", &YZExExecutor::bufferprevious,true);
	NEW_EX_COMMAND("bdelete", &YZExExecutor::bufferdelete,true);
	NEW_EX_COMMAND("edit", &YZExExecutor::edit,true);
	NEW_EX_COMMAND("quit", &YZExExecutor::quit,true);
}

void YZCommandPool::execExCommand(YZView *view, const QString& inputs) {
	QString command = inputs;
	yzDebug() << "EX CommandLine " << command << endl;
	// assume a command is like : "numberCOMMANDNAME parameters"
	QRegExp rx ( "^(\\d*)(\\S+)\\b");
	rx.search(command);
	if ( rx.cap( 2 ) != QString::null )
		command = rx.cap( 2 );
	else
		return; //no command identified XXX eventually popup error messages
	QString cmd = QString::null;
	QMap<QString,YZCommand>::Iterator it;
	for ( it = globalExCommands.begin(); it!=globalExCommands.end(); ++it ) {
		cmd = static_cast<QString>( it.key() );
		yzDebug() << "execExCommand : testing for match " << cmd << endl;
		if ( cmd.startsWith( command ) ) break;
		cmd = QString::null;
	}

	if ( cmd != QString::null ) {
		switch ( globalExCommands[ cmd ].obj ) {
				case EX :
					( *executor.*(globalExCommands[ cmd ].exFunc )) (view,inputs) ;
					break;
			default:
				break;
		}
	}
}
