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
#include <assert.h>
#include "commands.h"
#include "debug.h"
#include "view.h"
#include "viewcursor.h"
#include "ex_executor.h"
#include "ex_lua.h"
#include "session.h"
#include "buffer.h"
#include "cursor.h"
#include "action.h"
#include "mark.h"

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
//	textObjects << "aw" << "iw";
	commands.append( new YZNewMotion("0", &YZCommandPool::gotoSOL, ARG_NONE) );
	commands.append( new YZNewMotion("$", &YZCommandPool::gotoEOL, ARG_NONE) );
	commands.append( new YZNewMotion("^", &YZCommandPool::firstNonBlank, ARG_NONE) );
	commands.append( new YZNewMotion("w", &YZCommandPool::moveWordForward, ARG_NONE) );
	commands.append( new YZNewMotion("b", &YZCommandPool::moveWordBackward, ARG_NONE) );
	commands.append( new YZNewMotion("j", &YZCommandPool::moveDown, ARG_NONE) );
	commands.append( new YZNewMotion("k", &YZCommandPool::moveUp, ARG_NONE) );
	commands.append( new YZNewMotion("h", &YZCommandPool::moveLeft, ARG_NONE) );
	commands.append( new YZNewMotion("l", &YZCommandPool::moveRight, ARG_NONE) );
	commands.append( new YZNewMotion("<BS>", &YZCommandPool::moveLeftWrap, ARG_NONE) );
	commands.append( new YZNewMotion(" ", &YZCommandPool::moveRightWrap, ARG_NONE) );
	commands.append( new YZNewMotion("f", &YZCommandPool::find, ARG_CHAR) );
	commands.append( new YZNewMotion("<HOME>", &YZCommandPool::gotoSOL, ARG_NONE) );
	commands.append( new YZNewMotion("<END>", &YZCommandPool::gotoEOL, ARG_NONE) );
	commands.append( new YZNewMotion("<LEFT>", &YZCommandPool::moveLeft, ARG_NONE) );
	commands.append( new YZNewMotion("<RIGHT>", &YZCommandPool::moveRight, ARG_NONE) );
	commands.append( new YZNewMotion("<UP>", &YZCommandPool::moveUp, ARG_NONE) );
	commands.append( new YZNewMotion("<DOWN>", &YZCommandPool::moveDown, ARG_NONE) );
	commands.append( new YZNewMotion("<PUP>", &YZCommandPool::movePageUp, ARG_NONE) );
	commands.append( new YZNewMotion("<PDOWN>", &YZCommandPool::movePageDown, ARG_NONE) );
	commands.append( new YZNewMotion("%", &YZCommandPool::matchPair, ARG_NONE) );
	commands.append( new YZNewMotion("`", &YZCommandPool::gotoMark, ARG_MARK) );
	commands.append( new YZNewMotion("'", &YZCommandPool::gotoMark, ARG_MARK) );
	commands.append( new YZCommand("i", &YZCommandPool::gotoInsertMode) );
	commands.append( new YZCommand("<INS>", &YZCommandPool::gotoInsertMode) );
	commands.append( new YZCommand(":", &YZCommandPool::gotoExMode) );
	commands.append( new YZCommand("R", &YZCommandPool::gotoReplaceMode) );
	commands.append( new YZCommand("v", &YZCommandPool::gotoVisualMode) );
	commands.append( new YZCommand("V", &YZCommandPool::gotoVisualLineMode) );
	commands.append( new YZCommand("gg", &YZCommandPool::gotoFirstLine) );
	commands.append( new YZCommand("G", &YZCommandPool::gotoLastLine) );
	commands.append( new YZCommand("dd", &YZCommandPool::deleteLine) );
	commands.append( new YZCommand("d", &YZCommandPool::del, ARG_MOTION) );
	commands.append( new YZCommand("D", &YZCommandPool::deleteToEOL) );
	commands.append( new YZCommand("x", &YZCommandPool::deleteChar) );
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
	commands.append( new YZCommand("u", &YZCommandPool::undo) );
	commands.append( new YZCommand("<CTRL>R", &YZCommandPool::redo) );
	commands.append( new YZCommand("q", &YZCommandPool::macro) );
	commands.append( new YZCommand("@", &YZCommandPool::replayMacro) );
	commands.append( new YZCommand("<CTRL>l", &YZCommandPool::redisplay) );
}

cmd_state YZCommandPool::execCommand(YZView *view, const QString& inputs) {
	unsigned int count=1;
	unsigned int i=0;
	QValueList<QChar> regs;
	// read in the register operations and the counts
	while(i<inputs.length()) {
		if(inputs[i].digitValue() > 0) {
			unsigned int j=i+1;
			while(j<inputs.length() && inputs[j].digitValue() >= 0)
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
	//if regs is empty add the default register
	if ( regs.count() == 0 ) 
		regs << '\"';

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
		if(inputs[i].digitValue() > 0) {
			while(j<inputs.length() && inputs[j].digitValue() > 0)
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
				for(commands.first(); commands.current(); commands.next()) {
					const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(commands.current());
					if(m && m->matches(s))
						break;
				}
				if(!commands.current()) {
					// look for an incomplete motion
					for(commands.first(); commands.current(); commands.next()) {
						const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(commands.current());
						if(m && m->matches(s, false))
							return OPERATOR_PENDING;
					}
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
		(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, s));
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
				(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, m));
			else
				return OPERATOR_PENDING;
		} else {
			/* two or more commands with the same name, we assert that these are exactly
			a cmd that needs a motion and one without an argument. In visual mode, we take
			the operator, in normal mode, we take the other. */
			//this is not sufficient, see the 'q' (record macro command), we need a q+ARG_CHAR and a 'q' commands //mm //FIXME
			for(cmds.first();cmds.current();cmds.next()) {
				if(cmds.current()->arg() == ARG_MOTION && visual ||
				        cmds.current()->arg() == ARG_NONE && !visual)
					c=cmds.current();
			}
			if(!c)
				return CMD_ERROR;
			if(visual)
				(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, m));
			else
				(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, QString::null));
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

bool YZNewMotion::matches(const QString &s, bool fully) const {
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

YZCursor YZCommandPool::move(YZView *view, const QString &inputs, unsigned int count) {
	for(commands.first(); commands.current(); commands.next()) {
		// is the command a motion and does it match to the string?
		const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(commands.current());
		if(m && m->matches(inputs)) {
			// execute the corresponding method
			YZCursor to=(this->*(m->motionMethod()))(YZNewMotionArgs(view, count,
					inputs.mid(m->keySeq().length())));
			return to;
		}
	}
	return *view->getBufferCursor();
}


// MOTIONS

YZCursor YZCommandPool::moveLeft(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveLeft(&viewCursor, args.count, false, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveRight(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveRight(&viewCursor, args.count, false, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveLeftWrap( const YZNewMotionArgs & args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveLeft(&viewCursor, args.count, true, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveRightWrap( const YZNewMotionArgs & args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveRight(&viewCursor, args.count, true, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveDown(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveDown(&viewCursor, args.count, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveUp(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveUp(&viewCursor, args.count, args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::movePageUp(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveUp(&viewCursor, args.view->getLinesVisible(), args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::movePageDown(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveDown(&viewCursor, args.view->getLinesVisible(), args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::matchPair(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	bool found = false;
	YZCursor pos = args.view->myBuffer()->action()->match( args.view, *viewCursor.buffer(), &found );
	if ( found ) return pos;
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::find(const YZNewMotionArgs &args) {
	//improve me XXX , merge search commands in YZView
	YZCursor c=*args.view->getBufferCursor();
	args.view->doSearch( args.arg );
	if ( args.count > 1 )
		args.view->searchAgain( args.count - 1);
	YZCursor d=*args.view->getBufferCursor();
	args.view->gotoxy(c.getX(), c.getY(), args.standalone);
	return d;
}

YZCursor YZCommandPool::gotoSOL(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToStartOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::gotoEOL(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToEndOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::moveWordForward(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	YZCursor result( viewCursor.buffer() );
	unsigned int c = 0;
	QRegExp rex1("^\\w+\\s*");//a word with boundaries
	QRegExp rex2("^[^\\w\\s]+\\s*");//non-word chars with boundaries
	QRegExp ws("^\\s+");//whitespace
			
	while ( c < args.count ) { //for each word
		const QString& current = args.view->myBuffer()->textline( result.getY() );
//		if ( current.isNull() ) return false; //be safe ?
		
		int idx = rex1.search( current, result.getX(), QRegExp::CaretAtOffset );
		int len = rex1.matchedLength();
		if ( idx == -1 ) {
			idx = rex2.search( current, result.getX(), QRegExp::CaretAtOffset );
			len = rex2.matchedLength();
		}
		if ( idx == -1 ) {
			idx = ws.search( current, result.getX(), QRegExp::CaretAtOffset );
			len = ws.matchedLength();
		}			
		if ( idx != -1 ) {
			yzDebug() << "Match at " << idx << " Matched length " << len << endl;
			c++; //one match
			result.setX( idx + len );
			if(result.getX() == current.length() &&	result.getY() < args.view->myBuffer()->lineCount() - 1) {
				result.setY(result.getY() + 1);
				ws.search(args.view->myBuffer()->textline( result.getY() ));
				result.setX(ws.matchedLength());
			}
		} else {
			if ( result.getY() >= args.view->myBuffer()->lineCount() - 1 ) {
				result.setX( current.length() );
				break;
			}
			result.setX(0);
			result.setY( result.getY() + 1 );
		}
		
	}
	return result;
}

QString invertQString( const QString& from ) {
	QString res = "";
	for ( int i = from.length() - 1 ; i >= 0; i-- ) {
		res.append( from[ i ] );
	}
	return res;
}

YZCursor YZCommandPool::moveWordBackward(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	YZCursor result( viewCursor.buffer() );
	unsigned int c = 0;
	QRegExp rex1("^\\w+\\s*");//a word with boundaries
	QRegExp rex2("^[^\\w\\s]+\\s*");//non-word chars with boundaries
	QRegExp rex3("^\\s+");//whitespace
			
	while ( c < args.count ) { //for each word
		const QString& current = invertQString( args.view->myBuffer()->textline( result.getY() ) );
		int lineLength = current.length();
		int offset = lineLength - result.getX();
		yzDebug() << current << " at " << offset << endl;

		
		int idx = rex1.search( current, offset , QRegExp::CaretAtOffset );
		int len = rex1.matchedLength();
		yzDebug() << "rex1 : " << idx << "," << len << endl;
		if ( idx == -1 ) {
			idx = rex2.search( current, offset, QRegExp::CaretAtOffset );
			len = rex2.matchedLength();
			yzDebug() << "rex2 : " << idx << "," << len << endl;
			if ( idx == -1 ) {
				idx = rex3.search( current, offset, QRegExp::CaretAtOffset );
				len = rex3.matchedLength();
				yzDebug() << "rex3 : " << idx << "," << len << endl;
			}
		}			
		if ( idx != -1 ) {
			yzDebug() << "Match at " << idx << " = " << lineLength - idx << " Matched length " << len << endl;
			c++; //one match
			result.setX( lineLength - idx - len );
		} else {
			if ( result.getY() == 0 ) break; //stop here
			yzDebug() << "Previous line " << result.getY() - 1 << endl;
			const QString& ncurrent = args.view->myBuffer()->textline( result.getY() - 1 );
			result.setX( ncurrent.length() );
			result.setY( result.getY() - 1 );
		}
		
	}
	return result;
}

YZCursor YZCommandPool::firstNonBlank(const YZNewMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToFirstNonBlankOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZCommandPool::gotoMark( const YZNewMotionArgs &args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	bool found = false;
	YZCursorPos pos = args.view->myBuffer()->marks()->get( args.arg, &found );
	if ( found )
		return *pos.bPos;
	else
		return *viewCursor.buffer();
}

// COMMANDS

QString YZCommandPool::execMotion( const YZCommandArgs &args ) {
	const YZNewMotion *m=dynamic_cast<const YZNewMotion*>(args.cmd);
	assert(m);
	YZCursor to = (this->*(m->motionMethod()))(YZNewMotionArgs(args.view, args.count, args.arg,true));
	args.view->gotoxy(to.getX(), to.getY());
	return QString::null;
}

QString YZCommandPool::appendAtEOL(const YZCommandArgs &args) {
	args.view->moveToEndOfLine();
	args.view->append();
	return QString::null;
}

QString YZCommandPool::append(const YZCommandArgs &args) {
	args.view->append();
	return QString::null;
}

QString YZCommandPool::change(const YZCommandArgs &args) {
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) 
		args.view->myBuffer()->action()->deleteArea(args.view, ( args.selection )[ 0 ].from(), ( args.selection )[ 0 ].to() , args.regs);
	else {
		YZCursor to=move(args.view, args.arg, args.count);
		args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	}
	args.view->commitNextUndo();
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) 
		args.view->leaveVisualMode();
	args.view->append();
	return QString::null;
}

QString YZCommandPool::changeLine(const YZCommandArgs &args) {
	args.view->deleteLine(args.count, args.regs);
	args.view->openNewLineBefore();
//	args.view->gotoInsertMode();
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::changeToEOL(const YZCommandArgs &args) {
	YZCursor to=move(args.view, "$", 1);
	args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	args.view->gotoInsertMode();
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::deleteLine(const YZCommandArgs &args) {
	args.view->deleteLine(args.count, args.regs);
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::deleteToEOL(const YZCommandArgs &args) {
	//in vim : 2d$ does not behave as d$d$, this is illogical ..., you cannot delete twice to end of line ...
	YZCursor to=move(args.view, "$", 1);
	args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::gotoExMode(const YZCommandArgs &args) {
	args.view->gotoExMode();
	return QString::null;
}

QString YZCommandPool::gotoFirstLine(const YZCommandArgs &args) {
	if ( ! args.view->getLocalBoolOption( "startofline" ) ) {
		args.view->gotoxy( 0, 0 );
	} else {
		args.view->gotoxy( 0,0 );
		args.view->moveToFirstNonBlankOfLine();
	}
	return QString::null;
}

QString YZCommandPool::gotoInsertMode(const YZCommandArgs &args) {
	args.view->gotoInsertMode();
	return QString::null;
}

QString YZCommandPool::gotoLastLine(const YZCommandArgs &args) {
	args.view->gotoLastLine();
	return QString::null;
}

QString YZCommandPool::gotoReplaceMode(const YZCommandArgs &args) {
	args.view->gotoReplaceMode();
	return QString::null;
}

QString YZCommandPool::gotoVisualLineMode(const YZCommandArgs &args) {
	args.view->gotoVisualMode(true);
	return QString::null;
}

QString YZCommandPool::gotoVisualMode(const YZCommandArgs &args) {
	args.view->gotoVisualMode();
	return QString::null;
}

QString YZCommandPool::insertLineAfter(const YZCommandArgs &args) {
	args.view->openNewLineAfter(args.count);
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::insertLineBefore(const YZCommandArgs &args) {
	args.view->openNewLineBefore(args.count);
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::joinLine(const YZCommandArgs &args) {
	args.view->joinLine( args.view->getBufferCursor()->getY(), args.count );
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::pasteAfter(const YZCommandArgs &args) {
	args.view->paste( args.regs[ 0 ], true );
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::pasteBefore(const YZCommandArgs &args) {
	args.view->paste( args.regs[ 0 ], false );
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::yankLine(const YZCommandArgs &args) {
	args.view->myBuffer()->action()->copyLine( args.view, *args.view->getBufferCursor(), args.count, args.regs );
	return QString::null;
}

QString YZCommandPool::yankToEOL(const YZCommandArgs &args) {
	YZCursor to=move(args.view, "$", 1);
	args.view->myBuffer()->action()->copyArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	return QString::null;
}

QString YZCommandPool::closeWithoutSaving(const YZCommandArgs &/*args*/) {
	YZSession::me->exitRequest( 0 );
	return QString::null;
}

QString YZCommandPool::saveAndClose(const YZCommandArgs &/*args*/) {
	YZSession::me->saveBufferExit();
	return QString::null;
}

QString YZCommandPool::searchBackwards(const YZCommandArgs &args) {
	args.view->gotoSearchMode(true);
	return QString::null;
}

QString YZCommandPool::searchForwards(const YZCommandArgs &args) {
	args.view->gotoSearchMode();
	return QString::null;
}

QString YZCommandPool::searchNext(const YZCommandArgs &args) {
	args.view->searchAgain( args.count, false );
	return QString::null;
}

QString YZCommandPool::searchPrev(const YZCommandArgs &args) {
	args.view->searchAgain( args.count, true );
	return QString::null;
}

QString YZCommandPool::del(const YZCommandArgs &args) {
	bool entireLines =  ( args.arg.length() > 0 && args.arg[ 0 ] == "'" ) || args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE;
	if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL ) 
		args.view->myBuffer()->action()->deleteArea(args.view, ( args.selection )[ 0 ].from(), ( args.selection )[ 0 ].to() , args.regs);
	else if ( entireLines ) {
		YZCursor from, to;
		if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE ) {
			from = ( args.selection )[ 0 ].from();
			to = ( args.selection )[ 0 ].to();
		} else {
			from = *args.view->getBufferCursor();
			to = move(args.view, args.arg, args.count);
			if ( from > to ) {
				YZCursor tmp( to );
				to.setCursor( from );
				from.setCursor( tmp );
			}
		}
		args.view->myBuffer()->action()->deleteLine( args.view, from.getY(), to.getY() - from.getY() + 1 );
	} else {
		YZCursor to=move(args.view, args.arg, args.count);
		args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	}
	args.view->commitNextUndo();
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) 
		args.view->leaveVisualMode();
	return QString::null;
}

QString YZCommandPool::yank(const YZCommandArgs &args) {
	bool entireLines =  ( args.arg.length() > 0 && args.arg[ 0 ] == "'" ) || args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE;
	if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL ) 
		args.view->myBuffer()->action()->copyArea(args.view, ( args.selection )[ 0 ].from(), ( args.selection )[ 0 ].to() , args.regs);
	else if ( entireLines ) {
		YZCursor from, to;
		if ( args.view->getCurrentMode() == YZView::YZ_VIEW_MODE_VISUAL_LINE ) {
			from = ( args.selection )[ 0 ].from();
			to = ( args.selection )[ 0 ].to();
		} else {
			from = *args.view->getBufferCursor();
			to = move(args.view, args.arg, args.count);
			if ( from > to ) {
				YZCursor tmp( to );
				to.setCursor( from );
				from.setCursor( tmp );
			}
		}
		args.view->myBuffer()->action()->copyLine( args.view, from, to.getY() - from.getY() + 1, args.regs );
	} else {
		YZCursor to=move(args.view, args.arg, args.count);
		args.view->myBuffer()->action()->copyArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	}
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) 
		args.view->leaveVisualMode();
	return QString::null;
}

QString YZCommandPool::mark(const YZCommandArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->myBuffer()->marks()->add( args.arg, *viewCursor.buffer(), *viewCursor.screen() );
	return QString::null;
}

QString YZCommandPool::undo(const YZCommandArgs &args) {
	args.view->undo( args.count );
	return QString::null;
}

QString YZCommandPool::redo(const YZCommandArgs &args) {
	args.view->redo( args.count );
	return QString::null;
}

QString YZCommandPool::macro( const YZCommandArgs &args ) {
	if ( args.view->isRecording() )
		args.view->stopRecordMacro();
	else
		args.view->recordMacro( args.regs );
	return QString::null;
}

QString YZCommandPool::replayMacro( const YZCommandArgs &args ) {
	args.view->purgeInputBuffer();
	for ( QValueList<QChar>::const_iterator it = args.regs.begin(); it != args.regs.end(); it++ ) {
		args.view->sendMultipleKey(YZSession::mRegisters.getRegister(*it)[ 0 ]);
	}
	args.view->commitNextUndo();
	return QString::null;
}

QString YZCommandPool::deleteChar( const YZCommandArgs &args ) {
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL )
		args.view->myBuffer()->action()->deleteArea(args.view, ( args.selection )[ 0 ].from(), ( args.selection )[ 0 ].to() , args.regs);
	else {
		args.view->myBuffer()->action()->deleteChar( args.view, args.view->getBufferCursor(), args.count );
	}
	args.view->commitNextUndo();
	if ( args.view->getCurrentMode()>=YZView::YZ_VIEW_MODE_VISUAL ) 
		args.view->leaveVisualMode();
	return QString::null;
}

QString YZCommandPool::redisplay( const YZCommandArgs &args ) {
	args.view->refreshScreen();
	return QString::null;
}

