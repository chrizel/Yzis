/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2003-2004 Philippe Fremy <phil@freehackers.org>
 *  Copyright (C) 2003-2004 Pascal "Poizon" Maillard <poizon@gmx.at>
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2005 Erlend Hamberg <ehamberg@online.no>
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

#include "mode_command.h"

#include <assert.h>

#if QT_VERSION < 0x040000
#include <qregexp.h>
#include <qptrlist.h>
#else
#include <QRegExp>
#endif

#include "debug.h"

#include "action.h"
#include "buffer.h"
#include "cursor.h"
#include "linesearch.h"
#include "mark.h"
#include "selection.h"
#include "session.h"
#include "view.h"
#include "viewcursor.h"

YZModeCommand::YZModeCommand() : YZMode() {
	mType = MODE_COMMAND;
	mString = _( "[ Awaiting Command ]" );
	commands.clear();
#if QT_VERSION < 0x040000
	commands.setAutoDelete(true);
#endif
}

YZModeCommand::~YZModeCommand() {
#if QT_VERSION >= 0x040000
	for ( int ab = 0 ; ab < commands.size(); ++ab)
		delete commands.at(ab);
#endif
	commands.clear();
}
void YZModeCommand::init() {
	initPool();
	initModifierKeys();
}

void YZModeCommand::initPool() {
	initMotionPool();
	initCommandPool();
}
void YZModeCommand::initMotionPool() {
	commands.append( new YZMotion("0", &YZModeCommand::gotoSOL, ARG_NONE) );
	commands.append( new YZMotion("$", &YZModeCommand::gotoEOL, ARG_NONE) );
	commands.append( new YZMotion("^", &YZModeCommand::firstNonBlank, ARG_NONE) );
	commands.append( new YZMotion("w", &YZModeCommand::moveWordForward, ARG_NONE) );
	commands.append( new YZMotion("W", &YZModeCommand::moveSWordForward, ARG_NONE) );
	commands.append( new YZMotion("b", &YZModeCommand::moveWordBackward, ARG_NONE) );
	commands.append( new YZMotion("B", &YZModeCommand::moveSWordBackward, ARG_NONE) );
	commands.append( new YZMotion("j", &YZModeCommand::moveDown, ARG_NONE) );
	commands.append( new YZMotion("k", &YZModeCommand::moveUp, ARG_NONE) );
	commands.append( new YZMotion("h", &YZModeCommand::moveLeft, ARG_NONE) );
	commands.append( new YZMotion("l", &YZModeCommand::moveRight, ARG_NONE) );
	commands.append( new YZMotion("<BS>", &YZModeCommand::moveLeftWrap, ARG_NONE) );
	commands.append( new YZMotion(" ", &YZModeCommand::moveRightWrap, ARG_NONE) );
	commands.append( new YZMotion("f", &YZModeCommand::findNext, ARG_CHAR) );
	commands.append( new YZMotion("t", &YZModeCommand::findBeforeNext, ARG_CHAR) );
	commands.append( new YZMotion("F", &YZModeCommand::findPrevious, ARG_CHAR) );
	commands.append( new YZMotion("T", &YZModeCommand::findAfterPrevious, ARG_CHAR) );
	commands.append( new YZMotion(";", &YZModeCommand::repeatFind, ARG_CHAR) );
	commands.append( new YZMotion("*", &YZModeCommand::searchWord, ARG_NONE) );
	commands.append( new YZMotion("g*", &YZModeCommand::searchWord, ARG_NONE) );
	commands.append( new YZMotion("#", &YZModeCommand::searchWord, ARG_NONE) );
	commands.append( new YZMotion("g#", &YZModeCommand::searchWord, ARG_NONE) );
	commands.append( new YZMotion("n", &YZModeCommand::searchNext, ARG_NONE) );
	commands.append( new YZMotion("N", &YZModeCommand::searchPrev, ARG_NONE) );
	commands.append( new YZMotion("<HOME>", &YZModeCommand::gotoSOL, ARG_NONE) );
	commands.append( new YZMotion("<END>", &YZModeCommand::gotoEOL, ARG_NONE) );
	commands.append( new YZMotion("<LEFT>", &YZModeCommand::moveLeft, ARG_NONE) );
	commands.append( new YZMotion("<RIGHT>", &YZModeCommand::moveRight, ARG_NONE) );
	commands.append( new YZMotion("<UP>", &YZModeCommand::moveUp, ARG_NONE) );
	commands.append( new YZMotion("<DOWN>", &YZModeCommand::moveDown, ARG_NONE) );
	commands.append( new YZMotion("<PUP>", &YZModeCommand::movePageUp, ARG_NONE) );
	commands.append( new YZMotion("<CTRL>B", &YZModeCommand::movePageUp, ARG_NONE) );
	commands.append( new YZMotion("<PDOWN>", &YZModeCommand::movePageDown, ARG_NONE) );
	commands.append( new YZMotion("<CTRL>F", &YZModeCommand::movePageDown, ARG_NONE) );
	commands.append( new YZMotion("%", &YZModeCommand::matchPair, ARG_NONE) );
	commands.append( new YZMotion("`", &YZModeCommand::gotoMark, ARG_MARK) );
	commands.append( new YZMotion("'", &YZModeCommand::gotoMark, ARG_MARK) );
	commands.append( new YZMotion("<ENTER>", &YZModeCommand::firstNonBlankNextLine, ARG_NONE) );
	commands.append( new YZMotion("gg", &YZModeCommand::gotoLine, ARG_NONE) );
	commands.append( new YZMotion("G", &YZModeCommand::gotoLine, ARG_NONE) );
}
void YZModeCommand::initCommandPool() {
	commands.append( new YZCommand("I", &YZModeCommand::insertAtSOL) );
	commands.append( new YZCommand("i", &YZModeCommand::gotoInsertMode) );
	commands.append( new YZCommand("<INS>", &YZModeCommand::gotoInsertMode) );
	commands.append( new YZCommand(":", &YZModeCommand::gotoExMode) );
	commands.append( new YZCommand("R", &YZModeCommand::gotoReplaceMode) );
	commands.append( new YZCommand("v", &YZModeCommand::gotoVisualMode) );
	commands.append( new YZCommand("V", &YZModeCommand::gotoVisualLineMode) );
	commands.append( new YZCommand("z<ENTER>", &YZModeCommand::gotoLineAtTop) );
	commands.append( new YZCommand("z+", &YZModeCommand::gotoLineAtTop) );
	commands.append( new YZCommand("z.", &YZModeCommand::gotoLineAtCenter) );
	commands.append( new YZCommand("z-", &YZModeCommand::gotoLineAtBottom) );
	commands.append( new YZCommand("dd", &YZModeCommand::deleteLine) );
	commands.append( new YZCommand("d", &YZModeCommand::del, ARG_MOTION) );
	commands.append( new YZCommand("D", &YZModeCommand::deleteToEOL) );
	commands.append( new YZCommand("s", &YZModeCommand::substitute) );
	commands.append( new YZCommand("x", &YZModeCommand::deleteChar) );
	commands.append( new YZCommand("X", &YZModeCommand::deleteCharBackwards) );
	commands.append( new YZCommand("yy", &YZModeCommand::yankLine) );
	commands.append( new YZCommand("y", &YZModeCommand::yank, ARG_MOTION) );
	commands.append( new YZCommand("Y", &YZModeCommand::yankToEOL) );
	commands.append( new YZCommand("cc", &YZModeCommand::changeLine) );
	commands.append( new YZCommand("S", &YZModeCommand::changeLine) );
	commands.append( new YZCommand("c", &YZModeCommand::change, ARG_MOTION) );
	commands.append( new YZCommand("C", &YZModeCommand::changeToEOL) );
	commands.append( new YZCommand("p", &YZModeCommand::pasteAfter) );
	commands.append( new YZCommand("P", &YZModeCommand::pasteBefore) );
	commands.append( new YZCommand("o", &YZModeCommand::insertLineAfter) );
	commands.append( new YZCommand("O", &YZModeCommand::insertLineBefore) );
	commands.append( new YZCommand("a", &YZModeCommand::append) );
	commands.append( new YZCommand("A", &YZModeCommand::appendAtEOL) );
	commands.append( new YZCommand("J", &YZModeCommand::joinLine) );
	commands.append( new YZCommand("<", &YZModeCommand::indent, ARG_MOTION ) );
	commands.append( new YZCommand("<<", &YZModeCommand::indent ) );
	commands.append( new YZCommand(">", &YZModeCommand::indent, ARG_MOTION ) );
	commands.append( new YZCommand(">>", &YZModeCommand::indent ) );
	commands.append( new YZCommand("ZZ", &YZModeCommand::saveAndClose) );
	commands.append( new YZCommand("ZQ", &YZModeCommand::closeWithoutSaving) );
	commands.append( new YZCommand("/", &YZModeCommand::searchForwards) );
	commands.append( new YZCommand("?", &YZModeCommand::searchBackwards) );
	commands.append( new YZCommand("~", &YZModeCommand::changeCase) );
	commands.append( new YZCommand("m", &YZModeCommand::mark, ARG_CHAR) );
	commands.append( new YZCommand("r", &YZModeCommand::replace, ARG_CHAR) );
	commands.append( new YZCommand("u", &YZModeCommand::undo) );
	commands.append( new YZCommand("U", &YZModeCommand::redo) );
	commands.append( new YZCommand("<CTRL>r", &YZModeCommand::redo) );
	commands.append( new YZCommand("<CTRL>R", &YZModeCommand::redo) );
	commands.append( new YZCommand("q", &YZModeCommand::macro) );
	commands.append( new YZCommand("@", &YZModeCommand::replayMacro) );
	commands.append( new YZCommand("<CTRL>l", &YZModeCommand::redisplay) );
	commands.append( new YZCommand("<CTRL>[", &YZModeCommand::gotoCommandMode) );
	commands.append( new YZCommand("<ESC>", &YZModeCommand::abort) );
	commands.append( new YZCommand("<DEL>", &YZModeCommand::delkey) );
	commands.append( new YZCommand("<ALT>:", &YZModeCommand::gotoExMode) );
	commands.append( new YZCommand("gUU", &YZModeCommand::lineToUpperCase) );
	commands.append( new YZCommand("gUgU", &YZModeCommand::lineToUpperCase) );
	commands.append( new YZCommand("guu", &YZModeCommand::lineToLowerCase) );
	commands.append( new YZCommand("gugu", &YZModeCommand::lineToLowerCase) );
}
void YZModeCommand::initModifierKeys() {
#if QT_VERSION < 0x040000
	for ( commands.first(); commands.current(); commands.next() ) {
		const QString& keys = commands.current()->keySeq();
		if ( keys.find( "<CTRL>" ) > -1 || keys.find( "<ALT>" ) > -1 ) {
			mModifierKeys << keys;
		}
	}
#else
	for ( int ab = 0 ; ab < commands.size(); ++ab) {
		const QString& keys = commands.at(ab)->keySeq();
		if ( keys.indexOf( "<CTRL>" ) > -1 || keys.indexOf( "<ALT>" ) > -1 ) {
			mModifierKeys << keys;
		}
	}
#endif
}

cmd_state YZModeCommand::execCommand(YZView *view, const QString& inputs) {
//	yzDebug() << "ExecCommand : " << inputs << endl;
	unsigned int count=1;
	bool hadCount = false;
	unsigned int i=0;
#if QT_VERSION < 0x040000
	QValueList<QChar> regs;
	YZView* it;
#else
	QList<QChar> regs;
#endif

	// read in the register operations and the counts
	while(i<inputs.length()) {
		if(inputs.at( i ).digitValue() > 0) {
			unsigned int j=i+1;
			while(j<inputs.length() && inputs.at(j).digitValue() >= 0)
				j++;
			count*=inputs.mid(i, j-i).toInt();
			i=j;
			yzDebug() << "Count " << count << endl;
			hadCount=true; //we found digits given by the user
		} else if(inputs.at( i ) == '\"') {
			if(++i>=inputs.length())
				break;
			regs << inputs.at(i++);
		} else
			break;
	}
	//if regs is empty add the default register
	if ( regs.count() == 0 )
		regs << '\"';

	if(i>=inputs.length())
		return NO_COMMAND_YET;

	// collect all the commands
#if QT_VERSION < 0x040000
	QPtrList<const YZCommand> cmds, prevcmds;
	cmds.setAutoDelete(false);
	prevcmds.setAutoDelete(false);
#else
	QList<YZCommand*> cmds, prevcmds;
#endif

	unsigned int j=i;

	// retrieve all the matching commands
	// .. first the ones whose first key matches
	if(j<inputs.length()) {
#if QT_VERSION < 0x040000
		for(commands.first(); commands.current(); commands.next())
			if(commands.current()->keySeq().startsWith(inputs.mid(j,1)))
				cmds.append(commands.current());
#else
		for (int ab = 0; ab < commands.size(); ++ab )
			if (commands.at(ab)->keySeq().startsWith(inputs.mid(j,1)))
				cmds.append(commands.at(ab));
#endif
	}
	j++;
	// .. then the ones whose next keys match, too
	while(!cmds.isEmpty() && ++j<=inputs.length()) {
		prevcmds=cmds;
#if QT_VERSION < 0x040000
		// delete all the commands that don't match
		for(cmds.first(); cmds.current();)
			if(cmds.current()->keySeq().startsWith(inputs.mid(i,j-i)))
				cmds.next();
			else
				cmds.remove();
#else
		// delete all the commands that don't match
		for ( int bc = 0 ; bc < cmds.size() ;  )
			if(cmds.at(bc)->keySeq().startsWith(inputs.mid(i,j-i)))
				++bc;
			else
				cmds.removeAt(bc);
#endif
	}
	if(cmds.isEmpty()) {
		// perhaps it is a command with an argument, isolate all those
#if QT_VERSION < 0x040000
		for(prevcmds.first(); prevcmds.current();)
			if(prevcmds.current()->arg() == ARG_NONE)
				prevcmds.remove();
			else
				prevcmds.next();
#else
		for ( int bc = 0 ; bc < prevcmds.size() ; )
			if ( prevcmds.at(bc)->arg() == ARG_NONE )
				prevcmds.removeAt(bc);
			else
				++bc;
#endif
		if(prevcmds.isEmpty())
			return CMD_ERROR;
		// it really is a command with an argument, read it in
		const YZCommand *c=prevcmds.first();
		i=j-1;
		// read in a count that may follow
		if (c->arg() == ARG_CHAR) {//dont try to read a motion !
			(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount, inputs.mid(i)));
			return CMD_OK;
		}
		if(inputs.at(i).digitValue() > 0) {
			while(j<inputs.length() && inputs.at(j).digitValue() > 0)
				j++;
			count*=inputs.mid(i,j-i).toInt();
			i=j;
			if(i>=inputs.length() )
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
#if QT_VERSION < 0x040000
				// motion, look for a motion that matches exactly
				for(commands.first(); commands.current(); commands.next()) {
					const YZMotion *m=dynamic_cast<const YZMotion*>(commands.current());
					if(m && m->matches(s))
						break;
				}
#else
				bool matched = false;
				for (int ab = 0; ab < commands.size(); ++ab) {
					const YZMotion *m = dynamic_cast<const YZMotion*>(commands.at(ab));
					if (m && m->matches(s)) {
						matched = true;
						break;
					}
				}
#endif
#if QT_VERSION < 0x040000
				if(!commands.current()) {
					// look for an incomplete motion
					for(commands.first(); commands.current(); commands.next()) {
						const YZMotion *m=dynamic_cast<const YZMotion*>(commands.current());
						if(m && m->matches(s, false))
							return OPERATOR_PENDING;
					}
					return CMD_ERROR;
				}
#else
				if (!matched) {
					for (int ab = 0; ab < commands.size(); ++ab ) {
						const YZMotion *m=dynamic_cast<const YZMotion*>(commands.at(ab));
						if(m && m->matches(s, false))
							return OPERATOR_PENDING;

					}
				}
#endif
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

#if QT_VERSION < 0x040000
		for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
			it->setPaintAutoCommit( false );
#else
		for ( int bc = 0; bc < view->myBuffer()->views().size(); ++bc )
			view->myBuffer()->views().at(bc)->setPaintAutoCommit(false);
#endif

		(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount, s));

#if QT_VERSION < 0x040000
		for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
			it->commitPaintEvent();
#else
		for ( int bc = 0; bc < view->myBuffer()->views().size(); ++bc )
			view->myBuffer()->views().at(bc)->commitPaintEvent();
#endif

	} else {
		// keep the commands that match exactly
		QString s=inputs.mid(i);
#if QT_VERSION < 0x040000
		for(cmds.first();cmds.current();) {
			if(cmds.current()->keySeq()!=s)
				cmds.remove();
			else
				cmds.next();
		}
#else
		for ( int ab = 0 ; ab < cmds.size(); ) {
			if (cmds.at(ab)->keySeq()!=s)
				cmds.removeAt(ab);
			else
				++ab;
		}
#endif
		if(cmds.isEmpty())
			return NO_COMMAND_YET;
		const YZCommand *c=0;
		if(cmds.count()==1) {
			c=cmds.first();
			if(c->arg() == ARG_NONE)
				(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount));
			else
				return OPERATOR_PENDING;
		} else {
			/* two or more commands with the same name, we assert that these are exactly
			a cmd that needs a motion and one without an argument. In visual mode, we take
			the operator, in normal mode, we take the other. */
			//this is not sufficient, see the 'q' (record macro command), we need a q+ARG_CHAR and a 'q' commands //mm //FIXME
#if QT_VERSION < 0x040000
			for(cmds.first();cmds.current();cmds.next()) {
				if(cmds.current()->arg() == ARG_MOTION && visual ||
				        cmds.current()->arg() == ARG_NONE && !visual)
					c=cmds.current();
			}
#else
			for ( int ab = 0 ; ab < cmds.size(); ++ab ) {
				if ( cmds.at(ab)->arg() == ARG_MOTION && visual ||
						cmds.at(ab)->arg() == ARG_NONE && !visual )
					c = cmds.at(ab);
			}
#endif
			if(!c)
				return CMD_ERROR;
#if QT_VERSION < 0x040000
			for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
				it->setPaintAutoCommit( false );
#else
			for ( int bc = 0; bc < view->myBuffer()->views().size(); ++bc )
				view->myBuffer()->views().at(bc)->setPaintAutoCommit(false);
#endif
			(this->*(c->poolMethod()))(YZCommandArgs(c, view, regs, count, hadCount, QString::null));
#if QT_VERSION < 0x040000
			for ( it = view->myBuffer()->views().first(); it; it = view->myBuffer()->views().next() )
				it->commitPaintEvent();
#else
			for ( int bc = 0; bc < view->myBuffer()->views().size(); ++bc )
				view->myBuffer()->views().at(bc)->commitPaintEvent();
#endif
		}
	}

	return CMD_OK;
}

bool YZMotion::matches(const QString &s, bool fully) const {
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
			if(s.length() == ks.length()+1 && isMark(s.at(s.length()-1)) || !fully && s.length() == ks.length())
				return true;
			break;
		default:
			break;
		}
	} else if(!fully && ks.startsWith(s))
		return true;

	return false;
}

YZCursor YZModeCommand::move(YZView *view, const QString &inputs, unsigned int count, bool usercount) {
#if QT_VERSION < 0x040000
	for(commands.first(); commands.current(); commands.next()) {
		// is the command a motion and does it match to the string?
		const YZMotion *m=dynamic_cast<const YZMotion*>(commands.current());
		if(m && m->matches(inputs)) {
			// execute the corresponding method
			YZCursor to=(this->*(m->motionMethod()))(YZMotionArgs(view, count, inputs.right( m->keySeq().length()), 
					inputs.left(m->keySeq().length()), usercount ));
			return to;
		}
	}
#else
	for (int ab = 0 ; ab < commands.size(); ++ab ) {
		const YZMotion *m=dynamic_cast<const YZMotion*>(commands.at(ab));
		if(m && m->matches(inputs)) {
			// execute the corresponding method
			YZCursor to=(this->*(m->motionMethod()))(YZMotionArgs(view, count, inputs.right( m->keySeq().length()), 
					inputs.left(m->keySeq().length()), usercount ));
			return to;
		}
	}
#endif
	return *view->getBufferCursor();
}


// MOTIONS

YZCursor YZModeCommand::moveLeft(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveLeft(&viewCursor, args.count, false, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::moveRight(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveRight(&viewCursor, args.count, false, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::moveLeftWrap( const YZMotionArgs & args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveLeft(&viewCursor, args.count, true, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::moveRightWrap( const YZMotionArgs & args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveRight(&viewCursor, args.count, true, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::moveDown(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	if ( args.standalone )
		args.view->moveDown(&viewCursor, args.count, true );
	else {//LINEWISE
		//update starting point
		args.view->gotoxy( 0, viewCursor.bufferY(), false );
		// end point
		args.view->moveDown( &viewCursor, args.count + 1, false );
		args.view->moveToStartOfLine( &viewCursor, true );
	}
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::moveUp(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	if ( args.standalone )
		args.view->moveUp(&viewCursor, args.count, true );
	else {//LINEWISE
		//update starting point
		if ( viewCursor.bufferY() == args.view->myBuffer()->lineCount() - 1 )
			args.view->moveToEndOfLine( &viewCursor, false );
		else
			args.view->gotoxy( 0, viewCursor.bufferY() + 1, false );
		// end point
		args.view->moveUp( &viewCursor, args.count, false );
		args.view->gotoxy ( &viewCursor, 0, viewCursor.bufferY(), true );
	}
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::movePageUp(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveUp(&viewCursor, args.view->getLinesVisible(), args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::movePageDown(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveDown(&viewCursor, args.view->getLinesVisible(), args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::matchPair(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	bool found = false;
	YZCursor pos = args.view->myBuffer()->action()->match( args.view, *viewCursor.buffer(), &found );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::findNext(const YZMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->forward( args.arg, found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZModeCommand::findBeforeNext(const YZMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->forwardBefore( args.arg, found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZModeCommand::findPrevious(const YZMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->reverse( args.arg, found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZModeCommand::findAfterPrevious(const YZMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->reverseAfter( args.arg, found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZModeCommand::repeatFind(const YZMotionArgs &args) {
	YZLineSearch* finder = args.view->myLineSearch();
	bool found;
	YZCursor pos = finder->searchAgain( found, args.count );
	if ( found ) {
		if ( args.standalone ) 
			args.view->gotoxyAndStick( &pos );
		return pos;
	}
	return *args.view->getBufferCursor();
}

YZCursor YZModeCommand::gotoSOL(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToStartOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::gotoEOL(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToEndOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::moveWordForward(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	YZCursor result( viewCursor.buffer() );
	unsigned int c = 0;
	QRegExp rex1("^\\w+\\s*");//a word with boundaries
	QRegExp rex2("^[^\\w\\s]+\\s*");//non-word chars with boundaries
	QRegExp ws("^\\s+");//whitespace
	bool wrapped = false;

	while ( c < args.count ) { //for each word
		const QString& current = args.view->myBuffer()->textline( result.getY() );
//		if ( current.isNull() ) return false; //be safe ?

#if QT_VERSION < 0x040000
		int idx = rex1.search( current, result.getX(), QRegExp::CaretAtOffset );
#else
		int idx = rex1.indexIn( current, result.getX(), QRegExp::CaretAtOffset );
#endif
		int len = rex1.matchedLength();
		if ( idx == 0 && wrapped )
			len = 0;
		if ( idx == -1 ) {
#if QT_VERSION < 0x040000
			idx = rex2.search( current, result.getX(), QRegExp::CaretAtOffset );
#else
			idx = rex2.indexIn( current, result.getX(), QRegExp::CaretAtOffset );
#endif
			len = rex2.matchedLength();
		}
		if ( idx == -1 ) {
#if QT_VERSION < 0x040000
			idx = ws.search( current, result.getX(), QRegExp::CaretAtOffset );
#else
			idx = ws.indexIn( current, result.getX(), QRegExp::CaretAtOffset );
#endif
			len = ws.matchedLength();
		}
		if ( idx != -1 ) {
			yzDebug() << "Match at " << idx << " Matched length " << len << endl;
			c++; //one match
			result.setX( idx + len );
			if ( ( c < args.count || args.standalone ) && result.getX() == current.length() && result.getY() < args.view->myBuffer()->lineCount() - 1) {
				result.setY(result.getY() + 1);
#if QT_VERSION < 0x040000
				ws.search(args.view->myBuffer()->textline( result.getY() ));
#else
				ws.indexIn(args.view->myBuffer()->textline( result.getY() ));
#endif
				result.setX( qMax( ws.matchedLength(), 0 ));
			}
		} else {
			if ( result.getY() >= args.view->myBuffer()->lineCount() - 1 ) {
				result.setX( current.length() );
				break;
			}
			result.setX(0);
			result.setY( result.getY() + 1 );
			wrapped = true;
		}

	}
	if ( args.standalone )
		args.view->gotoxyAndStick( &result );

	return result;
}


YZCursor YZModeCommand::moveSWordForward(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	YZCursor result( viewCursor.buffer() );
	unsigned int c = 0;
	QRegExp ws("\\s+");//whitespace

	while ( c < args.count ) { //for each word
		const QString& current = args.view->myBuffer()->textline( result.getY() );
//		if ( current.isNull() ) return false; //be safe ?

#if QT_VERSION < 0x040000
		int idx = ws.search( current, result.getX(), QRegExp::CaretAtOffset );
#else
		int idx = ws.indexIn( current, result.getX(), QRegExp::CaretAtOffset );
#endif
		int len = ws.matchedLength();

		if ( idx != -1 ) {
			yzDebug() << "Match at " << idx << " Matched length " << len << endl;
			c++; //one match
			result.setX( idx + len );
			if ( ( c < args.count || args.standalone ) && result.getX() == current.length() && result.getY() < args.view->myBuffer()->lineCount() - 1) {
				result.setY(result.getY() + 1);
#if QT_VERSION < 0x040000
				ws.search(args.view->myBuffer()->textline( result.getY() ));
#else
				ws.indexIn(args.view->myBuffer()->textline( result.getY() ));
#endif
				result.setX( qMax( ws.matchedLength(), 0 ));
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
	if ( args.standalone )
		args.view->gotoxyAndStick( &result );

	return result;
}


QString invertQString( const QString& from ) {
	QString res = "";
	for ( int i = from.length() - 1 ; i >= 0; i-- )
		res.append( from[ i ] );
	return res;
}

YZCursor YZModeCommand::moveWordBackward(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	YZCursor result( viewCursor.buffer() );
	unsigned int c = 0;
	QRegExp rex1("^(\\w+)\\s*");//a word with boundaries
	QRegExp rex2("^([^\\w\\s]+)\\s*");//non-word chars with boundaries
	QRegExp rex3("^\\s+([^\\w\\s$]+|\\w+)");//whitespace
	bool wrapped = false;

	while ( c < args.count ) { //for each word
		const QString& current = invertQString( args.view->myBuffer()->textline( result.getY() ) );
		int lineLength = current.length();
		int offset = lineLength - result.getX();
		yzDebug() << current << " at " << offset << endl;


#if QT_VERSION < 0x040000
		int idx = rex1.search( current, offset , QRegExp::CaretAtOffset );
#else
		int idx = rex1.indexIn( current, offset , QRegExp::CaretAtOffset );
#endif
		int len = rex1.cap( 1 ).length();
		yzDebug() << "rex1 : " << idx << "," << len << endl;
		if ( idx == -1 ) {
#if QT_VERSION < 0x040000
			idx = rex2.search( current, offset, QRegExp::CaretAtOffset );
#else
			idx = rex2.indexIn( current, offset, QRegExp::CaretAtOffset );
#endif
			len = rex2.cap( 1 ).length();
			yzDebug() << "rex2 : " << idx << "," << len << endl;
			if ( idx == -1 ) {
#if QT_VERSION < 0x040000
				idx = rex3.search( current, offset, QRegExp::CaretAtOffset );
#else
				idx = rex3.indexIn( current, offset, QRegExp::CaretAtOffset );
#endif
				len = rex3.matchedLength();
				yzDebug() << "rex3 : " << idx << "," << len << endl;
			}
		}
		if ( wrapped && lineLength == 0 ) {
			idx = 0;
			len = 0;
		}
		if ( idx != -1 ) {
			yzDebug() << "Match at " << idx << " = " << lineLength - idx << " Matched length " << len << endl;
			c++; //one match
			result.setX( lineLength - idx - len );
		} else {
			if ( result.getY() == 0 ) break; //stop here
			yzDebug() << "Previous line " << result.getY() - 1 << endl;
			const QString& ncurrent = args.view->myBuffer()->textline( result.getY() - 1 );
			wrapped = true;
			result.setX( ncurrent.length() );
			result.setY( result.getY() - 1 );
		}

	}

	if ( args.standalone )
		args.view->gotoxyAndStick( &result );

	return result;
}


YZCursor YZModeCommand::moveSWordBackward(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	YZCursor result( viewCursor.buffer() );
	unsigned int c = 0;
	QRegExp rex1("([\\S]+)\\s*"); //

	while ( c < args.count ) { //for each word
		const QString& current = invertQString( args.view->myBuffer()->textline( result.getY() ) );
		int lineLength = current.length();
		int offset = lineLength - result.getX();
		yzDebug() << current << " at " << offset << endl;


#if QT_VERSION < 0x040000
		int idx = rex1.search( current, offset , QRegExp::CaretAtOffset );
#else
		int idx = rex1.indexIn( current, offset , QRegExp::CaretAtOffset );
#endif
		int len = rex1.cap( 1 ).length();

		yzDebug() << "rex1 : " << idx << "," << len << endl;
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

	if ( args.standalone )
		args.view->gotoxyAndStick( &result );

	return result;
}

YZCursor YZModeCommand::firstNonBlank(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveToFirstNonBlankOfLine(&viewCursor,args.standalone);
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::gotoMark( const YZMotionArgs &args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	bool found = false;
	YZCursorPos pos = args.view->myBuffer()->viewMarks()->get( args.arg, &found );
	if ( found )
		return *pos.bPos;
	else {
		yzDebug() << "WARNING! mark " << args.arg << " not found" << endl;
		return *viewCursor.buffer();
	}
}

YZCursor YZModeCommand::firstNonBlankNextLine( const YZMotionArgs &args ) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->moveDown(&viewCursor, args.count, args.standalone );
	args.view->moveToFirstNonBlankOfLine( &viewCursor, args.standalone );
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::gotoLine(const YZMotionArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	unsigned int line = 0;
	yzDebug() << "gotoLine " << args.cmd << "," << args.count << endl;
	if ( args.count > 0 ) line	= args.count - 1;

	if ( args.cmd == "gg"  || ( args.cmd == "G" && args.usercount ) )
		args.view->gotoLine( &viewCursor, line, args.standalone );
	else {
		if ( args.cmd == "G" )
			args.view->gotoLastLine( &viewCursor, args.standalone );
		else
			args.view->gotoLine( &viewCursor, 0, args.standalone );
	}
	return *viewCursor.buffer();
}

YZCursor YZModeCommand::searchWord(const YZMotionArgs &args) {
	YZCursor from = *args.view->getBufferCursor();

	QString word = args.view->myBuffer()->getWordAt( from );
	if ( ! word.isNull() ) {
		yzDebug() << "searchWord : " << word << endl;
		YZCursor pos( args.view );
		bool found = true;
		bool moved = true;
		word = QRegExp::escape( word );
		if ( ! args.cmd.contains( 'g' ) ) {
			if ( word[ 0 ].isLetterOrNumber() || word[ 0 ] == '_' ) // \w
				word = "\\b" + word + "\\b";
			else
				word = word + "(?=[\\s\\w]|$)";
//				word = "(?=^|[\\s\\w])" + word + "(?=[\\s\\w]|$)"; seems that positive lookahead cannot work together...
		}
		for ( unsigned int i = 0; found && i < args.count; i++ ) {
			if ( args.cmd.contains('*') ) {
				pos = YZSession::me->search()->forward( args.view, word, &found, &from );
			} else {
				pos = YZSession::me->search()->backward( args.view, word, &found, &from );
			}
			if ( found ) {
				from.setCursor( pos );
				moved = true;
			}
		}
		if ( args.standalone && moved ) args.view->gotoxyAndStick( &from );
	}
	return from;
}

YZCursor YZModeCommand::searchNext(const YZMotionArgs &args) {
	YZCursor from = *args.view->getBufferCursor();
	YZCursor pos( args.view );
	bool found = true;
	bool moved = true;
	for ( unsigned int i = 0; found && i < args.count; i++ ) {
		pos = YZSession::me->search()->replayForward( args.view, &found, &from );
		if ( found ) {
			from.setCursor( pos );
			moved = true;
		}
	}
	if ( args.standalone && moved ) args.view->gotoxyAndStick( &from );
	return from;
}

YZCursor YZModeCommand::searchPrev(const YZMotionArgs &args) {
	YZCursor from = *args.view->getBufferCursor();
	YZCursor pos( args.view );
	bool found = true;
	bool moved = false;
	for ( unsigned int i = 0; found && i < args.count; i++ ) {
		pos = YZSession::me->search()->replayBackward( args.view, &found, &from );
		if ( found ) {
			from.setCursor( pos );
			moved = true;
		}
	}
	if ( args.standalone && moved ) args.view->gotoxyAndStick( &from );
	return from;
}

// COMMANDS

void YZModeCommand::execMotion( const YZCommandArgs &args ) {
	const YZMotion *m=dynamic_cast<const YZMotion*>(args.cmd);
	assert(m);
	YZCursor to = (this->*(m->motionMethod()))(YZMotionArgs(args.view, args.count, args.arg, args.cmd->keySeq(), args.usercount, true));
	args.view->gotoxy(to.getX(), to.getY());
	
}

YZInterval YZModeCommand::interval(const YZCommandArgs& args) {
	YZCursor from( *args.view->getBufferCursor() );
	YZCursor to = move( args.view, args.arg, args.count, args.usercount );
	if ( from > to ) {
		YZCursor tmp( from );
		from.setCursor( to );
		to.setCursor( tmp );
	}
	bool entireLines = ( args.arg.length() > 0 && args.arg[ 0 ] == QChar('\'') );
	if ( entireLines ) {
		from.setX( 0 );
		to.setX( 0 );
		to.setY( to.getY() + 1 );
	}
	YZInterval ret( from, YZBound(to, true) );
	return ret;
}

void YZModeCommand::appendAtEOL(const YZCommandArgs &args) {
	args.view->moveToEndOfLine();
	args.view->append();
	
}

void YZModeCommand::append(const YZCommandArgs &args) {
	args.view->append();
	
}

void YZModeCommand::change(const YZCommandArgs &args) {
	YZCursor cur = args.view->getBufferCursor();

	YZInterval area = interval( args );
	yzDebug() << "YZModeCommand::change " << area << endl;
	args.view->myBuffer()->action()->deleteArea(args.view, area, args.regs);
	args.view->commitNextUndo();

	// start insert mode, append if at EOL
	if ( (unsigned int)cur.getX() != args.view->myBuffer()->getLineLength( cur.getY() ) )
		args.view->modePool()->change( YZMode::MODE_INSERT );
	else
		args.view->append();
}

void YZModeCommand::changeLine(const YZCommandArgs &args) {
	args.view->myBuffer()->action()->deleteLine(args.view, *args.view->getBufferCursor(), args.count, args.regs);
	args.view->myBuffer()->action()->insertNewLine( args.view, 0, args.view->getBufferCursor()->getY() );
	args.view->modePool()->push( YZMode::MODE_INSERT );
	args.view->commitNextUndo();
	
}

void YZModeCommand::changeToEOL(const YZCommandArgs &args) {
	YZCursor to=move(args.view, "$", 1, false);
	args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	args.view->append();
	args.view->commitNextUndo();
	
}

void YZModeCommand::deleteLine(const YZCommandArgs &args) {
	args.view->myBuffer()->action()->deleteLine(args.view, *args.view->getBufferCursor(), args.count, args.regs);
	args.view->commitNextUndo();
	
}

void YZModeCommand::deleteToEOL(const YZCommandArgs &args) {
	//in vim : 2d$ does not behave as d$d$, this is illogical ..., you cannot delete twice to end of line ...
	YZCursor to=move(args.view, "$", 1, false);
	args.view->myBuffer()->action()->deleteArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	args.view->commitNextUndo();
	
}



void YZModeCommand::insertAtSOL(const YZCommandArgs &args) {
	args.view->moveToFirstNonBlankOfLine();
	args.view->modePool()->push( YZMode::MODE_INSERT );
	
}

void YZModeCommand::gotoCommandMode(const YZCommandArgs &args) {
	args.view->modePool()->pop( YZMode::MODE_COMMAND );
}

void YZModeCommand::gotoLineAtTop(const YZCommandArgs &args) {
	unsigned int line;

	line = ( args.usercount ) ? args.count - 1 : args.view->drawLineNumber() - 1;
	args.view->alignViewVertically( line );
	args.view->gotoLine( line );
	args.view->moveToFirstNonBlankOfLine();
	
}

void YZModeCommand::gotoLineAtCenter(const YZCommandArgs &args) {
	unsigned int line;

	line = ( args.usercount ) ? args.count - 1 : args.view->drawLineNumber() - 1;
	args.view->centerViewVertically( line );
	args.view->gotoLine( line );
	args.view->moveToFirstNonBlankOfLine();
	
}

void YZModeCommand::gotoLineAtBottom(const YZCommandArgs &args) {
	unsigned int line;
	//unsigned int linesFromCenter;

	line = ( args.usercount ) ? args.count - 1 : args.view->drawLineNumber() - 1;
	//if ( line > linesFromCenter ) {
		args.view->bottomViewVertically( line );
	//}
	args.view->gotoLine( line );
	args.view->moveToFirstNonBlankOfLine();
	
}


void YZModeCommand::gotoExMode(const YZCommandArgs &args) {
	args.view->modePool()->push( YZMode::MODE_EX );
}
void YZModeCommand::gotoInsertMode(const YZCommandArgs &args) {
	args.view->modePool()->push( YZMode::MODE_INSERT );
}
void YZModeCommand::gotoReplaceMode(const YZCommandArgs &args) {
	args.view->modePool()->push( YZMode::MODE_REPLACE );
}
void YZModeCommand::gotoVisualMode(const YZCommandArgs &args) {
	args.view->modePool()->push( YZMode::MODE_VISUAL );
}
void YZModeCommand::gotoVisualLineMode(const YZCommandArgs &args) {
	args.view->modePool()->push( YZMode::MODE_VISUAL_LINE );
}

void YZModeCommand::insertLineAfter(const YZCommandArgs &args) {
	unsigned int y = args.view->getBufferCursor()->getY();
	args.view->myBuffer()->action()->insertNewLine( args.view, args.view->myBuffer()->textline( y ).length(), y );
	for ( unsigned int i = 1 ; i < args.count ; i++ )
		args.view->myBuffer()->action()->insertNewLine( args.view, 0, y + i );
	args.view->modePool()->push( YZMode::MODE_INSERT );
	args.view->commitNextUndo();
	
}

void YZModeCommand::insertLineBefore(const YZCommandArgs &args) {
	unsigned int y = args.view->getBufferCursor()->getY();
	for ( unsigned int i = 0 ; i < args.count ; i++ )
		args.view->myBuffer()->action()->insertNewLine( args.view, 0, y );
	args.view->moveUp();
	args.view->modePool()->push( YZMode::MODE_INSERT );
	args.view->commitNextUndo();
	
}

void YZModeCommand::joinLine(const YZCommandArgs &args) {
	for ( unsigned int i = 0; i < args.count; i++ ) 
		args.view->myBuffer()->action()->mergeNextLine( args.view, args.view->getBufferCursor()->getY(), true );
	args.view->commitNextUndo();
}

void YZModeCommand::pasteAfter(const YZCommandArgs &args) {
	for ( unsigned int i = 0 ; i < args.count ; i++ )
		args.view->paste( args.regs[ 0 ], true );
	args.view->commitNextUndo();
	
}

void YZModeCommand::pasteBefore(const YZCommandArgs &args) {
	for ( unsigned int i = 0 ; i < args.count ; i++ )
		args.view->paste( args.regs[ 0 ], false );
	args.view->commitNextUndo();
	
}

void YZModeCommand::yankLine(const YZCommandArgs &args) {
	args.view->myBuffer()->action()->copyLine( args.view, *args.view->getBufferCursor(), args.count, args.regs );
	
}

void YZModeCommand::yankToEOL(const YZCommandArgs &args) {
	YZCursor to=move(args.view, "$", 1, false);
	args.view->myBuffer()->action()->copyArea(args.view, *args.view->getBufferCursor(), to, args.regs);
	
}

void YZModeCommand::closeWithoutSaving(const YZCommandArgs &/*args*/) {
	YZSession::me->exitRequest( 0 );
	
}

void YZModeCommand::saveAndClose(const YZCommandArgs &/*args*/) {
	YZSession::me->saveBufferExit();
	
}

void YZModeCommand::searchBackwards(const YZCommandArgs &args) {
	args.view->modePool()->push( YZMode::MODE_SEARCH_BACKWARD );
	
}

void YZModeCommand::searchForwards(const YZCommandArgs &args) {
	args.view->modePool()->push( YZMode::MODE_SEARCH );
}

void YZModeCommand::del(const YZCommandArgs &args) {
	args.view->myBuffer()->action()->deleteArea( args.view, interval( args ), args.regs );
	args.view->commitNextUndo();
	args.view->modePool()->pop();
}

void YZModeCommand::yank(const YZCommandArgs &args) {
	YZCursor topLeft = args.view->getSelectionPool()->visual()->bufferMap()[0].fromPos();

	args.view->myBuffer()->action()->copyArea( args.view, interval( args ), args.regs );
	args.view->modePool()->pop();

	// move cursor to top left corner of selection (yes, this is correct behaviour :)
	args.view->gotoxy( topLeft.getX(), topLeft.getY(), true );
	args.view->updateStickyCol( );
}

void YZModeCommand::mark(const YZCommandArgs &args) {
	YZViewCursor viewCursor = args.view->viewCursor();
	args.view->myBuffer()->viewMarks()->add( args.arg, *viewCursor.buffer(), *viewCursor.screen() );
}

void YZModeCommand::undo(const YZCommandArgs &args) {
	args.view->undo( args.count );
}

void YZModeCommand::redo(const YZCommandArgs &args) {
	args.view->redo( args.count );
}

void YZModeCommand::changeCase( const YZCommandArgs &args ) {
	YZCursor pos = *args.view->getBufferCursor();
	const QString line = args.view->myBuffer()->textline( pos.getY() );
	if ( ! line.isNull() ) {
		unsigned int length = line.length();
		unsigned int end = pos.getX() + args.count;
		for ( ; pos.getX() < length && pos.getX() < end; pos.setX( pos.getX() + 1 ) ) {
#if QT_VERSION < 0x040000
			QString ch = line.at( pos.getX() );
			if ( ch != ch.lower() )
				ch = ch.lower();
			else
				ch = ch.upper();
#else
			QString ch = QString(line.at( pos.getX() ));
			if ( ch != ch.toLower() )
				ch = ch.toLower();
			else
				ch = ch.toUpper();
#endif
			args.view->myBuffer()->action()->replaceChar( args.view, pos, ch );
		}
		args.view->commitNextUndo();
	}

}

void YZModeCommand::lineToUpperCase( const YZCommandArgs &args ) {
	YZCursor pos = *args.view->getBufferCursor();
	uint i = 0;
	while ( i < args.count ) {
		const QString line = args.view->myBuffer()->textline( pos.getY() + i );
		if ( ! line.isNull() ) {
#if QT_VERSION < 0x040000
			args.view->myBuffer()->action()->replaceLine( args.view, pos.getY() + i , line.upper());
#else
			args.view->myBuffer()->action()->replaceLine( args.view, pos.getY() + i , line.toUpper());
#endif
		}
		i++;
	}
	args.view->gotoxy( 0, pos.getY() + i );
	args.view->commitNextUndo();
}

void YZModeCommand::lineToLowerCase( const YZCommandArgs &args ) {
	YZCursor pos = *args.view->getBufferCursor();
	uint i = 0;
	while ( i < args.count ) {
		const QString line = args.view->myBuffer()->textline( pos.getY() + i );
		if ( ! line.isNull() ) {
#if QT_VERSION < 0x040000
			args.view->myBuffer()->action()->replaceLine( args.view, pos.getY() + i, line.lower());
#else
			args.view->myBuffer()->action()->replaceLine( args.view, pos.getY() + i, line.toLower());
#endif
		}
		i++;
	}
	args.view->gotoxy( 0, pos.getY() + i );
	args.view->commitNextUndo();
}

void YZModeCommand::macro( const YZCommandArgs &args ) {
	if ( args.view->isRecording() )
		args.view->stopRecordMacro();
	else
		args.view->recordMacro( args.regs );
	args.view->modeChanged();

}

void YZModeCommand::replayMacro( const YZCommandArgs &args ) {
	args.view->purgeInputBuffer();
	if ( args.view->isRecording()) {
		yzDebug() << "User asked to play a macro he is currently recording, forget it !" << endl;
		if ( args.view->registersRecorded() == args.regs )
			return;
	}

	for ( unsigned int i = 0; i < args.count; i++ ) {
#if QT_VERSION < 0x040000
		QValueList<QChar>::const_iterator it = args.regs.begin(), end = args.regs.end();
		for ( ; it != end; ++it )
			args.view->sendMultipleKey(YZSession::mRegisters->getRegister(*it)[ 0 ]);
#else
		for ( int ab = 0 ; ab < args.regs.size(); ++ab)
			args.view->sendMultipleKey(YZSession::mRegisters->getRegister(args.regs.at(ab))[0]);
#endif
	}

	args.view->commitNextUndo();
	
}

void YZModeCommand::deleteChar( const YZCommandArgs &args ) {
	args.view->myBuffer()->action()->deleteChar( args.view, args.view->getBufferCursor(), args.count );
	args.view->commitNextUndo();
}

void YZModeCommand::deleteCharBackwards( const YZCommandArgs &args ) {
	YZCursor pos = args.view->getBufferCursor();
	int oldX = pos.getX();
	int newX = oldX - args.count;
	if( newX < 0 )
		newX = 0;
	int delCount = oldX - newX;
	if( delCount == 0 )
		return; // nothing to delete
	pos.setX( newX );
	args.view->myBuffer()->action()->deleteChar( args.view, pos, delCount );
	args.view->commitNextUndo();
}

void YZModeCommand::substitute( const YZCommandArgs &args ) {
	YZCursor cur = args.view->getBufferCursor();

	args.view->myBuffer()->action()->deleteChar( args.view, args.view->getBufferCursor(), args.count );
	args.view->commitNextUndo();

	// start insert mode, append if at EOL
	if ( (unsigned int)cur.getX() != args.view->myBuffer()->getLineLength( cur.getY() ) )
		args.view->modePool()->push( YZMode::MODE_INSERT );
	else
		args.view->append();
}

void YZModeCommand::redisplay( const YZCommandArgs &args ) {
	args.view->recalcScreen();
	
}

void YZModeCommand::replace( const YZCommandArgs &args ) {
	YZCursor pos = args.view->getBufferCursor();
	args.view->myBuffer()->action()->replaceChar( args.view, pos, args.arg );
	args.view->gotoxy(pos.getX(),pos.getY(),true);
	args.view->updateStickyCol();
	args.view->commitNextUndo();
	
}

void YZModeCommand::abort( const YZCommandArgs& /*args*/) {
}

void YZModeCommand::delkey( const YZCommandArgs &args ) {
	args.view->myBuffer()->action()->deleteChar( args.view, *(args.view->getBufferCursor()), 1);
	args.view->commitNextUndo();
	
}

void YZModeCommand::indent( const YZCommandArgs& args ) {
	YZInterval area = interval( args );
	unsigned int fromY = area.fromPos().getY();
	unsigned int toY = area.toPos().getY();
	if ( toY > fromY && area.to().opened() && area.toPos().getX() == 0 )
		--toY;
	unsigned int maxY = args.view->myBuffer()->lineCount() - 1;
	if ( toY > maxY ) toY = maxY;
	int factor = ( args.cmd->keySeq()[0] == '<' ? -1 : 1 ) * args.count;
	for ( unsigned int l = fromY; l <= toY; l++ ) {
		args.view->myBuffer()->action()->indentLine( args.view, l, factor );
	}
	args.view->commitNextUndo();
	args.view->modePool()->pop();
}

