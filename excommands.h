/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>,
 *  Thomas Capricelli <orzel@freehackers.org>,
 *  Philippe Fremy <phil@freehackers.org>,
 *  Loic Pauleve <panard@inzenet.org>
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

#ifndef YZ_EXCOMMANDS_H
#define YZ_EXCOMMANDS_H

#include <qregexp.h>

#include "view.h"

class YZView;
class YZExCommand;
class YZExRange;
class YZExCommandPool;

struct YZExRangeArgs {
	const YZExRange* cmd;
	YZView* view;
	QString arg;

	YZExRangeArgs( const YZExRange* _cmd, YZView* _view, const QString& a ) {
		cmd = _cmd;
		view = _view;
		arg = a;
	}
};

struct YZExCommandArgs {
	// caller view
	YZView* view;
	// all input
	QString input;
	// command to execute
	QString cmd;
	// arguments
	QString arg;
	// range
	unsigned int fromLine;
	unsigned int toLine;
	// !
	bool force;

	YZExCommandArgs( YZView* _view, const QString& _input, const QString& _cmd, const QString& _arg, unsigned int _fromLine, unsigned int _toLine, bool _force ) {
		input = _input;
		cmd = _cmd;
		arg = _arg;
		view = _view;
		fromLine = _fromLine;
		toLine = _toLine;
		force = _force;
	}
};

typedef QString (YZExCommandPool::*ExPoolMethod) (const YZExCommandArgs&);
typedef int (YZExCommandPool::*ExRangeMethod) (const YZExRangeArgs&);

class YZExRange {

	public :
		YZExRange( const QString& regexp, ExRangeMethod pm );
		virtual ~YZExRange() { }
		
		QString keySeq() const { return mKeySeq; }
		const ExRangeMethod& poolMethod() const { return mPoolMethod; }
		const QRegExp & regexp() const { return mRegexp; }

	private :
		QRegExp mRegexp;
		QString mKeySeq;
		ExRangeMethod mPoolMethod;

};

class YZExCommand {

	public :
		YZExCommand( const QString& input, ExPoolMethod pm, bool word = true );
		virtual ~YZExCommand() { }

		const QString & keySeq() const { return mKeySeq; }
		const QRegExp & regexp() const { return mRegexp; }
		const ExPoolMethod& poolMethod() const { return mPoolMethod; }

	private :
		QRegExp mRegexp;
		QString mKeySeq;
		ExPoolMethod mPoolMethod;

};

class YZExCommandPool {

	public :
		YZExCommandPool();
		virtual ~YZExCommandPool();
		
		void initPool();
		bool execCommand( YZView* view, const QString& inputs );

	private :
		QPtrList<const YZExCommand> commands;
		QPtrList<const YZExRange> ranges;

		QString parseRange( const QString& inputs, YZView* view, int* range, bool* matched );

		// ranges
		int rangeLine( const YZExRangeArgs& args );
		int rangeCurrentLine( const YZExRangeArgs& args );
		int rangeLastLine( const YZExRangeArgs& args );
		int rangeMark( const YZExRangeArgs& args );
		int rangeVisual( const YZExRangeArgs& args );
		int rangeSearch( const YZExRangeArgs& args );

		// commands
		QString write( const YZExCommandArgs& args );
		QString quit( const YZExCommandArgs& args );
		QString buffernext( const YZExCommandArgs& args );
		QString bufferprevious( const YZExCommandArgs& args );
		QString bufferdelete( const YZExCommandArgs& args );
		QString edit( const YZExCommandArgs& args );
		QString mkyzisrc( const YZExCommandArgs& args );
		QString setlocal( const YZExCommandArgs& args );
		QString set( const YZExCommandArgs& args );
		QString substitute( const YZExCommandArgs& args );
		QString hardcopy( const YZExCommandArgs& args );
		QString gotoOpenMode( const YZExCommandArgs& args );
		QString gotoCommandMode( const YZExCommandArgs& args );
		QString preserve( const YZExCommandArgs& args );
		QString lua( const YZExCommandArgs& args );
		QString source( const YZExCommandArgs& args );
		QString map( const YZExCommandArgs& args );
		QString imap( const YZExCommandArgs& args );
		QString indent( const YZExCommandArgs& args );
};

#endif

