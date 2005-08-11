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
#include "search.h"
#include "events.h"
#include "internal_options.h"
#include "yzisinfo.h"
#include "ex_lua.h"

#include <qapplication.h>
#include <qdir.h>

#include "mode_command.h"
#include "mode_complete.h"
#include "mode_ex.h"
#include "mode_insert.h"
#include "mode_search.h"
#include "mode_visual.h"

#include "tags_stack.h"

YZInternalOptionPool *YZSession::mOptions = 0;
YZRegisters *YZSession::mRegisters = 0;
YZSession *YZSession::me = 0;
YZEvents *YZSession::events = 0;
YZYzisinfo *YZSession::mYzisinfo= 0;
unsigned int YZSession::mCurrentExItem = 0;
unsigned int YZSession::mCurrentSearchItem = 0;
unsigned int YZSession::mCurrentJumpListItem = 0;
YZVector<QString> YZSession::mExHistory;
YZVector<QString> YZSession::mSearchHistory;
JumpListVector YZSession::mJumpList;
StartPositionVector YZSession::mStartPosition;
YZTagStack YZSession::mTagStack;

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
	mExHistory.resize(200);
	mSearchHistory.resize(200);
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
	mBufferList.push_back( b );
}

void YZSession::rmBuffer( YZBuffer *b ) {
//	yzDebug() << "Session : rmBuffer " << b->fileName() << endl;
	if ( mBufferList.find( b ) != mBufferList.end() ) {
			mBufferList.remove( b );
			deleteBuffer( b );
	}
	if ( mBufferList.empty() )
		exitRequest( );
//	delete b; // kinda hot,no?
}

QString YZSession::saveBufferExit() {
	if ( saveAll() )
		quit();
	return QString::null;
}

YZView* YZSession::findView( const YZViewId &id ) {
	YZView *result = NULL;
	
	for ( YZViewList::Iterator i = mViewList.begin(); i != mViewList.end(); ++i ) {
		if ( (*i)->getId() == id ) {
			result = *i;
			break;
		}
	}
	
	return result;
}

void YZSession::setCurrentView( YZView* view ) {
	yzDebug() << "Session : setCurrentView" << endl;
	changeCurrentView( view );
	
	mCurView = view;
	mCurBuffer = view->myBuffer();
	mCurBuffer->filenameChanged();
}

YZView* YZSession::firstView() {
	return mViewList.front();
}

YZView* YZSession::lastView() {
	return mViewList.back();
}

YZView* YZSession::prevView() {
	if ( currentView() == 0 ) {
		yzDebug() << "WOW, current view is NULL !" << endl;
		return NULL;
	}
	
	YZViewList::Iterator i = mViewList.find( currentView() );
	
	// handle wrap around
	if ( i == mViewList.begin() ) {
		i = mViewList.end();
	}
	
	--i;
	
	return *i;
}

YZView* YZSession::nextView() {
	if ( currentView() == 0 ) {
		yzDebug() << "WOW, current view is NULL !" << endl;
		return NULL;
	}
	
	YZViewList::Iterator i = ++mViewList.find( currentView() );
	
	// check for wrap around
	if ( i == mViewList.end() ) {
		i = mViewList.begin();
	}
	
	return *i;
}

YZBuffer* YZSession::findBuffer( const QString& path ) {
	YZBufferList::Iterator it = mBufferList.begin();
	YZBufferList::Iterator end = mBufferList.end();
	for ( ; it != end; ++it ) {
		YZBuffer *b = ( *it );
		if ( b->fileName() == path ) {
			return b;
		}
	}
	return NULL; //not found
}

bool YZSession::saveAll() {
	YZBufferList::Iterator it = mBufferList.begin();
	YZBufferList::Iterator end = mBufferList.end();
	bool savedAll=true;
	for ( ; it!=end; ++it ) {
		YZBuffer* b = *it;
		if ( !b->fileIsNew() ) {
			if ( b->fileIsModified() && !b->save() ) {
				savedAll=false;
			}
		}
	}
	return savedAll;
}

bool YZSession::isOneBufferModified() {
	YZBufferList::Iterator it = mBufferList.begin();
	YZBufferList::Iterator end = mBufferList.end();
	for ( ; it != end; ++it ) {
		YZBuffer* b = ( *it );
		if ( b->fileIsNew() ) {
			return true;
		}
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
                  mCurBuffer->fileName(),
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
	for ( YZViewList::Iterator i = mViewList.begin(); i != mViewList.end(); ++i ) {
		(*i)->registerModifierKeys( mod );
	}
}

void YZSession::unregisterModifier ( const QString& mod ) {
	for ( YZViewList::Iterator i = mViewList.begin(); i != mViewList.end(); ++i ) {
		(*i)->unregisterModifierKeys( mod );
	}
}

void YZSession::saveJumpPosition() {
	mYzisinfo->updateJumpList( mCurBuffer, currentView()->getCursor()->x(), currentView()->getCursor()->y() );
}

void YZSession::saveJumpPosition( const int x, const int y ) {
	mYzisinfo->updateJumpList( mCurBuffer, x, y );
}

void YZSession::saveJumpPosition( const YZCursor * cursor ) {
	mYzisinfo->updateJumpList( mCurBuffer, cursor->x(), cursor->y() );
}

const YZCursor * YZSession::previousJumpPosition() {

	bool found = false;	
	bool repeating = false;

	while ( true ) {
		if ( mCurrentJumpListItem == 0 ) {
			// Make sure we don't end up in a endless loop
			if ( repeating ) {
				break;
			}
			
			repeating = true;
			mCurrentJumpListItem = mJumpList.count();
		}
		
		--mCurrentJumpListItem;
	
		if ( mJumpList[mCurrentJumpListItem]->filename() == mCurBuffer->fileName() ) {
			found = true;
			break;
		}
	}
	
	if ( found ) {
		return &mJumpList[mCurrentJumpListItem]->position();
	} else {
		return currentView()->getCursor();
	}
}

YZTagStack &YZSession::getTagStack()
{
	return mTagStack;
}

YZView *YZSession::createView( YZBuffer *buffer )
{
	YZView *view = doCreateView( buffer );
	
	addView( view );
	
	return view;
}

void YZSession::deleteView( const YZViewId &id /*=YZViewId::invalid*/ )
{
	YZView *view = currentView();
	
	if ( id != YZViewId::invalid ) {
		view = findView( id );
	}
	
	doDeleteView( view );
	
	removeView( view );
}

void YZSession::addView( YZView *view )
{
	mViewList.push_back( view );
}

void YZSession::removeView( YZView *view )
{
	mViewList.remove( view );
}
		
int YZSession::getIntegerOption( const QString& option ) {
	return YZSession::me->getOptions()->readIntegerOption( option );
}

bool YZSession::getBooleanOption( const QString& option ) {
	return YZSession::me->getOptions()->readBooleanOption( option );
}

QString YZSession::getStringOption( const QString& option ) {
	return YZSession::me->getOptions()->readStringOption( option );
}

QStringList YZSession::getListOption( const QString& option ) {
	return YZSession::me->getOptions()->readListOption( option );
}

void YZSession::eventConnect( const QString& event, const QString& function )
{
	events->connect( event, function );
}

QStringList YZSession::eventCall( const QString& event, YZView *view /*=NULL*/ )
{
	return events->exec( event, view );
}

YZInternalOptionPool *YZSession::getOptions()
{
	return mOptions;
}

void YZSession::setRegister( QChar r, const QStringList& value )
{
	mRegisters->setRegister( r, value );
}

QStringList& YZSession::getRegister ( QChar r )
{
	return mRegisters->getRegister( r );
}

QValueList<QChar> YZSession::getRegisters() 
{ 
	return mRegisters->keys(); 
}

const YZViewList YZSession::getAllViews() const
{
	YZViewList result;
	
	for ( YZBufferList::const_iterator itr = mBufferList.begin(); itr != mBufferList.end(); ++itr ) {
		YZBuffer *buf = *itr;
		const YZViewList views = buf->views();
		
		for ( YZViewList::const_iterator vitr = views.begin(); vitr != views.end(); ++vitr ) {
			result.push_back( *vitr );
		}
	}
	
	return result;
}

