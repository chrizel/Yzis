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
 * $Id$
 */

#ifndef YZ_MODE_H
#define YZ_MODE_H

#include "yzis.h"
#include "view.h"

#include <qstringlist.h>
#include <qmap.h>

class YZView;
class YZModePool;

enum cmd_state {
	/** The command does not exist */
	CMD_ERROR,
	/** The user hasn't entered a valid, non-ambigous command yet. */
	NO_COMMAND_YET,
	/** Waiting for a motion/text object. */
	OPERATOR_PENDING,
	/** The command has been successfully executed. */
	CMD_OK,
	CMD_QUIT,
};

class YZMode {
	public:
		enum modeType {
			MODE_INSERT,
			MODE_REPLACE,
			MODE_COMMAND,
			MODE_EX,
			MODE_SEARCH,
			MODE_SEARCH_BACKWARD,
			MODE_OPEN,
			MODE_INTRO,
			MODE_COMPLETION,
			MODE_VISUAL,
			MODE_VISUAL_LINE,
			MODE_VISUAL_BLOCK,
		};

		YZMode();
		virtual ~YZMode() {}

		virtual void init();
		virtual void initModifierKeys();
		virtual void enter( YZView* mView );
		virtual void leave( YZView* mView );
		virtual cmd_state execCommand( YZView* mView, const QString& key ) = 0;

		virtual void cursorMoved( YZView* mView );

		modeType type() const;
		const QString& toString() const;
		mapping_t mapMode() const;
		bool registered() const;
		void setRegistered( bool registered );
		QStringList modifierKeys();

		virtual bool isEditMode() const;

		/**
		 * returns true if we can select text using this mode
		 */
		virtual bool isSelMode() const;

		/**
		 * returns true if we can use input method in this mode
		 */
		virtual bool supportsInputMethod() const;

		/**
		 * Input Method
		 */
		virtual void imBegin( YZView* mView );
		virtual void imCompose( YZView* mView, const QString& entry );
		virtual void imEnd( YZView* mView, const QString& entry );

	protected:
		modeType mType;
		QString mString;
		bool mEditMode;
		bool mSelMode;
		bool mIM;
		mapping_t mMapMode;
		QStringList mModifierKeys;
		bool mRegistered;
};

class YZModeIntro : public YZMode {
	public:
		YZModeIntro();
		virtual ~YZModeIntro() {}

		void enter( YZView* mView );
		void leave( YZView* mView );
		cmd_state execCommand( YZView* mView, const QString& key );

};

typedef YZMode::modeType modeType;

typedef QMap<modeType, YZMode*> YZModeMap;
typedef QValueList<YZMode*> YZModeStack;

class YZModePool {
	public:
		YZModePool( YZView* view );
		virtual ~YZModePool();

		void sendKey( const QString& key, const QString& modifiers );
		void replayKey();

		/**
		 * pop current mode and push @arg mode
		 */
		void change( modeType mode, bool leave_me = true );

		/**
		 * push @arg mode
		 */
		void push( modeType mode );

		/**
		 * pop one mode (go to previous)
		 */
		void pop( bool leave_me = true );

		/**
		 * pop until current mode is @arg mode
		 */
		void pop( modeType mode );

		void registerModifierKeys();
		void unregisterModifierKeys();
		void stop();

		YZMode* current();
		modeType currentType();
	
	private :
		YZView* mView;
		QString mKey;
		QString mModifiers;
		YZModeMap mModes;
		YZModeStack stack;
		int mapMode;
		bool mRegisterKeys;
		bool mStop;
};

#endif

