/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
*  Copyright (C) 2005      Scott Newton <scottn@ihug.co.nz>
*  Copyright (C) 2006-2007 Philippe Fremy <phil at freehackers dot org>
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
#include "mode_pool.h"
#include "mode_command.h"
#include "mode_complete.h"
#include "mode_ex.h"
#include "mode_insert.h"
#include "mode_search.h"
#include "mode_visual.h"

using namespace yzis;

#include "tags_stack.h"

#define dbg()    yzDebug("YSession")
#define err()    yzError("YSession")
#define ftl()    yzFatal("YSession")

YSessionIface::~YSessionIface()
{
    // nothing to do here
}

YSession* YSession::mInstance = 0;

void YSession::initDebug( int argc, char ** argv )
{
    // initDebug() must be run early in the creation process
    YDebugBackend::self()->parseRcfile( DEBUGRC_FNAME );
    YDebugBackend::self()->parseArgv( argc, argv );
    dbg() << " ==============[ Yzis started at: " << QDateTime::currentDateTime().toString() << "]====================" << endl;
}

YSession * YSession::self()
{
    if (mInstance == 0L) {
        err() << "YSession::setInstance() has not been called" << endl;
        err() << "There is currently no instance of the session" << endl;
        err() << "Expect SEGFAULT as the next thing to happen!" << endl;
    }
    return mInstance;
}

void YSession::setInstance(YSession* instance)
{
    mInstance = instance;
    mInstance->init();
}

YSession::YSession()
{
    // do not use debug code here because debug backend is not initialised yet
}

void YSession::init()
{
    initLanguage();
    initModes();
    initResource();

    YZIS_SAFE_MODE {
        dbg() << "Yzis SAFE MODE enabled." << endl;
    }
    mSearch = new YSearch();
    mCurView = 0;
    mCurBuffer = 0;
    events = new YEvents();
    mSchemaManager = new YzisSchemaManager();
    mOptions = new YInternalOptionPool();
    mRegisters = new YRegisters();
    mYzisinfo = new YInfo();
    mTagStack = new YTagStack;

    initScript();
    // mYzisinfo->read(this);

    // create HlManager right from the beginning to ensure that this isn't
    // done in YSession::~YSession
    YzisHlManager::self();
}

void YSession::initLanguage()
{
    setlocale( LC_ALL, "");
#ifndef YZIS_WIN32_GCC
    bindtextdomain( "yzis", QString("%1%2").arg( PREFIX ).arg("/share/locale").toUtf8().data() );
    bind_textdomain_codeset( "yzis", "UTF-8" );
    textdomain( "yzis" );
#endif
}

void YSession::initResource()
{
    mResourceMgr = new YResourceMgr();
}

void YSession::initScript()
{
    QString resource;
    resource = resourceMgr()->findResource( ConfigScriptResource, "init.lua" );
    if (! resource.isEmpty()) {
        YLuaEngine::self()->source( resource );
    }
}

void YSession::parseCommandLine( int argc, char * argv[] )
{
    QStringList args;
    YView* first = NULL;
    YView* v;
    QString s;

    for ( int i = 0; i < argc; i++) args << argv[i];

    //quick and very durty way to remove debug args from the args list
    //needed to avoid nyzis to find "unknown options"
    YDebugBackend::self()->parseArgv( args );

    for ( int i = 1; i < args.count(); ++i ) {
        if ( args.at(i)[0] != '-' ) {
            dbg() << "Opening file : " << args.at(i) << endl;
            v = YSession::self()->createBufferAndView( args.at(i) );
            if ( !first) {
                first = v;
            }
        } else {
            dbg() << "Parsing option : " << args.at(i) << endl;
            s = args.at(i);

            // --level and --area-level are parsed in YDebugBackend, Ignore them here
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
                YSession::self()->getOptions()->setGroup("Global");
                YSession::self()->getOptions()->getOption("blocksplash")->setBoolean( false );

                if (s.length() > 2) mInitkeys = args[i].mid(2);
                else if (i < args.count() - 1) mInitkeys = args[++i];

                dbg().sprintf("Init keys = '%s'", qp(mInitkeys) );
            }

            // Load a LUA script from a file
            else if (s == "-s") {
                // no splash screen when executing scripts
                YSession::self()->getOptions()->setGroup("Global");
                YSession::self()->getOptions()->getOption("blocksplash")->setBoolean( false );

                QString luaScript;
                if (s.length() > 2) mLuaScript = args[i].mid(2);
                else if (i < args.count() - 1) mLuaScript = args[++i];

            }

            // Everything else gets an unknown option
            else {
                showCmdLineUnknowOption( args[i] );
                exit( -1);
            }
        }
    }

    if ( !first ) {
        /* no view opened */
        first = YSession::self()->createBufferAndView();
        first->myBuffer()->openNewFile();
        first->displayIntro();
    }

    YSession::self()->setCurrentView( first );
}

void YSession::frontendGuiReady()
{
    dbg() << "frontendGuiReady()" << endl;
    sendInitkeys();
    if (mLuaScript.length()) { 
        runLuaScript();
    }
}

void YSession::runLuaScript()
{
    if (mLuaScript.length() == 0) return;

    dbg() << "runLuaScript(): Running lua script '" << mLuaScript << "'" << endl;

    QString retValue = YLuaEngine::self()->source(mLuaScript);
    dbg().sprintf( "runLuaScript(): Return Value='%s'", qp(retValue) );
    bool ok;
    int retInt = retValue.toInt(&ok, 0);
    if (ok == false) {
        err().sprintf("runLuaScript(): Could not convert script return value '%s' to int: ", qp(retValue));
        exit( -2);
    } else {
        exit(retInt);
    }
}

void YSession::sendInitkeys()
{
    dbg() << HERE() << endl;
    dbg() << toString() << endl;
    dbg() << "Init keys to send: '" << mInitkeys << "'" << endl;
    if (mInitkeys.length()) {
        YSession::self()->scriptSendMultipleKeys( mInitkeys );
    }
}


QString YSession::toString() const
{
    QString s;
    s += "Session Content: \n";
    s += "- Buffer list: \n";
    foreach( YBuffer * b, mBufferList ) {
        s += "  + " + b->toString() + '\n';
    }
    s += "- View list: \n";
    foreach( YView * v, mViewList ) {
        s += "  + " + v->toString() + '\n';
    }
    return s;
}

YSession::~YSession()
{
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
    delete YLuaEngine::self();
    delete mTagStack;
    delete mResourceMgr;
}

void YSession::initModes()
{
    mModes[ YMode::ModeIntro ] = new YModeIntro();
    mModes[ YMode::ModeCommand ] = new YModeCommand();
    mModes[ YMode::ModeEx ] = new YModeEx();
    mModes[ YMode::ModeInsert ] = new YModeInsert();
    mModes[ YMode::ModeReplace ] = new YModeReplace();
    mModes[ YMode::ModeVisual ] = new YModeVisual();
    mModes[ YMode::ModeVisualLine ] = new YModeVisualLine();
    mModes[ YMode::ModeVisualBlock ] = new YModeVisualBlock();
    mModes[ YMode::ModeSearch ] = new YModeSearch();
    mModes[ YMode::ModeSearchBackward ] = new YModeSearchBackward();
    mModes[ YMode::ModeCompletion ] = new YModeCompletion();
    YModeMap::Iterator it;
    // XXX orzel : why isn't that done in YMode ctor or YMode* ctors ?
    for ( it = mModes.begin(); it != mModes.end(); ++it )
        it.value()->init();
}
void YSession::endModes()
{
    YModeMap::Iterator it;
    for ( it = mModes.begin(); it != mModes.end(); ++it )
        delete it.value();
    mModes.clear();
}
YModeMap YSession::getModes() const
{
    return mModes;
}
YModeEx* YSession::getExPool()
{
    return (YModeEx*)mModes[ YMode::ModeEx ];
}
YModeCommand* YSession::getCommandPool()
{
    return (YModeCommand*)mModes[ YMode::ModeCommand ];
}

YInfo * YSession::getYzisinfo()
{
    return mYzisinfo;
}

// ================================================================
//
//                          Buffer stuff
//
// ================================================================

YBuffer *YSession::createBuffer( const QString &filename )
{
    dbg().sprintf("createBuffer( filename='%s' )", qp(filename) );
    //make sure we don't have a buffer of this path yet
    YBuffer *buffer = findBuffer( filename );
    if (buffer) { //already open !
        return buffer;
    }

    buffer = guiCreateBuffer();
    buffer->setState( YBuffer::BufferActive );

    if ( !filename.isEmpty() ) {
        buffer->load( filename );
    } else {
        buffer->openNewFile();
    }

    mBufferList.push_back( buffer );

    return buffer;
}

YView *YSession::createBufferAndView( const QString& path )
{
    dbg().sprintf("createBufferAndView( path='%s' )", qp(path) );
    QString filename = YBuffer::parseFilename(path);
    YBuffer *buffer = findBuffer( filename );
    bool alreadyopen = true;
    if (!buffer) {
        alreadyopen = false;
        buffer = createBuffer( filename );
    }

    YView *view;
    if (!alreadyopen) {
        view = createView( buffer );
    } else {
        view = findViewByBuffer(buffer);
    }
    setCurrentView( view );
	buffer->checkRecover();

    view->applyStartPosition( YBuffer::getStartPosition(path) );

    return view;
}

void YSession::rmBuffer( YBuffer *b )
{
    dbg() << "rmBuffer( " << b->toString() << " )" << endl;
    if ( mBufferList.indexOf( b ) >= 0 ) {
        mBufferList.removeAll( b );
        guiDeleteBuffer( b );
    }
    if ( mBufferList.empty() )
        exitRequest( );
}

YBuffer* YSession::findBuffer( const QString& path )
{
    QFileInfo fi (path);
    foreach( YBuffer *b, mBufferList )
    if ( b->fileName() == fi.absoluteFilePath())
        return b;
    return NULL; //not found
}

bool YSession::isOneBufferModified() const
{
    foreach( YBuffer * b, mBufferList ) {
        if (b->fileIsNew() ) return true;
    }
    return false;
}

// ================================================================
//
//                          View stuff
//
// ================================================================
YView *YSession::createView( YBuffer *buffer )
{
    dbg().sprintf("createView( %s )", qp(buffer->toString()) );
    YView *view = guiCreateView( buffer );
    mViewList.push_back( view );
    return view;
}

void YSession::deleteView( YView* view )
{
    dbg().sprintf("deleteView( %s )", qp(view->toString()) );
    if ( !mViewList.contains(view) ) {
        err() << "trying to remove an unknown view " << view->getId() << endl;
        return ;
    }

    // Guardian, if we're deleting the last view, close the app
    if ( mViewList.size() == 1 ) {
        dbg() << "last view being deleted, exiting!" << endl;
        exitRequest( 0 );
        return ;
    }

    // if we're deleting the current view, then we have to switch views
    if ( view == currentView() ) {
        setCurrentView( prevView() );
    }

    // remove it
    mViewList.removeAll( view );
    guiDeleteView( view );
}

void YSession::setCurrentView( YView* view )
{
    dbg() << "setCurrentView( " << view->toString() << " )" << endl;
    guiChangeCurrentView( view );

    mCurView = view;
    mCurBuffer = view->myBuffer();
    mCurBuffer->filenameChanged();

    guiSetFocusMainWindow();
}

const YViewList YSession::getAllViews() const
{
    YViewList result;

    for ( YBufferList::const_iterator itr = mBufferList.begin(); itr != mBufferList.end(); ++itr ) {
        YBuffer *buf = *itr;
        const YViewList views = buf->views();

        for ( YViewList::const_iterator vitr = views.begin(); vitr != views.end(); ++vitr ) {
            result.push_back( *vitr );
        }
    }

    return result;
}

YView* YSession::findViewByBuffer( const YBuffer *buffer )
{
    if (buffer == NULL) return NULL;
    foreach( YView *view, mViewList )
    if ( view->myBuffer() == buffer )
        return view;
    return NULL;
}

YView* YSession::firstView()
{
    return mViewList.front();
}

YView* YSession::lastView()
{
    return mViewList.back();
}

YView* YSession::prevView()
{
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

YView* YSession::nextView()
{
    if ( currentView() == 0 ) {
        dbg() << "WOW, current view is NULL !" << endl;
        return NULL;
    }

    int idx = mViewList.indexOf( currentView() );
    if ( idx == -1 ) {
        dbg() << "WOW, current view is not in mViewList !" << endl;
        return NULL;
    }
    return mViewList.value( (idx + 1) % mViewList.size() );
}

// ================================================================
//
//                          Application Termination
//
// ================================================================

bool YSession::exitRequest( int errorCode )
{
    dbg() << "exitRequest( " << errorCode << " ) " << endl;
    //prompt unsaved files XXX
    foreach( YBuffer *b, mBufferList )
    b->saveYzisInfo( b->firstView() );
    /*
    mBuffers.clear();

    getYzisinfo()->updateStartPosition( 
                     mCurBuffer->fileName(),
                     (currentView())->getCursor());
                                          
    getYzisinfo()->writeYzisinfo();*/

    return guiQuit( errorCode );
}


void YSession::saveBufferExit()
{
    dbg() << HERE() << endl;
    if ( saveAll() )
        guiQuit();
}

bool YSession::saveAll()
{
    dbg() << HERE() << endl;
    bool savedAll = true;
    foreach( YBuffer *b, mBufferList )
    if ( !b->fileIsNew() )
        if ( b->fileIsModified() && !b->save() )
            savedAll = false;
    return savedAll;
}

void YSession::scriptSendMultipleKeys ( const QString& text)
{
    dbg() << "scriptSendMultipleKeys(" << text << ")" << endl;
    // QStringList list = QStringList::split("<ENTER>", text);
    /* QStringList::Iterator it = list.begin(), end = list.end();
     for (; it != end; ++it) {*/
    sendMultipleKeys( currentView(),  /* *it + "<ENTER>" */ text);
    QCoreApplication::instance()->processEvents();
    /* }*/
}

bool YSession::sendMultipleKeys( YView * view, const QString& _keys)
{
    bool stopped = false;
    dbg() << "sendMultipleKeys(" << view << ", keys=" << _keys << ")" << endl;
    if (view->modePool()->current()->mapMode() & MapCmdline) {
        view->modePool()->change( YMode::ModeCommand );
    }
    QString keys = _keys;
    for ( int i = 0 ; i < keys.length(); ) {
        QString key = keys.mid( i );
        dbg() << "Handling key: " << key << endl;
        //exception : in SEARCH, SEARCH_BACKWARD and EX mode we don't send keys immediately
        if (view->modePool()->current()->mapMode() & MapCmdline) {
            if ( key.startsWith( "<ESC>" ) ) {
                stopped = sendKey( view, "<ESC>" );
                continue;
            } else if ( key.startsWith( "<ENTER>" ) ) {
                stopped = sendKey( view, "<ENTER>" );
                i += 7;
                continue;
            } else if ( key.startsWith( "<UP>" ) ) {
                stopped = sendKey( view, "<UP>" );
                i += 4;
                continue;
            } else if ( key.startsWith( "<DOWN>" ) ) {
                stopped = sendKey( view, "<DOWN>" );
                i += 6;
                continue;
            } else {
                view->guiSetCommandLineText( view->guiGetCommandLineText() + key.mid(0, 1) );
                i++;
                continue;
            }
        }
        if ( key.startsWith( "<CTRL>" ) ) {
            dbg() << "Sending " << key.mid(6, 1) << endl;
            stopped = sendKey( view, key.mid( 6, 1 ), "<CTRL>" );
            i += 7;
            continue;
        } else if ( key.startsWith( "<ALT>" ) ) {
            stopped = sendKey( view, key.mid( 5, 1 ), "<ALT>" );
            i += 6;
            continue;
        } else if ( key.startsWith( "<SHIFT>" ) ) {
            stopped =sendKey( view, key.mid( 7, 1 ), "<SHIFT>" );
            i += 8;
            continue;
        } else if ( key.startsWith( "<ESC>" ) ) {
            stopped = sendKey( view, "<ESC>" );
            i += 5;
            continue;
        } else if ( key.startsWith( "<ENTER>" ) ) {
            stopped =sendKey( view, "<ENTER>" );
            i += 7;
            continue;
        } else if ( key.startsWith( "<TAB>" ) ) {
            stopped = sendKey( view, "<TAB>" );
            i += 5;
            continue;
        } else if ( key.startsWith( "<UP>" ) ) {
            stopped = sendKey( view, "<UP>" );
            i += 4;
            continue;
        } else if ( key.startsWith( "<DOWN>" ) ) {
            stopped = sendKey( view, "<DOWN>" );
            i += 6;
            continue;
        } else if ( key.startsWith( "<RIGHT>" ) ) {
            stopped = sendKey( view, "<RIGHT>" );
            i += 7;
            continue;
        } else if ( key.startsWith( "<LEFT>" ) ) {
            stopped = sendKey( view, "<LEFT>" );
            i += 6;
            continue;
        } else if ( key.startsWith( "<DEL>" ) ) {
            stopped = sendKey( view, "<DEL>" );
            i += 5;
            continue;
        } else if ( key.startsWith( "<BS>" ) ) {
            stopped = sendKey( view, "<BS>" );
            i += 4;
            continue;
        } else {
            stopped = sendKey( view, key.mid( 0, 1 ) );
            i++;
        }
	if ( stopped )
	    break;
    }

    return stopped;
}

bool YSession::sendKey( YView * view, const QString& _key, const QString& _modifiers)
{
    dbg() << "sendKey( " << view << ", key=" << _key << " mod=" << _modifiers << ")" << endl;

    QString key = _key;
    QString modifiers = _modifiers;
    bool stopped;

    if ( _key == "<SHIFT>" || _key == "<CTRL>" || _key == "<ALT>" ) return false; //we are not supposed to received modifiers in key

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
    if ( rightleft && ( view->modePool()->current()->mapMode() & (MapVisual | MapNormal) ) ) {
#define SWITCH_KEY( a, b ) \
    if ( key == a ) key = b; \
    else if ( key == b ) key = a
        SWITCH_KEY( "<RIGHT>", "<LEFT>" );
        SWITCH_KEY( "h", "l" );
    }

//    if ( modifiers.contains ("<SHIFT>")) { //useful?
//        key = key.toUpper();
//        modifiers.remove( "<SHIFT>" );
//    }

//    view->appendInputBuffer( modifiers + key );
    view->setPaintAutoCommit( false );
    stopped = view->modePool()->sendKey( key, modifiers );
    view->commitPaintEvent();

    return stopped;
}

void YSession::registerModifier ( const QString& mod )
{
    foreach( YView *view, mViewList )
    view->registerModifierKeys( mod );
}

void YSession::unregisterModifier ( const QString& mod )
{
    foreach( YView *view, mViewList )
    view->unregisterModifierKeys( mod );
}

void YSession::saveJumpPosition()
{
    mYzisinfo->updateJumpList( mCurBuffer, currentView()->getCursor());
}

void YSession::saveJumpPosition( const QPoint cursor )
{
    mYzisinfo->updateJumpList( mCurBuffer, cursor );
}

const YCursor YSession::previousJumpPosition()
{
    return mYzisinfo->previousJumpPosition();
}

YTagStack &YSession::getTagStack()
{
    return *mTagStack;
}

int YSession::getIntegerOption( const QString& option )
{
    return YSession::self()->getOptions()->readIntegerOption( option );
}

bool YSession::getBooleanOption( const QString& option )
{
    return YSession::self()->getOptions()->readBooleanOption( option );
}

QString YSession::getStringOption( const QString& option )
{
    return YSession::self()->getOptions()->readStringOption( option );
}

QStringList YSession::getListOption( const QString& option )
{
    return YSession::self()->getOptions()->readListOption( option );
}

void YSession::eventConnect( const QString& event, const QString& function )
{
    events->connect( event, function );
}

QStringList YSession::eventCall( const QString& event, YView *view /*=NULL*/ )
{
    return events->exec( event, view );
}

YInternalOptionPool *YSession::getOptions()
{
    return mOptions;
}

void YSession::setRegister( QChar r, const QStringList& value )
{
    mRegisters->setRegister( r, value );
}

QStringList& YSession::getRegister ( QChar r )
{
    return mRegisters->getRegister( r );
}

QList<QChar> YSession::getRegisters() const
{
    return mRegisters->keys();
}

void * YSession::operator new( size_t tSize )
{
    dbg() << "YSession::new()" << tSize << endl;
    return yzmalloc( tSize );
}

void YSession::operator delete( void *p )
{
    dbg().sprintf("YSession::delete( %p )", p );
    yzfree(p);
}

// ================================================================
//
//                          Command line stuff
//
// ================================================================

/** Show help text for -h and --help option */
void YSession::showCmdLineHelp( const QString & progName )
{
    QString usage = QString(
                        "%1 [options] [file1 [file2] ... ]\n"
                        "-h | --help : show this message\n"
                        "-v | --version: version information\n"
                        "-c <some key presses> : execute immediately the key presses when yzis starts, asif they were typed by the user.\n"
                    ).arg(progName);
    fputs(qp(usage), stderr);
}

/** Show version text for -v and --version option */
void YSession::showCmdLineVersion()
{
    QString versionText = version();
    fputs(qp(versionText), stderr);
}

QString YSession::version()
{
    return QString( "Yzis - http://www.yzis.org\n" VERSION_CHAR_LONG " " VERSION_CHAR_DATE );
}

/** Show error message for unknown option */
void YSession::showCmdLineUnknowOption( const QString & opt )
{
    fprintf(stderr, "Unrecognised option: %s", qp(opt) );
    dbg().sprintf("Unrecognised option: %s", qp(opt) );
}

