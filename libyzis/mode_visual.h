/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>,
 *  Copyright (C) 2005 Erlend Hamberg <hamberg@online.no>
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

#ifndef YZ_MODE_VISUAL_H
#define YZ_MODE_VISUAL_H

#include "cursor.h"
#include "mode.h"
#include "mode_command.h"
#include "view.h"

#if QT_VERSION < 0x040000
#include <qstring.h>
#include <qmap.h>
#include <qstringlist.h>
#else
#include <QList>
#include <QStringList>
#endif

class YZCursor;
class YZMode;
class YZView;

class YZModeVisual : public YZModeCommand {
	public:
		YZModeVisual();
		virtual ~YZModeVisual();

		virtual void initCommandPool();
		virtual void initVisualCommandPool();

		virtual void enter( YZView* mView );
		virtual void leave( YZView* mView );

		virtual void cursorMoved( YZView* mView );
		virtual void toClipboard( YZView* mView );


		void commandInsert( const YZCommandArgs& args );
		void commandAppend( const YZCommandArgs& args );
	  	void gotoExMode( const YZCommandArgs& args );
	  	void movetoExMode( const YZCommandArgs& args );
	  	void movetoInsertMode( const YZCommandArgs& args );
	  	void escape( const YZCommandArgs& args );
		void translateToVisualLine( const YZCommandArgs& args );
		void yankWholeLines(const YZCommandArgs &args);

		virtual YZInterval interval(const YZCommandArgs &args);

	protected:
		virtual YZInterval buildInterval( const YZCursor& from, const YZCursor& to );
		bool mEntireLines;
};

class YZModeVisualLine : public YZModeVisual {
	public:
		YZModeVisualLine();
		virtual ~YZModeVisualLine();

		virtual void initVisualCommandPool();

		void translateToVisual( const YZCommandArgs& args );

	protected:
		virtual YZInterval buildInterval( const YZCursor& from, const YZCursor& to );
};

#endif

