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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

// Project
#include "session.h"

#include "yzis.h"
#include "debug.h"
#include "kate/schema.h"
#include "buffer.h"
#include "registers.h"
#include "mapping.h"
#include "search.h"
#include "events.h"
#include "internal_options.h"
#include "view.h"
#include "yzisinfo.h"
#include "resourcemgr.h"
#include "luaengine.h"

// system
#include <stdlib.h>

// Qt
#include <QCoreApplication>
#include <QDir>
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QStringList>
#include <QBuffer>

// project
#include "mode_command.h"
#include "mode_complete.h"
#include "mode_ex.h"
#include "mode_insert.h"
#include "mode_search.h"
#include "mode_visual.h"

using namespace yzis;

#include "tags_stack.h"

#define dbg()    yzDebug("YZSession")
#define err()    yzError("YZSession")
#define ftl()    yzFatal("YZSession")

YZSessionIface::~YZSessionIface()
{
    // nothing to do here
}

YZSession* YZSession::mInstance = 0;

void YZSession::initDebug( int argc, char ** argv )
{
    // initDebug() must be run early in the creation process
    YZDebugBackend::self()->parseRcfile( DEBUGRC_FNAME );
    YZDebugBackend::self()->parseArgv( argc, argv );
    dbg() << " ==============[ Yzis started at: " << QDateTime::currentDateTime().toString() << "]====================" << endl;
}

YZSession * YZSession::self()
{
    if (mInstance == 0L) {
        err() << "YZSession::setInstance() has not been called" << endl;
        err() << "There is currently no instance of the session" << endl;
        err() << "Expect SEGFAULT as the next thing to happen!" << endl;
    }
    return mInstance;
}

void YZSession::setInstance(YZSession* instance) 
{ 
    mInstance = instance;
    mInstance->init();
}

YZSession::YZSession() {
    // do not use debug code here because debug backend is not initialised yet
}

void YZSession::init()
{
	initLanguage();
	initModes();
    initResource();

	YZIS_SAFE_MODE {
		dbg() << "Yzis SAFE MODE enabled." << endl;
	}
	mSearch = new YZSearch();
	mCurView = 0;
	mCurBuffer = 0;
	events = new YZEvents();
	mSchemaManager = new YzisSchemaManager();
	mOptions = new YZInternalOptionPool();
	mRegisters = new YZRegisters();
	mYzisinfo= new YZYzisinfo();
	mTagStack = new YZTagStack;

	initScript();
//	mYzisinfo->read(this);

	// create HlManager right from the beginning to ensure that this isn't
	// done in YZSession::~YZSession
	YzisHlManager::self();
}

void YZSession::initLanguage()
{
	setlocale( LC_ALL, "");
#ifndef YZIS_WIN32_GCC
	bindtextdomain( "yzis", QString("%1%2").arg( PREFIX ).arg("/share/locale").toUtf8().data() );
	bind_textdomain_codeset( "yzis", "UTF-8" );
	textdomain( "yzis" );
#endif
}

void YZSession::initResource()
{
    mResourceMgr = new YZResourceMgr();
}

void YZSession::initScript()
{
    QString resource;
    resource = resourceMgr()->findResource( ConfigScriptResource, "init.lua" );
    if (! resource.isEmpty()) {
		YZLuaEngine::self()->source( resource );
    }
}

void YZSession::parseCommandLine( int argc, char * argv[] )
{
    QStringList args;
	YZView* first = NULL;
	YZView* v;
    QString s;

    for( int i=0; i<argc; i++) args << argv[i];

    //quick and very durty way to remove debug args from the args list
    //needed to avoid nyzis to find "unknown options"
    YZDebugBackend::self()->parseArgv( args );

	for ( int i = 1; i < args.count(); ++i ) {
		if ( args.at(i)[0] != '-' ) {
            dbg() << "Opening file : " << args.at(i) << endl;
			v = YZSession::self()->createBufferAndView( args.at(i) );
			if ( !first) {
				first = v;
            }
	    } else {
            dbg() << "Parsing option : " << args.at(i) << endl;
            s = args.at(i);

            // --level and --area-level are parsed in YZDebugBackend, Ignore them here
            if (s.startsWith("--level") || s.startsWith("--area-level") ) ;

            // Asking for Help
            else if (s == "-h" || s == "--help") {
                showCmdLineHelp( args[0] );
                exit(0);
            }

            // Asking for Version Information
            else if (s == "-v" || s == "--version") {
                showCmdLineVersion();
                exit(0);
            }

            // Passing Keys after the event loop started
            else if (s == "-c") {
                // no splash screen when executing scripts
                YZSession::self()->getOptions()->setGroup("Global");
                YZSession::self()->getOptions()->getOption("blocksplash")->setBoolean( false );

                if (s.length() > 2) mInitkeys = args[i].mid(2);
                else if (i < args.count()-1) mInitkeys = args[++i];

                dbg().sprintf("Init keys = '%s'", qp(mInitkeys) );
            }

            // Load a LUA script from a file
            else if (s == "-s") {
                // no splash screen when executing scripts
                YZSession::self()->getOptions()->setGroup("Global");
                YZSession::self()->getOptions()->getOption("blocksplash")->setBoolean( false );

                QString luaScript;
                if (s.length() > 2) mLuaScript = args[i].mid(2);
                else if (i < args.count()-1) mLuaScript = args[++i];

            }

            // Everything else gets an unknown option
            else {
                showCmdLineUnknowOption( args[i] );
                exit(-1);
            }
	    }
    }

	if ( !first ) {
		/* no view opened */
		first = YZSession::self()->createBufferAndView();
		first->myBuffer()->openNewFile();
		first->displayIntro();
	}

	YZSession::self()->setCurrentView( first );
}

void YZSession::frontendGuiReady()
{
    dbg() << "frontendGuiReady()" << endl;
    sendInitkeys();
    runLuaScript();
}

void YZSession::runLuaScript()
{
    if (mLuaScript.length()) {
        QString retValue = YZLuaEngine::self()->source(mLuaScript);
        dbg() << "Return Value:" << retValue << endl;
        bool ok;
        int retInt = retValue.toInt(&ok, 0);
        if(ok == true){
            exit(retInt);
        }
        exit(1);
    }
}

void YZSession::sendInitkeys()
{
    dbg() << HERE() << endl;
    dbg() << toString() << endl;
    dbg() << "Init keys to send: '" << mInitkeys << "'" << endl;
    if (mInitkeys.length()) {
        YZSession::self()->scriptSendMultipleKeys( mInitkeys );
    }
}


QString YZSession::toString() const
{
    QString s;
    s += "Session Content: \n";
    s += "- Buffer list: \n";
    foreach( YZBuffer * b, mBufferList ) {
        s += "  + " + b->toString() + '\n';
    }
    s += "- View list: \n";
    foreach( YZView * v, mViewList ) {
        s += "  + " + v->toString() + '\n';
    }
    return s;
}

YZSession::~YZSession() {
	mYzisinfo->write(); // save yzisinfo
	endModes();
	delete YzisHlManager::self();
	delete mSchemaManager;
	delete mSearch;
	delete events;
	delete mRegisters;
	delete mOptions;
	delete mYzisinfo;
	delete YZMapping::self();
	delete YZLuaEngine::self();
	delete mTagStack;
    delete mResourceMgr;
}

void YZSession::initModes() {
	mModes[ YZMode::ModeIntro ] = new YZModeIntro();
	mModes[ YZMode::ModeCommand ] = new YZModeCommand();
	mModes[ YZMode::ModeEx ] = new YZModeEx();
	mModes[ YZMode::ModeInsert ] = new YZModeInsert();
	mModes[ YZMode::ModeReplace ] = new YZModeReplace();
	mModes[ YZMode::ModeVisual ] = new YZModeVisual();
	mModes[ YZMode::ModeVisualLine ] = new YZModeVisualLine();
	mModes[ YZMode::ModeVisualBlock ] = new YZModeVisualBlock();
	mModes[ YZMode::ModeSearch ] = new YZModeSearch();
	mModes[ YZMode::ModeSearchBackward ] = new YZModeSearchBackward();
	mModes[ YZMode::ModeCompletion ] = new YZModeCompletion();
	YZModeMap::Iterator it;
	// XXX orzel : why isn't that done in YZMode ctor or YZMode* ctors ?
	for( it = mModes.begin(); it != mModes.end(); ++it )
		it.value()->init();
}
void YZSession::endModes() {
	YZModeMap::Iterator it;
	for( it = mModes.begin(); it != mModes.end(); ++it )
		delete it.value();
	mModes.clear();
}
YZModeMap YZSession::getModes() const {
	return mModes;
}
YZModeEx* YZSession::getExPool() {
	return (YZModeEx*)mModes[ YZMode::ModeEx ];
}
YZModeCommand* YZSession::getCommandPool() {
	return (YZModeCommand*)mModes[ YZMode::ModeCommand ];
}

YZYzisinfo * YZSession::getYzisinfo() {
	return mYzisinfo;
}

// ================================================================
//
//                          Buffer stuff
//
// ================================================================

YZBuffer *YZSession::createBuffer( const QString &filename ) {
    dbg().sprintf("createBuffer( filename='%s' )", qp(filename) );
	//make sure we don't have a buffer of this path yet
	YZBuffer *buffer = findBuffer( filename );
	if (buffer) { //already open !
		return buffer;
	}

	buffer = guiCreateBuffer();
	buffer->setState( YZBuffer::BufferActive );
	
	if ( !filename.isEmpty() ) {
		buffer->load( filename );
	} else {
		buffer->openNewFile();
	}
	
	mBufferList.push_back( buffer );
	
	return buffer;
}

YZView *YZSession::createBufferAndView( const QString& path ) {
    dbg().sprintf("createBufferAndView( path='%s' )", qp(path) );
	QString filename = YZBuffer::parseFilename(path);
	YZBuffer *buffer = findBuffer( filename );
	bool alreadyopen = true;
	if (!buffer) {
		alreadyopen=false;
		buffer = createBuffer( filename );
	}

	YZView *view;
	if (!alreadyopen) {
		view = createView( buffer );
	} else {
		view = findViewByBuffer(buffer);
	}
	setCurrentView( view );

	view->applyStartPosition( YZBuffer::getStartPosition(path) );

	return view;
}

void YZSession::rmBuffer( YZBuffer *b ) {
	dbg() << "rmBuffer( " << b->toString() << " )" << endl;
	if ( mBufferList.indexOf( b ) >= 0 ) {
			mBufferList.removeAll( b );
			guiDeleteBuffer( b );
	}
	if ( mBufferList.empty() )
		exitRequest( );
}

YZBuffer* YZSession::findBuffer( const QString& path ) {
	QFileInfo fi (path);
	foreach( YZBuffer *b, mBufferList )
		if ( b->fileName() == fi.absoluteFilePath())
			return b;
	return NULL; //not found
}

bool YZSession::isOneBufferModified() const {
    foreach( YZBuffer * b, mBufferList ) {
        if (b->fileIsNew() ) return true;
    }
    return false;
}

// ================================================================
//
//                          View stuff
//
// ================================================================
YZView *YZSession::createView( YZBuffer *buffer ) {
    dbg().sprintf("createView( %s )", qp(buffer->toString()) );
	YZView *view = guiCreateView( buffer );
	mViewList.push_back( view );
	return view;
}

void YZSession::deleteView( YZView* view ) {
    dbg().sprintf("deleteView( %s )", qp(view->toString()) );
	if ( !mViewList.contains(view) ) {
		err() << "trying to remove an unknown view " << view->getId() << endl;
		return;
	}

	// Guardian, if we're deleting the last view, close the app
	if ( mViewList.size() == 1 ) {
        dbg() << "last view being deleted, exiting!" << endl;
		exitRequest( 0 );
		return;
	}

	// if we're deleting the current view, then we have to switch views
	if ( view == currentView() ) {
		setCurrentView( prevView() );
	}
	
	// remove it
	mViewList.removeAll( view );
	guiDeleteView( view );
}

void YZSession::setCurrentView( YZView* view ) {
	dbg() << "setCurrentView( " << view->toString() <<  " )" << endl;
	guiChangeCurrentView( view );
	
	mCurView = view;
	mCurBuffer = view->myBuffer();
	mCurBuffer->filenameChanged();
	
	guiSetFocusMainWindow();
}

const YZViewList YZSession::getAllViews() const {
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

YZView* YZSession::findViewByBuffer( const YZBuffer *buffer ) {
    if (buffer == NULL) return NULL;
	foreach(  YZView *view, mViewList )
		if ( view->myBuffer() == buffer )
			return view;
	return NULL;
}

YZView* YZSession::firstView() {
	return mViewList.front();
}

YZView* YZSession::lastView() {
	return mViewList.back();
}

YZView* YZSession::prevView() {
	if ( currentView() == 0 ) {
		dbg() << "WOW, current view is NULL !" << endl;
		return NULL;
	}
	int idx = mViewList.indexOf( currentView() );
	if ( idx == -1 ) {
		dbg() << "WOW, current view is not in mViewList !" << endl;
		return NULL;
	}

	if ( idx == 0 )
		idx = mViewList.size();
	return mViewList.value( idx - 1 );
}

YZView* YZSession::nextView() {
	if ( currentView() == 0 ) {
		dbg() << "WOW, current view is NULL !" << endl;
		return NULL;
	}
	
	int idx = mViewList.indexOf( currentView() );
	if ( idx == -1 ) {
		dbg() << "WOW, current view is not in mViewList !" << endl;
		return NULL;
	}
	return mViewList.value( (idx+1)%mViewList.size() );
}

// ================================================================
//
//                          Application Termination
//
// ================================================================

bool YZSession::exitRequest( int errorCode ) {
	dbg() << "exitRequest( " << errorCode << " ) " << endl;
	//prompt unsaved files XXX
	foreach( YZBuffer *b, mBufferList )
		b->saveYzisInfo( b->firstView() );
	/*
	mBuffers.clear();
	
	getYzisinfo()->updateStartPosition( 
                  mCurBuffer->fileName(),
                  (currentView())->getCursor());
                                       
	getYzisinfo()->writeYzisinfo();*/
                                          
	return guiQuit( errorCode );
}


void YZSession::saveBufferExit() {
    dbg() << HERE() << endl;
	if ( saveAll() )
		guiQuit();
}

bool YZSession::saveAll() {
    dbg() << HERE() << endl;
	bool savedAll=true;
	foreach( YZBuffer *b, mBufferList )
		if ( !b->fileIsNew() )
			if ( b->fileIsModified() && !b->save() )
				savedAll=false;
	return savedAll;
}

void YZSession::scriptSendMultipleKeys ( const QString& text) {
    dbg() << "scriptSendMultipleKeys(" << text << ")" << endl;
//	QStringList list = QStringList::split("<ENTER>", text);
/*	QStringList::Iterator it = list.begin(), end = list.end();
	for (; it != end; ++it) {*/
		sendMultipleKeys( currentView(), /* *it + "<ENTER>" */ text);
		QCoreApplication::instance()->processEvents();
/*	}*/
}

void YZSession::sendMultipleKeys( YZView * view, const QString& _keys) {
	dbg() << "sendMultipleKeys(" << view << ", keys=" << _keys << ")" << endl;
	if (view->modePool()->current()->mapMode() & MapCmdline) {
		view->modePool()->change( YZMode::ModeCommand );
	}
	QString keys = _keys;
	for ( int i = 0 ; i < keys.length(); ) {
		QString key = keys.mid( i );
		dbg() << "Handling key: " << key << endl;
		//exception : in SEARCH, SEARCH_BACKWARD and EX mode we don't send keys immediately
		if (view->modePool()->current()->mapMode() & MapCmdline) {
			if ( key.startsWith( "<ESC>" ) ) {
				sendKey( view, "<ESC>" );
				continue;
			} else if ( key.startsWith( "<ENTER>" ) ) {
				sendKey( view, "<ENTER>" );
				i+=7;
				continue;
			} else if ( key.startsWith( "<UP>" ) ) {
				sendKey( view, "<UP>" );
				i+=4;
				continue;
			} else if ( key.startsWith( "<DOWN>" ) ) {
				sendKey( view, "<DOWN>" );
				i+=6;
				continue;
			} else {
				view->guiSetCommandLineText( view->guiGetCommandLineText() + key.mid(0,1) );
				i++;
				continue;
			}
		}
		if ( key.startsWith( "<CTRL>" ) ) {
			dbg() << "Sending " << key.mid(6,1) << endl;
			sendKey( view, key.mid( 6,1 ), "<CTRL>" );
			i+=7;
			continue;
		} else if ( key.startsWith( "<ALT>" ) ) {
			sendKey( view, key.mid( 5,1 ), "<ALT>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<SHIFT>" ) ) {
			sendKey( view, key.mid( 7,1 ), "<SHIFT>" );
			i+=8;
			continue;
		} else if ( key.startsWith( "<ESC>" ) ) {
			sendKey( view, "<ESC>" );
			i+=5;
			continue;
		} else if ( key.startsWith( "<ENTER>" ) ) {
			sendKey( view, "<ENTER>" );
			i+=7;
			continue;
		} else if ( key.startsWith( "<TAB>" ) ) {
			sendKey( view, "<TAB>" );
			i+=5;
			continue;
		} else if ( key.startsWith( "<UP>" ) ) {
			sendKey( view, "<UP>" );
			i+=4;
			continue;
		} else if ( key.startsWith( "<DOWN>" ) ) {
			sendKey( view, "<DOWN>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<RIGHT>" ) ) {
			sendKey( view, "<RIGHT>" );
			i+=7;
			continue;
		} else if ( key.startsWith( "<LEFT>" ) ) {
			sendKey( view, "<LEFT>" );
			i+=6;
			continue;
		} else if ( key.startsWith( "<DEL>" ) ) {
			sendKey( view, "<DEL>" );
			i+=5;
			continue;
		} else if ( key.startsWith( "<BS>" ) ) {
			sendKey( view, "<BS>" );
			i+=4;
			continue;
		} else {
			sendKey( view, key.mid( 0,1 ) );
			i++;
		}
	}
}

void YZSession::sendKey( YZView * view, const QString& _key, const QString& _modifiers) {
	dbg() << "sendKey( " << view << ", key=" << _key << " mod=" << _modifiers << ")" << endl;

	QString key=_key;
	QString modifiers=_modifiers;
	if ( _key == "<SHIFT>" || _key == "<CTRL>" || _key == "<ALT>" ) return; //we are not supposed to received modifiers in key

    QList<QChar> reg = view->registersRecorded();
	if ( reg.count() > 0 ) {
		for ( int ab = 0 ; ab < reg.size(); ++ab ) {
			QString newReg = modifiers + _key;
			QStringList curReg = getRegister( reg.at(ab) );
			if ( curReg.size() > 0 )
				newReg.prepend( curReg[ 0 ] );
			setRegister( reg.at(ab), QStringList( newReg ) );
		}
	}

	/** rightleft mapping **/
	bool rightleft = view->getLocalBooleanOption( "rightleft" );
	if ( rightleft && ( view->modePool()->current()->mapMode() & (MapVisual|MapNormal) ) ) {
#define SWITCH_KEY( a, b ) \
	if ( key == a ) key = b; \
	else if ( key == b ) key = a
		SWITCH_KEY( "<RIGHT>", "<LEFT>" );
		SWITCH_KEY( "h", "l" );
	}

	if ( modifiers.contains ("<SHIFT>")) {//useful?
		key = key.toUpper();
		modifiers.remove( "<SHIFT>" );
	}

	view->appendInputBuffer( modifiers + key );
	view->setPaintAutoCommit( false );
	view->modePool()->sendKey( key, modifiers );
	view->commitPaintEvent();
}

void YZSession::registerModifier ( const QString& mod ) {
	foreach(  YZView *view, mViewList )
		view->registerModifierKeys( mod );
}

void YZSession::unregisterModifier ( const QString& mod ) {
	foreach(  YZView *view, mViewList )
		view->unregisterModifierKeys( mod );
}

void YZSession::saveJumpPosition() {
	mYzisinfo->updateJumpList( mCurBuffer, currentView()->getCursor());
}

void YZSession::saveJumpPosition( const QPoint cursor ) {
	mYzisinfo->updateJumpList( mCurBuffer, cursor );
}

const YZCursor YZSession::previousJumpPosition() {
	return mYzisinfo->previousJumpPosition();
}

YZTagStack &YZSession::getTagStack() {
	return *mTagStack;
}

int YZSession::getIntegerOption( const QString& option ) {
	return YZSession::self()->getOptions()->readIntegerOption( option );
}

bool YZSession::getBooleanOption( const QString& option ) {
	return YZSession::self()->getOptions()->readBooleanOption( option );
}

QString YZSession::getStringOption( const QString& option ) {
	return YZSession::self()->getOptions()->readStringOption( option );
}

QStringList YZSession::getListOption( const QString& option ) {
	return YZSession::self()->getOptions()->readListOption( option );
}

void YZSession::eventConnect( const QString& event, const QString& function ) {
	events->connect( event, function );
}

QStringList YZSession::eventCall( const QString& event, YZView *view /*=NULL*/ ) {
	return events->exec( event, view );
}

YZInternalOptionPool *YZSession::getOptions() {
	return mOptions;
}

void YZSession::setRegister( QChar r, const QStringList& value ) {
	mRegisters->setRegister( r, value );
}

QStringList& YZSession::getRegister ( QChar r ) {
	return mRegisters->getRegister( r );
}

QList<QChar> YZSession::getRegisters() const { 
	return mRegisters->keys(); 
}

void * YZSession::operator new( size_t tSize )
{
	dbg() << "YZSession::new()" << tSize << endl;
	return yzmalloc( tSize );
}

void YZSession::operator delete( void *p )
{
    dbg().sprintf("YZSession::delete( %p )", p );
	yzfree(p);
}

// ================================================================
//
//                          Command line stuff
//
// ================================================================

/** Show help text for -h and --help option */
void YZSession::showCmdLineHelp( const QString & progName )
{
    QString usage = QString(
"%1 [options] [file1 [file2] ... ]\n"
"-h | --help : show this message\n"
"-v | --version: version information\n"
"-c <some key presses> : execute immediately the key presses when yzis starts, asif they were typed by the user.\n"
    ).arg(progName);
    fputs(qp(usage),stderr);
}

/** Show version text for -v and --version option */
void YZSession::showCmdLineVersion()
{
    QString versionText	= version(); 
    fputs(qp(versionText),stderr);
}

QString YZSession::version()
{
    return QString( "Yzis - http://www.yzis.org\n" VERSION_CHAR_LONG " " VERSION_CHAR_DATE );
}

/** Show error message for unknown option */
void YZSession::showCmdLineUnknowOption( const QString & opt )
{
    fprintf(stderr, "Unrecognised option: %s", qp(opt) );
    dbg().sprintf("Unrecognised option: %s", qp(opt) );
}

