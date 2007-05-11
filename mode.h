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

#ifndef YZ_MODE_H
#define YZ_MODE_H

/* Qt */
#include <QMap>
#include <QStringList>

/* Yzis */
#include "yzis.h"


class YZDebugStream;
class YZView;
class YZModePool;

enum CmdState {
	/** The command does not exist */
	CmdError,
	/** The user hasn't entered a valid, non-ambigous command yet. */
	CmdNotYetValid,
	/** Waiting for a motion/text object. */
	CmdOperatorPending,
	/** The command has been successfully executed. */
	CmdOk,
	/** It is time to leave the event loop */
	CmdQuit,
};

YZIS_EXPORT YZDebugStream& operator<<( YZDebugStream& out, const CmdState & state );

class YZIS_EXPORT YZMode
{
public:
	enum modeType {
		MODE_COMMAND,		   //!< default mode, the one from which you move to the following :
		MODE_INSERT,		   //!< 'i' : entering text (insert)
		MODE_REPLACE,		   //!< 'R' : entering text (replace)
		MODE_EX,		       //!< ':' : execute some yzis command
		MODE_SEARCH,		   //!< '/' : search text
		MODE_SEARCH_BACKWARD,  //!< '?' : search backward
		MODE_INTRO,		       //!< display intro text, and move to command mode on first key entered
		MODE_COMPLETION,	   //!< used from within insert mode for completion
		MODE_VISUAL,		   //!< 'v' : visual mode with characters and lines 
		MODE_VISUAL_LINE,      //!< 'V' : visual mode lines by lines
		MODE_VISUAL_BLOCK,     //!< C-V : visual mode, by blocks
	};

	YZMode();
	virtual ~YZMode() {}

	virtual void init();
	virtual void initModifierKeys();
	virtual void enter( YZView* mView );
	virtual void leave( YZView* mView );
	virtual CmdState execCommand( YZView* mView, const QString& key ) = 0;

	virtual void cursorMoved( YZView* mView );

	modeType type() const;
	const QString& toString() const;
	yzis::mapping_t mapMode() const;
	bool registered() const;
	void setRegistered( bool registered );
	QStringList modifierKeys() const;

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
	yzis::mapping_t mMapMode;
	QStringList mModifierKeys;
	bool mRegistered;
};

YZIS_EXPORT YZDebugStream& operator<<( YZDebugStream& out, const YZMode::modeType & type );


class YZModeIntro : public YZMode {
public:
	YZModeIntro();
	virtual ~YZModeIntro() {}

	void enter( YZView* mView );
	void leave( YZView* mView );
	CmdState execCommand( YZView* mView, const QString& key );

};

typedef YZMode::modeType modeType;

typedef QMap<modeType, YZMode*> YZModeMap;
typedef QList<YZMode*> YZModeStack;

class YZIS_EXPORT YZModePool
{
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

	YZMode* current() const;
	modeType currentType() const;

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

