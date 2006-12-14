/*  This file is part of the Yzis libraries
 *  Copyright (C) 2004-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2005 Loic Pauleve <panard@inzenet.org>,
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

#ifndef YZ_MODE_VISUAL_H
#define YZ_MODE_VISUAL_H

#include "mode_command.h"
#include "yzismacros.h"

class YZMode;
class YZView;
class YZViewCursor;

class YZIS_EXPORT YZModeVisual : public YZModeCommand {
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
		void changeWholeLines(const YZCommandArgs &args);
		void deleteWholeLines(const YZCommandArgs &args);
		void yankWholeLines(const YZCommandArgs &args);
		void yank(const YZCommandArgs &args);
		void toUpperCase( const YZCommandArgs& args );
		void toLowerCase( const YZCommandArgs& args );
		void translateToVisual( const YZCommandArgs& args );
		void translateToVisualLine( const YZCommandArgs& args );
		void translateToVisualBlock( const YZCommandArgs& args );

		virtual YZInterval interval(const YZCommandArgs &args);

	protected:
		virtual YZInterval buildBufferInterval( YZView* mView, const YZViewCursor& from, const YZViewCursor& to );
		virtual YZInterval buildScreenInterval( YZView* mView, const YZViewCursor& from, const YZViewCursor& to );
		bool mEntireLines;
};

class YZModeVisualLine : public YZModeVisual {
	public:
		YZModeVisualLine();
		virtual ~YZModeVisualLine();

	protected:
		virtual YZInterval buildBufferInterval( YZView* mView, const YZViewCursor& from, const YZViewCursor& to );
		virtual YZInterval buildScreenInterval( YZView* mView, const YZViewCursor& from, const YZViewCursor& to );
};

class YZModeVisualBlock : public YZModeVisual {
	public:
		YZModeVisualBlock();
		virtual ~YZModeVisualBlock();

		virtual void cursorMoved( YZView* mView );
};

#endif

