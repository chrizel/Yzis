/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>,
 *  Thomas Capricelli <orzel@freehackers.org>,
 *  Philippe Fremy <phil@freehackers.org>
 *  Pascal "Poizon" Maillard <poizon@gmx.at>
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

#include <qregexp.h>
#include <qptrlist.h>
#include "commands.h"
#include "debug.h"
#include "view.h"
#include "ex_executor.h"
#include "ex_lua.h"
#include "session.h"
#include "buffer.h"

/**
 */
YZCommandPool::YZCommandPool() {
	commands.clear();
	commands.setAutoDelete(true);
	globalExCommands.clear();
}

YZCommandPool::~YZCommandPool() {
	commands.clear();
	globalExCommands.clear();
}

/**
 * NORMAL MODE COMMANDS
 */
void YZCommandPool::initPool() {
	textObjects << "aw" << "iw";
	commands.append( new YZCommand("0", &YZCommandPool::gotoSOL, ARG_NONE, true) );
	commands.append( new YZCommand("$", &YZCommandPool::gotoEOL, ARG_NONE, true) );
	commands.append( new YZCommand("w", &YZCommandPool::moveWordForward, ARG_NONE, true) );
	commands.append( new YZCommand("j", &YZCommandPool::moveDown, ARG_NONE, true) );
	commands.append( new YZCommand("k", &YZCommandPool::moveUp, ARG_NONE, true) );
	commands.append( new YZCommand("h", &YZCommandPool::moveLeft, ARG_NONE, true) );
	commands.append( new YZCommand("l", &YZCommandPool::moveRight, ARG_NONE, true) );
	commands.append( new YZCommand("f", &YZCommandPool::find, ARG_CHAR, true) );
	commands.append( new YZCommand("i", &YZCommandPool::gotoInsertMode) );
	commands.append( new YZCommand(":", &YZCommandPool::gotoExMode) );
	commands.append( new YZCommand("R", &YZCommandPool::gotoReplaceMode) );
	commands.append( new YZCommand("v", &YZCommandPool::gotoVisualMode) );
	commands.append( new YZCommand("V", &YZCommandPool::gotoVisualLineMode) );
	commands.append( new YZCommand("gg", &YZCommandPool::gotoFirstLine) );
	commands.append( new YZCommand("G", &YZCommandPool::gotoLastLine) );
	commands.append( new YZCommand("dd", &YZCommandPool::deleteLine) );
	commands.append( new YZCommand("d", &YZCommandPool::del, ARG_MOTION) );
	commands.append( new YZCommand("D", &YZCommandPool::deleteToEOL) );
	commands.append( new YZCommand("yy", &YZCommandPool::yankLine) );
	commands.append( new YZCommand("y", &YZCommandPool::yank, ARG_MOTION) );
	commands.append( new YZCommand("Y", &YZCommandPool::yankToEOL) );
	commands.append( new YZCommand("cc", &YZCommandPool::changeLine) );
	commands.append( new YZCommand("c", &YZCommandPool::change, ARG_MOTION) );
	commands.append( new YZCommand("C", &YZCommandPool::changeToEOL) );
	commands.append( new YZCommand("p", &YZCommandPool::pasteAfter) );
	commands.append( new YZCommand("P", &YZCommandPool::pasteBefore) );
	commands.append( new YZCommand("o", &YZCommandPool::insertLineAfter) );
	commands.append( new YZCommand("O", &YZCommandPool::insertLineBefore) );
	commands.append( new YZCommand("a", &YZCommandPool::append) );
	commands.append( new YZCommand("A", &YZCommandPool::appendAtEOL) );
	commands.append( new YZCommand("J", &YZCommandPool::joinLine) );
	commands.append( new YZCommand("ZZ", &YZCommandPool::saveAndClose) );
	commands.append( new YZCommand("ZQ", &YZCommandPool::closeWithoutSaving) );
	commands.append( new YZCommand("/", &YZCommandPool::searchForwards) );
	commands.append( new YZCommand("?", &YZCommandPool::searchBackwards) );
	commands.append( new YZCommand("n", &YZCommandPool::searchNext) );
	commands.append( new YZCommand("N", &YZCommandPool::searchPrev) );
	commands.append( new YZCommand("m", &YZCommandPool::mark, ARG_CHAR) );
	commands.append( new YZCommand("`", &YZCommandPool::gotoMark, ARG_MARK) );
	commands.append( new YZCommand("'", &YZCommandPool::gotoMark, ARG_MARK) );
	commands.append( new YZCommand("u", &YZCommandPool::undo) );
	//TODO: redo (will be added after the ::sendkey rewrite)
	//and %,$,0 (treat as 'pure' motions) maybe when we don't find a command try to use it as a motion : like 'w' moves the cursor one word forward in vim
	
}

cmd_state YZCommandPool::execCommand(YZView *view, const QString& inputs) {
	unsigned int count=1;
	unsigned int i=0;
	QValueList<QChar> regs;
	// read in the register operations and the counts
	while(i<inputs.length()) {
		if(inputs[i].digitValue() != -1) {
			unsigned int j=i+1;
			while(j<inputs.length() && inputs[j].digitValue() != -1)
				j++;
			count*=inputs.mid(i, j-i).toInt();
			i=j;
		} else if(inputs[i] == '\"') {
			if(++i>=inputs.length())
				break;
			regs << inputs[i++];
		} else
			break;
	}

	if(i>=inputs.length())
		return NO_COMMAND_YET;

	// collect all the commands
	QPtrList<const YZCommand> cmds, prevcmds;
	cmds.setAutoDelete(false);
	prevcmds.setAutoDelete(false);

	unsigned int j=i;

	// retrieve all the matching commands
	// .. first the ones whose first key matches
	if(j<inputs.length()) {
		for(commands.first(); commands.current(); commands.next())
			if(commands.current()->keySeq().startsWith(inputs.mid(j,1)))
				cmds.append(commands.current());
	}
	j++;
	// .. then the ones whose next keys match, too
	while(!cmds.isEmpty() && ++j<=inputs.length()) {
		prevcmds=cmds;
		// delete all the commands that don't match
		for(cmds.first(); cmds.current();)
			if(cmds.current()->keySeq().startsWith(inputs.mid(i,j-i)))
				cmds.next();
			else
				cmds.remove();
	}
	if(cmds.isEmpty()) {
		// perhaps it is a command with an argument, isolate all those
		for(prevcmds.first(); prevcmds.current();)
			if(prevcmds.current()->arg() == ARG_NONE)
				prevcmds.remove();
			else
				prevcmds.next();
		if(prevcmds.isEmpty())
			return CMD_ERROR;
		// it really is a command with an argument, read it in
		const YZCommand *c=prevcmds.first();
		i=j-1;
		// read in a count that may follow
		if(inputs[i].digitValue() != -1) {
			while(j<inputs.length() && inputs[j].digitValue() != -1)
				j++;
			count*=inputs.mid(i,j-i).toInt();
			i=j;
			if(i>=inputs.length())
				return OPERATOR_PENDING;
		}

		QString s=inputs.mid(i);
		switch(c->arg()) {
		case ARG_MOTION:
			if(s[0]=='a' || s[0]=='i') {
				// text object
				if(s.length()==1)
					return OPERATOR_PENDING;
				else if(!textObjects.contains(s))
					return CMD_ERROR;
			} else {
				// motion, look for a motion that matches exactly
				for(commands.first(); commands.current(); commands.next())
					if(commands.current()->matches(s))
						break;
				if(!commands.current()) {
					// look for an incomplete motion
					for(commands.first(); commands.current(); commands.next())
						if(commands.current()->matches(s, false))
							return OPERATOR_PENDING;
					return CMD_ERROR;
				}
			}
			break;
		case ARG_CHAR:
		case ARG_REG:
			if(s.length()!=1)
				return CMD_ERROR;
			break;
		case ARG_MARK:
			if(s.length()!=1 || !YZCommand::isMark(s[0]))
				return CMD_ERROR;
			break;
		default:
			break;
		}
		// the argument is OK, go for it
		(this->*(c->poolMethod()))(YZCommandArgs(view, regs, count, s));
	} else {
		// keep the commands that match exactly
		QString s=inputs.mid(i);
		for(cmds.first();cmds.current();) {
			if(cmds.current()->keySeq()!=s)
				cmds.remove();
			else
				cmds.next();
		}
		if(cmds.isEmpty())
			return NO_COMMAND_YET;
		bool visual = view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL
		              || view->getCurrentMode()==YZView::YZ_VIEW_MODE_VISUAL_LINE;
		YZSelectionMap m;
		if(visual)
			m=view->getVisualSelection();
		const YZCommand *c=0;
		if(cmds.count()==1) {
			c=cmds.first();
			if(c->arg() == ARG_NONE || visual && c->arg() == ARG_MOTION)
				(this->*(c->poolMethod()))(YZCommandArgs(view, regs, count, &m));
			else
				return OPERATOR_PENDING;
		} else {
			/* two or more commands with the same name, we assert that these are exactly
			a cmd that needs a motion and one without an argument. In visual mode, we take
			the operator, in normal mode, we take the other. */
			for(cmds.first();cmds.current();) {
				if(cmds.current()->arg() == ARG_MOTION && visual ||
				        cmds.current()->arg() == ARG_NONE && !visual)
					c=cmds.current();
			}
			if(!c)
				return CMD_ERROR;
			if(visual)
				(this->*(c->poolMethod()))(YZCommandArgs(view, regs, count, &m));
			else
				(this->*(c->poolMethod()))(YZCommandArgs(view, regs, count, QString::null));
		}
	}

	return CMD_OK;
}

/**
 * EX MODE COMMANDS
 */

void YZCommandPool::initExPool() {
	NEW_EX_COMMAND("write", &YZExExecutor::write,true,1);
	NEW_EX_COMMAND("wall", &YZExExecutor::write,true,2);
	NEW_EX_COMMAND("wqall", &YZExExecutor::write,true,3); //handles wq too
	NEW_EX_COMMAND("bnext", &YZExExecutor::buffernext,true,1);
	NEW_EX_COMMAND("bprevious", &YZExExecutor::bufferprevious,true,2);
	NEW_EX_COMMAND("bdelete", &YZExExecutor::bufferdelete,true,3);
	NEW_EX_COMMAND("edit", &YZExExecutor::edit,true,0);
	NEW_EX_COMMAND("quit", &YZExExecutor::quit,true,1);
	NEW_EX_COMMAND("qall", &YZExExecutor::quit,true,2);
	NEW_EX_COMMAND("mkyzisrc", &YZExExecutor::mkyzisrc,true,0);
	NEW_EX_COMMAND("substitute", &YZExExecutor::substitute,true,2);
	NEW_EX_COMMAND("set", &YZExExecutor::set,true,1);
	NEW_EX_COMMAND("setlocal", &YZExExecutor::setlocal,true,1);
	NEW_EX_COMMAND("hardcopy", &YZExExecutor::hardcopy,true,1);
	NEW_EX_COMMAND("open", &YZExExecutor::gotoOpenMode,true,1);
	NEW_EX_COMMAND("visual", &YZExExecutor::gotoCommandMode,true,1);
	NEW_EX_COMMAND("preserve", &YZExExecutor::preserve,true,1);
	NEW_LUA_COMMAND("lua", &YZExLua::lua,true,0);
	NEW_LUA_COMMAND("source", &YZExLua::loadFile,true,0);
}

void YZCommandPool::execExCommand(YZView *view, const QString& inputs) {
	QString command = inputs;
	yzDebug() << "EX CommandLine " << command << endl;
	// assume a command is like : "rangeCOMMANDNAME parameters"
	// see vim :help [range] for infos on 'range'
	//QRegExp rx ( "(%?|((\\d*)(,\\d*)?))(\\w+)((\\b)|(/.*/.*/.*))(.*)");
	QRegExp rx ( "(%?|\\d*|\\d*,\\d*)(\\w+)(.*)");
	if ( rx.exactMatch(command) ) {
		command = rx.cap( 2 );
	} else
		return; //no command identified XXX eventually popup error messages
	QString cmd = QString::null;
	QString tmpCmd = QString::null;
	yzDebug() << "Command : " << command << endl;
	QMap<QString,YZExCommand>::Iterator it;
	int priority=-1;
	for ( it = globalExCommands.begin(); it!=globalExCommands.end(); ++it ) {
		tmpCmd = static_cast<QString>( it.key() );
		//		yzDebug() << "execExCommand : testing for match " << tmpCmd << endl;
		if ( tmpCmd.startsWith( command ) && it.data().priority > priority ) {
			priority=it.data().priority; //store the priority
			cmd = tmpCmd;
			yzDebug() << "Found match for command " << cmd << " with priority " << priority << endl;
		}
	}

	if ( cmd != QString::null ) {
		switch ( globalExCommands[ cmd ].obj ) {
		case EX :
			( *YZSession::me->exExecutor().*(globalExCommands[ cmd ].exFunc )) (view,inputs) ;
			break;
		case LUA :
			( *YZSession::me->luaExecutor().*(globalExCommands[ cmd ].luaFunc )) (view,inputs) ;
			break;
		default:
			break;
		}
	}
}


/*!
    \fn YZCommand::matches(const QString &s)
 */
bool YZCommand::matches(const QString &s, bool fully) const {
	if(!mMotion)
		return false;
	QString ks=mKeySeq;
	if(s.startsWith(ks)) {
		switch(mArg) {
		case ARG_NONE:
			if(s.length() == ks.length())
				return true;
			break;
		case ARG_CHAR:
			if(s.length() == ks.length()+1 || !fully && s.length() == ks.length())
				return true;
			break;
		case ARG_MARK:
			if(s.length() == ks.length()+1 && isMark(s[s.length()-1]) || !fully && s.length() == ks.length())
				return true;
			break;
		default:
			break;
		}
	} else if(!fully && ks.startsWith(s))
		return true;

	return false;
}


/*!
    \fn YZCommandPool::moveLeft(const YZCommandArgs &args)
 */
QString YZCommandPool::moveLeft(const YZCommandArgs &args) {
	args.view->moveLeft( args.count );
	return QString::null;
}


/*!
    \fn YZCommandPool::moveRight(const YZCommandArgs &args)
 */
QString YZCommandPool::moveRight(const YZCommandArgs &args) {
	args.view->moveRight( args.count );
	return QString::null;
}


/*!
    \fn YZCommandPool::moveDown(const YZCommandArgs &args)
 */
QString YZCommandPool::moveDown(const YZCommandArgs &args) {
	args.view->moveDown( args.count );
	return QString::null;
}


/*!
    \fn YZCommandPool::moveUp(const YZCommandArgs &args)
 */
QString YZCommandPool::moveUp(const YZCommandArgs &args) {
	args.view->moveUp( args.count );
	return QString::null;
}


/*!
    \fn YZCommandPool::appendAtEOL(const YZCommandArgs &args)
 */
QString YZCommandPool::appendAtEOL(const YZCommandArgs &args) {
	args.view->moveToEndOfLine();
	args.view->append();
	return QString::null;
}


/*!
    \fn YZCommandPool::append(const YZCommandArgs &args)
 */
QString YZCommandPool::append(const YZCommandArgs &args) {
	args.view->append();
	return QString::null;
}


/*!
    \fn YZCommandPool::changeLine(const YZCommandArgs &args)
 */
QString YZCommandPool::changeLine(const YZCommandArgs &args) {
	//TODO (current implementation in trunk is wrong)

	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::changeToEOL(const YZCommandArgs &args)
 */
QString YZCommandPool::changeToEOL(const YZCommandArgs &args) {
	//TODO
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::deleteLine(const YZCommandArgs &args)
 */
QString YZCommandPool::deleteLine(const YZCommandArgs &args) {
	args.view->deleteLine(args.count, args.regs);
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::deleteToEOL(const YZCommandArgs &args)
 */
QString YZCommandPool::deleteToEOL(const YZCommandArgs &args) {
	//in vim : 2d$ does not behave as d$d$, this is illogical ..., you cannot delete twice to end of line ...
	args.view->del("$", args.regs);
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::gotoExMode(const YZCommandArgs &args)
 */
QString YZCommandPool::gotoExMode(const YZCommandArgs &args) {
	args.view->gotoExMode();
	return QString::null;
}


/*!
    \fn YZCommandPool::gotoFirstLine(const YZCommandArgs &args)
 */
QString YZCommandPool::gotoFirstLine(const YZCommandArgs &args) {
	if ( ! args.view->getLocalBoolOption( "startofline" ) ) { 
		args.view->gotoxy( 0, 0 );
	} else {
		args.view->gotoxy( 0,0 );
		args.view->moveToFirstNonBlankOfLine();
	}
	return QString::null;
}


/*!
    \fn YZCommandPool::gotoInsertMode(const YZCommandArgs &args)
 */
QString YZCommandPool::gotoInsertMode(const YZCommandArgs &args) {
	args.view->gotoInsertMode();
	return QString::null;
}


/*!
    \fn YZCommandPool::gotoLastLine(const YZCommandArgs &args)
 */
QString YZCommandPool::gotoLastLine(const YZCommandArgs &args) {
	args.view->gotoLastLine();
	return QString::null;
}


/*!
    \fn YZCommandPool::gotoReplaceMode(const YZCommandArgs &args)
 */
QString YZCommandPool::gotoReplaceMode(const YZCommandArgs &args) {
	args.view->gotoReplaceMode();
	return QString::null;
}


/*!
    \fn YZCommandPool::gotoVisualLineMode(const YZCommandArgs &args)
 */
QString YZCommandPool::gotoVisualLineMode(const YZCommandArgs &args) {
	args.view->gotoVisualMode(true);
	return QString::null;
}


/*!
    \fn YZCommandPool::gotoVisualMode(const YZCommandArgs &args)
 */
QString YZCommandPool::gotoVisualMode(const YZCommandArgs &args) {
	args.view->gotoVisualMode();
	return QString::null;
}


/*!
    \fn YZCommandPool::insertLineAfter(const YZCommandArgs &args)
 */
QString YZCommandPool::insertLineAfter(const YZCommandArgs &args) {
	args.view->openNewLineAfter(args.count);
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::insertLineBefore(const YZCommandArgs &args)
 */
QString YZCommandPool::insertLineBefore(const YZCommandArgs &args) {
	args.view->openNewLineBefore(args.count);
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::joinLine(const YZCommandArgs &args)
 */
QString YZCommandPool::joinLine(const YZCommandArgs &args) {
	args.view->joinLine( args.view->getBufferCursor()->getY(), args.count );
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::pasteAfter(const YZCommandArgs &args)
 */
QString YZCommandPool::pasteAfter(const YZCommandArgs &args) {
	args.view->paste( args.regs[ 0 ], true );
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::pasteBefore(const YZCommandArgs &args)
 */
QString YZCommandPool::pasteBefore(const YZCommandArgs &args) {
	args.view->paste( args.regs[ 0 ], false );
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::yankLine(const YZCommandArgs &args)
 */
QString YZCommandPool::yankLine(const YZCommandArgs &args) {
	args.view->copyLine( args.count, args.regs );
	return QString::null;
}


/*!
    \fn YZCommandPool::yankToEOL(const YZCommandArgs &args)
 */
QString YZCommandPool::yankToEOL(const YZCommandArgs &args) {
	args.view->copy( "$", args.regs );
	return QString::null;
}


/*!
    \fn YZCommandPool::closeWithoutSaving(const YZCommandArgs &args)
 */
QString YZCommandPool::closeWithoutSaving(const YZCommandArgs &/*args*/) {
	YZSession::me->exitRequest( 0 );
	return QString::null;
}


/*!
    \fn YZCommandPool::saveAndClose(const YZCommandArgs &args)
 */
QString YZCommandPool::saveAndClose(const YZCommandArgs &args) {
	YZSession::me->saveBufferExit();
	return QString::null;
}


/*!
    \fn YZCommandPool::searchBackwards(const YZCommandArgs &args)
 */
QString YZCommandPool::searchBackwards(const YZCommandArgs &args) {
	args.view->gotoSearchMode(true);
	return QString::null;
}


/*!
    \fn YZCommandPool::searchForwards(const YZCommandArgs &args)
 */
QString YZCommandPool::searchForwards(const YZCommandArgs &args) {
	args.view->gotoSearchMode();
	return QString::null;
}


/*!
    \fn YZCommandPool::searchNext(const YZCommandArgs &args)
 */
QString YZCommandPool::searchNext(const YZCommandArgs &args) {
	args.view->searchAgain( args.count, false );
	return QString::null;
}


/*!
    \fn YZCommandPool::searchPrev(const YZCommandArgs &args)
 */
QString YZCommandPool::searchPrev(const YZCommandArgs &args) {
	args.view->searchAgain( args.count, true );
	return QString::null;
}


/*!
    \fn YZCommandPool::change(const YZCommandArgs &args)
 */
QString YZCommandPool::change(const YZCommandArgs &args) {
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::del(const YZCommandArgs &args)
 */
QString YZCommandPool::del(const YZCommandArgs &args) {
	args.view->del( args.arg, args.regs );
	args.view->commitNextUndo();
	return QString::null;
}


/*!
    \fn YZCommandPool::find(const YZCommandArgs &args)
 */
QString YZCommandPool::find(const YZCommandArgs &args) {
	//improve me XXX , merge search commands in YZView
	args.view->doSearch( args.arg );
	if ( args.count > 1 )
		args.view->searchAgain( args.count - 1);
	return QString::null;
}


/*!
    \fn YZCommandPool::yank(const YZCommandArgs &args)
 */
QString YZCommandPool::yank(const YZCommandArgs &args) {
	args.view->copy( args.arg, args.regs );
	return QString::null;
}

QString YZCommandPool::mark(const YZCommandArgs &args) {
	args.view->mark( args.arg );
	return QString::null;
}

QString YZCommandPool::gotoMark(const YZCommandArgs &args) {
	args.view->gotoMark( args.arg );
	return QString::null;
}

QString YZCommandPool::undo(const YZCommandArgs &args) {
	args.view->undo( args.count );
	return QString::null;
}

/**
 * Motions
 */
QString YZCommandPool::gotoSOL(const YZCommandArgs &args) {
	args.view->moveToStartOfLine();
	return QString::null;
}

QString YZCommandPool::gotoEOL(const YZCommandArgs &args) {
	args.view->moveToEndOfLine();
	return QString::null;
}

QString YZCommandPool::moveWordForward(const YZCommandArgs &args) {
	//TODO
	return QString::null;
}

