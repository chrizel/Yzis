/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
 *  Copyright (C) 2004 Pascal "Poizon" Maillard <poizon@gmx.at>,
 *  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

/**
 * $Id$
 */

#ifndef YZ_MODE_EX_H
#define YZ_MODE_EX_H

#include <qregexp.h>

#include "mode.h"
#include "view.h"
#include "yzismacros.h"

class YZView;
class YZExCommand;
class YZExRange;
class YZModeEx;

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

typedef cmd_state (YZModeEx::*ExPoolMethod) (const YZExCommandArgs&);
typedef int (YZModeEx::*ExRangeMethod) (const YZExRangeArgs&);

class YZIS_EXPORT YZExRange {

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

class YZIS_EXPORT YZExCommand {

	public :
		YZExCommand( const QString& input, ExPoolMethod pm, const QStringList& longName = QStringList(), bool word = true );
		virtual ~YZExCommand() { }

		const QString & keySeq() const { return mKeySeq; }
		const QRegExp & regexp() const { return mRegexp; }
		const ExPoolMethod& poolMethod() const { return mPoolMethod; }

	private :
		QRegExp mRegexp;
		QString mKeySeq;
		QStringList mLongName;
		ExPoolMethod mPoolMethod;

};

class YZHistory;

class YZIS_EXPORT YZModeEx : public YZMode {

	public :
		YZModeEx();
		virtual ~YZModeEx();
		
		void init();
		void enter( YZView* mView );
		void leave( YZView* mView );
		
		void initPool();
		cmd_state execCommand( YZView* mView, const QString& key );
		cmd_state execExCommand( YZView* view, const QString& inputs );
		
		YZHistory *getHistory();

	private :
		YZList<const YZExCommand*> commands;
		YZList<const YZExRange*> ranges;
		YZHistory *mHistory;

		QString parseRange( const QString& inputs, YZView* view, int* range, bool* matched );

		// ranges
		int rangeLine( const YZExRangeArgs& args );
		int rangeCurrentLine( const YZExRangeArgs& args );
		int rangeLastLine( const YZExRangeArgs& args );
		int rangeMark( const YZExRangeArgs& args );
		int rangeVisual( const YZExRangeArgs& args );
		int rangeSearch( const YZExRangeArgs& args );

	public:
		// commands
		cmd_state write( const YZExCommandArgs& args );
		cmd_state quit( const YZExCommandArgs& args );
                cmd_state bufferfirst( const YZExCommandArgs& args );
                cmd_state bufferlast( const YZExCommandArgs& args );
		cmd_state buffernext( const YZExCommandArgs& args );
		cmd_state bufferprevious( const YZExCommandArgs& args );
		cmd_state bufferdelete( const YZExCommandArgs& args );
		cmd_state edit( const YZExCommandArgs& args );
		cmd_state mkyzisrc( const YZExCommandArgs& args );
		cmd_state set( const YZExCommandArgs& args );
		cmd_state substitute( const YZExCommandArgs& args );
		cmd_state hardcopy( const YZExCommandArgs& args );
		cmd_state gotoOpenMode( const YZExCommandArgs& args );
		cmd_state gotoCommandMode( const YZExCommandArgs& args );
		cmd_state preserve( const YZExCommandArgs& args );
		cmd_state lua( const YZExCommandArgs& args );
		cmd_state source( const YZExCommandArgs& args );
		cmd_state map( const YZExCommandArgs& args );
		cmd_state unmap( const YZExCommandArgs& args );
		cmd_state imap( const YZExCommandArgs& args );
		cmd_state iunmap( const YZExCommandArgs& args );
		cmd_state omap( const YZExCommandArgs& args );
		cmd_state ounmap( const YZExCommandArgs& args );
		cmd_state vmap( const YZExCommandArgs& args );
		cmd_state vunmap( const YZExCommandArgs& args );
		cmd_state cmap( const YZExCommandArgs& args );
		cmd_state cunmap( const YZExCommandArgs& args );
		cmd_state nmap( const YZExCommandArgs& args );
		cmd_state nunmap( const YZExCommandArgs& args );
		cmd_state noremap( const YZExCommandArgs& args );
		cmd_state onoremap( const YZExCommandArgs& args );
		cmd_state vnoremap( const YZExCommandArgs& args );
		cmd_state inoremap( const YZExCommandArgs& args );
		cmd_state cnoremap( const YZExCommandArgs& args );
		cmd_state nnoremap( const YZExCommandArgs& args );
		cmd_state indent( const YZExCommandArgs& args );
		cmd_state enew( const YZExCommandArgs& args );
		cmd_state syntax( const YZExCommandArgs& args );
		cmd_state highlight( const YZExCommandArgs& args );
		cmd_state registers( const YZExCommandArgs& args );
		cmd_state split( const YZExCommandArgs& args );
		cmd_state retab( const YZExCommandArgs& args );
		cmd_state genericMap( const YZExCommandArgs& args, int );
		cmd_state genericUnmap( const YZExCommandArgs& args, int );		
		cmd_state genericNoremap( const YZExCommandArgs& args, int );
		cmd_state foldCreate( const YZExCommandArgs& args );
		cmd_state cd( const YZExCommandArgs& args );
		cmd_state pwd( const YZExCommandArgs& args );
		cmd_state tag( const YZExCommandArgs& args );
		cmd_state pop( const YZExCommandArgs& args );
		cmd_state tagnext( const YZExCommandArgs& args );
		cmd_state tagprevious( const YZExCommandArgs& args );
};


#endif

