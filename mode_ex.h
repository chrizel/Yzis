/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2004 Philippe Fremy <phil@freehackers.org>,
 *  Copyright (C) 2004 Pascal "Poizon" Maillard <poizon@gmx.at>,
 *  Copyright (C) 2004-2005 Loic Pauleve <panard@inzenet.org>
 *  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
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

#ifndef YZ_MODE_EX_H
#define YZ_MODE_EX_H

/* Qt */
#include <QRegExp>
#include <QList>

/* yzis */
#include "mode.h"
#include "yzismacros.h"

class YZView;
class YZExCommand;
class YZExRange;
class YZModeEx;

/**
 * Range argument for an Ex command
 */
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

/**
 * Command arguments for an Ex command
 */
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

	YZExCommandArgs( YZView* _view, const QString& _input, const QString& _cmd, const QString& _arg, unsigned int _fromLine, unsigned int _toLine, bool _force );
    QString toString() const;
};

typedef CmdState (YZModeEx::*ExPoolMethod) (const YZExCommandArgs&);
typedef int (YZModeEx::*ExRangeMethod) (const YZExRangeArgs&);

/**
 * Ex range
 */
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

/**
  * Command in exution mode ( as ":w" or ":q")
  */
class YZIS_EXPORT YZExCommand
{
	public :
		/**
		  * Constructor. It creates a commands
		  * @arg input is the string that is to be matched to recognise this command
		  * @arg pm is the ( static ) function that gets executed when this command is entered.
		  * @arg longName is a list of names for this command. (to be displayed to the user )
		  * @arg word is.. ?? ( used in some regexp thinguy )
		 */
		YZExCommand( const QString& input, ExPoolMethod pm, const QStringList& longName = QStringList(), bool word = true );
		virtual ~YZExCommand() { }

		const QString & keySeq() const { return mKeySeq; }
		const QRegExp & regexp() const { return mRegexp; }
		const ExPoolMethod& poolMethod() const { return mPoolMethod; }
		const QStringList & longName() const { return mLongName; }

	private :
		QRegExp mRegexp;
		QString mKeySeq;
		QStringList mLongName;
		ExPoolMethod mPoolMethod;

};

class YZHistory;

/**
  * @short Execution mode ( "command ':').
  */
class YZIS_EXPORT YZModeEx : public YZMode {

	public :
		YZModeEx();
		virtual ~YZModeEx();
		
		void init();
		void enter( YZView* mView );
		void leave( YZView* mView );
		
		void initPool();
		CmdState execCommand( YZView* mView, const QString& key );
		CmdState execExCommand( YZView* view, const QString& inputs );
		
		YZHistory *getHistory();

	private :
		QList<const YZExCommand*> commands;
		QList<const YZExRange*> ranges;
		YZHistory *mHistory;
		//completion stuff
		QStringList mCompletePossibilities;
		int mCurrentCompletionProposal;
		QString mCompletionCurrentSearch;

		/*
		 * Init the completion for the command line
		 * it will search for command names or file names
		 */
		void completeCommandLine(YZView *view);
		/*
		 * Get the list of completion items available
		 * @return a QStringList containing the completion possibilities
		 */
		const QStringList& completionList();
		/*
		 * Reset all completion system
		 */
		void resetCompletion();
		/*
		 * Get the current completion proposal
		 */
		int completionIndex();
		/*
		 * Get the completion proposal at position @arg idx in the internal list
		 * @param idx the index position
		 * @return a QString containing the completion proposal
		 */
		const QString& completionItem(int idx);

		QString parseRange( const QString& inputs, YZView* view, int* range, bool* matched );

		// ranges
		int rangeLine( const YZExRangeArgs& args );
		int rangeCurrentLine( const YZExRangeArgs& args );
		int rangeLastLine( const YZExRangeArgs& args );
		int rangeMark( const YZExRangeArgs& args );
		int rangeVisual( const YZExRangeArgs& args );
		int rangeSearch( const YZExRangeArgs& args );

	public:
		// list all full command names
		const QStringList extractCommandNames();

		// commands
		CmdState write( const YZExCommandArgs& args );
		CmdState quit( const YZExCommandArgs& args );
                CmdState bufferfirst( const YZExCommandArgs& args );
                CmdState bufferlast( const YZExCommandArgs& args );
		CmdState buffernext( const YZExCommandArgs& args );
		CmdState bufferprevious( const YZExCommandArgs& args );
		CmdState bufferdelete( const YZExCommandArgs& args );
		CmdState edit( const YZExCommandArgs& args );
		CmdState mkyzisrc( const YZExCommandArgs& args );
		CmdState set( const YZExCommandArgs& args );
		CmdState substitute( const YZExCommandArgs& args );
		CmdState hardcopy( const YZExCommandArgs& args );
		CmdState gotoOpenMode( const YZExCommandArgs& args );
		CmdState gotoCommandMode( const YZExCommandArgs& args );
		CmdState preserve( const YZExCommandArgs& args );
		CmdState lua( const YZExCommandArgs& args );
		CmdState source( const YZExCommandArgs& args );
		CmdState map( const YZExCommandArgs& args );
		CmdState unmap( const YZExCommandArgs& args );
		CmdState imap( const YZExCommandArgs& args );
		CmdState iunmap( const YZExCommandArgs& args );
		CmdState omap( const YZExCommandArgs& args );
		CmdState ounmap( const YZExCommandArgs& args );
		CmdState vmap( const YZExCommandArgs& args );
		CmdState vunmap( const YZExCommandArgs& args );
		CmdState cmap( const YZExCommandArgs& args );
		CmdState cunmap( const YZExCommandArgs& args );
		CmdState nmap( const YZExCommandArgs& args );
		CmdState nunmap( const YZExCommandArgs& args );
		CmdState noremap( const YZExCommandArgs& args );
		CmdState onoremap( const YZExCommandArgs& args );
		CmdState vnoremap( const YZExCommandArgs& args );
		CmdState inoremap( const YZExCommandArgs& args );
		CmdState cnoremap( const YZExCommandArgs& args );
		CmdState nnoremap( const YZExCommandArgs& args );
		CmdState indent( const YZExCommandArgs& args );
		CmdState enew( const YZExCommandArgs& args );
		CmdState syntax( const YZExCommandArgs& args );
		CmdState highlight( const YZExCommandArgs& args );
		CmdState registers( const YZExCommandArgs& args );
		CmdState split( const YZExCommandArgs& args );
		CmdState retab( const YZExCommandArgs& args );
		CmdState genericMap( const YZExCommandArgs& args, int );
		CmdState genericUnmap( const YZExCommandArgs& args, int );		
		CmdState genericNoremap( const YZExCommandArgs& args, int );
		CmdState foldCreate( const YZExCommandArgs& args );
		CmdState cd( const YZExCommandArgs& args );
		CmdState pwd( const YZExCommandArgs& args );
		CmdState tag( const YZExCommandArgs& args );
		CmdState pop( const YZExCommandArgs& args );
		CmdState tagnext( const YZExCommandArgs& args );
		CmdState tagprevious( const YZExCommandArgs& args );
};


#endif

