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
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id: /yzis/trunk/libyzis/mode_insert.h 142 2005-06-03T19:19:14.776723Z svm  $
 */

#ifndef YZ_MODE_COMPLETE_H
#define YZ_MODE_COMPLETE_H

#include "mode.h"

class YZBuffer;
class YZCursor;

class YZModeCompletion : public YZMode {
	public :
		YZModeCompletion();
		virtual ~YZModeCompletion();

		virtual void leave( YZView* mView );
		virtual cmd_state execCommand( YZView* mView, const QString& _key );

	protected :
		void doComplete( YZView* mView, bool forward );
		bool initCompletion( YZView* mView, bool forward );
		
	private :
		void completeFromBuffer( YZBuffer *buffer, bool forward, const YZCursor &initBegin, const YZCursor &initEnd, bool doWrap, QStringList &proposed );
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

