/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

/*
 * $Id$
 */

#include "session.h"

#include "debug.h"
#include "schema.h"
#include "buffer.h"
#include "registers.h"
#include "swapfile.h"
#include "mapping.h"
#include "ex_lua.h"

#include <qapplication.h>
#include <qdir.h>

#include "mode_command.h"
#include "mode_ex.h"
#include "mode_insert.h"
#include "mode_search.h"
#include "mode_visual.h"

int YZSession::mNbViews = 0;
int YZSession::mNbBuffers = 0;
YZInternalOptionPool *YZSession::mOptions = 0;
YZRegisters *YZSession::mRegisters = 0;
YZSession *YZSession::me = 0;
YZEvents *YZSession::events = 0;
YZYzisinfo *YZSession::mYzisinfo= 0;
bool YZSession::mYzisinfoInitialized = false;
unsigned int YZSession::mCurrentExItem = 0;
unsigned int YZSession::mCurrentSearchItem = 0;
unsigned int YZSession::mCurrentJumpListItem = 0;
StringVector YZSession::mExHistory = 0;
StringVector YZSession::mSearchHistory = 0;
JumpListVector YZSession::mJumpList = 0;
StartPositionVector YZSession::mStartPosition = 0;

YZSession::YZSession( const QString& _sessionName ) {
	yzDebug() << "If you see me twice in the debug , then immediately call the police because it means yzis is damn borked ..." << endl;
	//if ( me != 0 ) int t = 5/( me - me );
	//FIXME

	initModes();
	mSearch = new YZSearch();
	mSessionName = _sessionName;
	mCurView = 0;
	mCurBuffer = 0;
	me = this;
	events = new YZEvents();
	mSchemaManager = new YzisSchemaManager();
	mOptions = new YZInternalOptionPool();
	mRegisters = new YZRegisters();
	mYzisinfo= new YZYzisinfo();
	mExHistory.resize(50);
	mSearchHistory.resize(50);
}

YZSession::~YZSession() {
	endModes();
	delete YzisHlManager::self();
	delete mSchemaManager;
	delete mSearch;
	delete events;
	delete mRegisters;
	delete mOptions;
	delete mYzisinfo;
	delete YZMapping::self();
	delete YZExLua::instance();
	delete YZDebugBackend::instance();
}

void YZSession::initModes() {
	mModes[ YZMode::MODE_INTRO ] = new YZModeIntro();
	mModes[ YZMode::MODE_COMMAND ] = new YZModeCommand();
	mModes[ YZMode::MODE_EX ] = new YZModeEx();
	mModes[ YZMode::MODE_INSERT ] = new YZModeInsert();
	mModes[ YZMode::MODE_REPLACE ] = new YZModeReplace();
	mModes[ YZMode::MODE_VISUAL ] = new YZModeVisual();
	mModes[ YZMode::MODE_VISUAL_LINE ] = new YZModeVisualLine();
	mModes[ YZMode::MODE_VISUAL_BLOCK ] = new YZModeVisualBlock();
	mModes[ YZMode::MODE_SEARCH ] = new YZModeSearch();
	mModes[ YZMode::MODE_SEARCH_BACKWARD ] = new YZModeSearchBackward();
	mModes[ YZMode::MODE_COMPLETION ] = new YZModeCompletion();
	YZModeMap::Iterator it;
	for( it = mModes.begin(); it != mModes.end(); ++it )
		it.data()->init();
}
void YZSession::endModes() {
	YZModeMap::Iterator it;
	for( it = mModes.begin(); it != mModes.end(); ++it )
		delete it.data();
	mModes.clear();
}
YZModeMap YZSession::getModes() {
	return mModes;
}
YZModeEx* YZSession::getExPool() {
	return (YZModeEx*)mModes[ YZMode::MODE_EX ];
}
YZModeCommand* YZSession::getCommandPool() {
	return (YZModeCommand*)mModes[ YZMode::MODE_COMMAND ];
}

YZYzisinfo * YZSession::getYzisinfo()
{
	return mYzisinfo;
}

void YZSession::guiStarted() {
	//read init files
	if (QFile::exists(QDir::rootDirPath() + "/etc/yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::rootDirPath() + "/etc/yzis/init.lua" );
	if (QFile::exists(QDir::homeDirPath() + "/.yzis/init.lua"))
		YZExLua::instance()->source( NULL, QDir::homeDirPath() + "/.yzis/init.lua" );
}

void YZSession::addBuffer( YZBuffer *b ) {
	yzDebug() << "Session : addBuffer " << b->fileName() << endl;
	mBuffers.insert(b->fileName(), b);
	mCurBuffer = b;
}

void YZSession::rmBuffer( YZBuffer *b ) {
//	yzDebug() << "Session : rmBuffer " << b->fileName() << endl;
	if ( mBuffers.contains( b->fileName() ) ) {
			mBuffers.remove( b->fileName() );
			deleteBuffer( b );
	}
	if ( mBuffers.isEmpty() )
		exitRequest( );
//	delete b; // kinda hot,no?
}

QString YZSession::saveBufferExit() {
	if ( saveAll() )
		quit();
	return QString::null;
}

YZView* YZSession::findView( int uid ) {
//	yzDebug() << " ========= " << endl;
//	yzDebug() << "Session::findView " << uid << endl;
	if ( uid<0 ) return NULL;
	YZBufferMap::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer *b = ( it.data() );
//		yzDebug() << "Session::findView, checking buffer " << b->fileName() << endl;
		YZView *v = b->findView( uid );
		if ( v ) return v;
	}
//	yzDebug() << "Session::findView " << uid << " not found !" << endl;
	return NULL;
}

void YZSession::setCurrentView( YZView* view ) {
	yzDebug() << "Session : setCurrentView" << endl;
	mCurView = view;
	mCurBuffer = view->myBuffer();
	changeCurrentView( view );
	mCurBuffer->filenameChanged();
}

YZView* YZSession::firstView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}

	YZView * v = NULL;
	int i = 0;
	// searching through all views
	// findView(0) is not enough, // because there may be a hole  in the begining
	// (for example, after bdel 0, bdel 1, etc )
	while (!v && i <= mNbViews  ) {
		v = findView( i );
		i++;
	}
	return v;
}

YZView* YZSession::lastView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}

	YZView * v = NULL;
	int i = mNbViews;
	// searching through all views backwards
	// findView(0) is not enough, 
	// because there may be a hole  in the end
	// (for example, after bdel <mNbViews>, <mNbViews-1> etc)
	while (!v && i >= 0  ) {
		// NOTE: Maybe add an option to findView to search backwards?
		v = findView( i );
		i--;
	}
	return v;

}

YZView* YZSession::prevView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}
//	yzDebug() << "Current view is " << mCurView->myId << endl;
	
	YZView * v = NULL;
	int i = 1;
	while (!v && i <= mNbViews  ) {
		v = findView( mCurView->myId - i );
		i++;
	}
	return v;
}

YZView* YZSession::nextView() {
	if ( mCurView == 0 ) {
		yzDebug() << "WOW, mCurview is NULL !" << endl;
		return NULL;
	}
//	yzDebug() << "Current view is " << mCurView->myId << endl;
	YZView * v = NULL;
	int i = 1;
	while (!v && i <= mNbViews) {
		v = findView( mCurView->myId + i );
		i++;
	}
	return v;
}

YZBuffer* YZSession::findBuffer( const QString& path ) {
	YZBufferMap::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer *b = ( *it );
		if ( b->fileName() == path ) return b;
	}
	return NULL; //not found
}

void YZSession::updateBufferRecord( const QString& oldname, const QString& newname, YZBuffer *buffer ) {
	mBuffers.remove( oldname );
	mBuffers.insert( newname, buffer );
}

bool YZSession::saveAll() {
	YZBufferMap::Iterator it = mBuffers.begin(), end = mBuffers.end();
	bool savedAll=true;
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		if ( !b->fileIsNew() ) {
			if ( b->fileIsModified() && !b->save() ) savedAll=false;
		}
	}
	return savedAll;
}

bool YZSession::isOneBufferModified() {
	YZBufferMap::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		if ( b->fileIsNew() ) return true;
	}
	return false;
}

bool YZSession::exitRequest( int errorCode ) {
	yzDebug() << "Preparing for final exit with code " << errorCode << endl;
	//prompt unsaved files XXX
/*	YZBufferMap::Iterator it = mBuffers.begin(), end = mBuffers.end();
	for ( ; it!=end; ++it ) {
		YZBuffer* b = ( *it );
		deleteBuffer( b );
	}
	mBuffers.clear();
	
   YZSession::me->getYzisinfo()->updateStartPosition( 
                  currentBuffer()->fileName(),
                  (YZSession::me->currentView())->getCursor()->x(),
                  (YZSession::me->currentView())->getCursor()->y() );
                                       
	YZSession::me->getYzisinfo()->writeYzisinfo();*/
                                          
	return quit( errorCode );
}

void YZSession::sendMultipleKeys ( const QString& text) {
//	QStringList list = QStringList::split("<ENTER>", text);
/*	QStringList::Iterator it = list.begin(), end = list.end();
	for (; it != end; ++it) {*/
		YZView* cView = YZSession::me->currentView();
		cView->sendMultipleKey(/* *it + "<ENTER>" */ text);
		qApp->processEvents();
/*	}*/
}

void YZSession::registerModifier ( const QString& mod ) {
	for (int i = 0 ; i < mNbViews; i++ ) {
		YZView *v = findView(i);
		if (v) {
			v->registerModifierKeys(mod);
		}
	}
}

void YZSession::unregisterModifier ( const QString& mod ) {
	for (int i = 0 ; i < mNbViews; i++ ) {
		YZView *v = findView(i);
		if (v) {
			v->unregisterModifierKeys(mod);
		}
	}
}

void YZSession::saveCursorPosition() {
	// Make sure we have a current view
	if ( ! mCurView ) {
		return;
	}
	
	// Only record our cursor position if we are in search mode or 
	// we are using the search keys
	QString last = mCurView->getLastInputBuffer();
	
	if ( YZSession::me->currentView()->modePool()->currentType() == YZMode::MODE_SEARCH 
	||   YZSession::me->currentView()->modePool()->currentType() == YZMode::MODE_SEARCH_BACKWARD 
	|| last == "n"
	|| last == "N" ) {
		mYzisinfo->updateJumpList( YZSession::me->currentBuffer()->fileName(), YZSession::me->currentView()->getCursor()->x(), YZSession::me->currentView()->getCursor()->y() );
	} else {
		return;
	}
}

YZCursor * YZSession::previousCursorPosition() {
	
	if ( mCurrentJumpListItem == 0 ) {
		mCurrentJumpListItem = mJumpList.count();
	}
	
	--mCurrentJumpListItem;
	
	if ( mJumpList[mCurrentJumpListItem]->filename() == YZSession::me->currentBuffer()->fileName() ) {
		return mJumpList[mCurrentJumpListItem]->position();
	} else {
		YZBuffer * b = findBuffer( mJumpList[mCurrentJumpListItem]->filename() );

		if ( b ) {
			setCurrentView( b->firstView() );
		} else {
			createBuffer( mJumpList[mCurrentJumpListItem]->filename() );
		}

		currentView()->gotodxdy(mJumpList[mCurrentJumpListItem]->position()->x(), mJumpList[mCurrentJumpListItem]->position()->y(), true);

		return mJumpList[mCurrentJumpListItem]->position();
	}
}
