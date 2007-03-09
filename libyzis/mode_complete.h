/*  This file is part of the Yzis libraries
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>
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

#ifndef YZ_MODE_COMPLETE_H
#define YZ_MODE_COMPLETE_H

/* Qt */
#include <QList>

/* yzis */
#include "mode.h"
#include "cursor.h"
#include "yzismacros.h"

class YZBuffer;

/**
  * @short Completion mode.
  */
class YZIS_EXPORT YZModeCompletion : public YZMode {
	public :
		YZModeCompletion();
		virtual ~YZModeCompletion();

		virtual void leave( YZView* mView );
		virtual cmd_state execCommand( YZView* mView, const QString& _key );

	protected :
		void doComplete( YZView* mView, bool forward );
		bool initCompletion( YZView* mView, bool forward );
		
	private :
		void completeFromBuffer( YZBuffer *buffer, QStringList &proposed, bool elimDups = true, QList<YZCursor> *cursors = NULL );
		void completeFromCurrentBuffer( const YZCursor cursor, bool forward, QStringList &proposed );
		void completeFromOtherBuffers( YZBuffer *skip, QStringList &proposed );
		void completeFromIncludes( QStringList &proposed );
		void completeFromTags( QStringList &proposed );
		void completeFromDictionary( QStringList &proposed );
		void completeFromFileNames( QStringList &proposed );
		QString mPrefix;
		YZCursor mCompletionStart;
		YZCursor mCompletionEnd;
		
		QStringList mProposedCompletions;
		unsigned int mCurrentProposal;
		QString mLastMatch;
		
		QString mLastKey;
		bool mForward;
};

#endif

