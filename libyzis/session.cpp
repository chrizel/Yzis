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
    dbg().SPrintf("setInstance( %p )", (void *) instance );
    mInstance = instance;
    mInstance->init();
}

YSession::YSession()
{
    dbg() << "YSession()" << endl;
    // do not use debug code here because debug backend is not initialised yet
}

void YSession::init()
{
    dbg() << "init()" << endl;
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
    dbg() << "init() done" << endl;
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
            if (s.startsWith("--level") || s.startsWith("--area-level") || s.startsWith("--debug-output") ) ;

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

                dbg().SPrintf("Init keys = '%s'", qp(mInitkeys) );
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
        first->buffer()->openNewFile();
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
    dbg().SPrintf( "runLuaScript(): Return Value='%s'", qp(retValue) );
    bool ok;
    int retInt = retValue.toInt(&ok, 0);
    if (ok == false) {
        err().SPrintf("runLuaScript(): Could not convert script return value '%s' to int: ", qp(retValue));
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
    dbg() << "~YSession" << endl;
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
    dbg().SPrintf("createBuffer( filename='%s' )", qp(filename) );
    //make sure we don't have a buffer of this path yet
    YBuffer *buffer = findBuffer( filename );
    if (buffer) { //already open !
        return buffer;
    }

    buffer = new YBuffer();
    buffer->setState( YBuffer::BufferActive );

    if ( !filename.isEmpty() ) {
        buffer->load( filename );
    } else {
        buffer->openNewFile();
    }

    mBufferList.push_back( buffer );
    guiCreateBuffer( buffer );

    return buffer;
}

YView *YSession::createBufferAndView( const QString& path )
{
    dbg().SPrintf("createBufferAndView( path='%s' )", qp(path) );
    QString filename = YBuffer::parseFilename(path);
    YView *view;
    YBuffer *buffer = findBuffer( filename );

    if (!buffer) {
        buffer = createBuffer( filename );
        view = createView( buffer );
    } else {
        view = findViewByBuffer(buffer);
    }

    setCurrentView( view );
    buffer->checkRecover();

    view->applyStartPosition( YBuffer::getStartPosition(path) );

    return view;
}

void YSession::removeBuffer( YBuffer *b )
{
    dbg() << "removeBuffer( " << b->toString() << " )" << endl;
    foreach ( YView *v, b->views() ) {
        deleteView( v );
    }
}

void YSession::deleteBuffer( YBuffer *b )
{
    dbg() << "deleteBuffer( " << b->toString() << " )" << endl;
    if ( mBufferList.indexOf( b ) >= 0 ) {
        mBufferList.removeAll( b );
        guiRemoveBuffer( b );
        delete b;
    }

    if ( mBufferList.empty() ) {
        exitRequest();
    }
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
        if (b->fileIsModified() ) return true;
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
    dbg().SPrintf("createView( %s )", qp(buffer->toString()) );
    YView *view = guiCreateView( buffer );
    mViewList.push_back( view );

    /* The view must poll the initial information. The reason is
     * that the buffer and the mode had been initialized before the view.
     */
    view->updateFileName();
    view->updateFileInfo();
    view->updateMode();
    view->updateCursor();

    return view;
}

void YSession::deleteView( YView* view )
{
    dbg().SPrintf("deleteView( %s )", qp(view->toString()) );
    if ( !mViewList.contains(view) ) {
        ftl() << "deleteView(): trying to remove an unknown view " << view->getId() << endl;
        return ;
    }

    // Guardian, if we're deleting the last view, close the app
    if ( mViewList.size() == 1 ) {
        dbg() << "deleteView(): last view being deleted, exiting!" << endl;
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
    if (view == currentView()) {
        dbg() << "setCurrentView(): view already set. Returning. " << endl;
        return;
    }
    guiChangeCurrentView( view );
    view->guiSetFocusMainWindow();

    mCurView = view;
    mCurBuffer = view->buffer();
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
    if ( view->buffer() == buffer )
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
    if (mViewList.isEmpty()) {
        ftl() << "prevView(): WOW, no view in the list!" << endl;
        return NULL;
    }

    if ( currentView() == NULL ) {
        err() << "prevView(): WOW, current view is NULL !" << endl;
        // if that's null, the previous view is the last view
        return mViewList.last();
    }

    int idx = mViewList.indexOf( currentView() );
    if ( idx == -1 ) {
        ftl() << "prevView(): WOW, current view is not in mViewList !" << endl;
        return NULL;
    }

    if ( idx == 0 )
        idx = mViewList.size();
    return mViewList.value( idx - 1 );
}

YView* YSession::nextView()
{
    if (mViewList.isEmpty()) {
        ftl() << "nextView(): WOW, no view in the list!" << endl;
        return NULL;
    }

    if ( currentView() == NULL ) {
        err() << "nextView(): WOW, current view is NULL !" << endl;
        // if that's null, the previous view is the last view
        return mViewList.first();
    }

    int idx = mViewList.indexOf( currentView() );
    if ( idx == -1 ) {
        ftl() << "nextView(): WOW, current view is not in mViewList !" << endl;
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
                     (currentView())->getRowColumnCursor());
                                          
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
    YKeySequence inputs(text);

    sendMultipleKeys( currentView(),  inputs);
    QCoreApplication::instance()->processEvents();
    /* }*/
}

CmdState YSession::sendMultipleKeys( YView * view, YKeySequence & inputs)
{
    CmdState state = CmdOk;
    dbg() << "sendMultipleKeys(" << view << ", keys=" << inputs.toString() << ")" << endl;
    if (view->modePool()->current()->mapMode() & MapCmdline) {
        view->modePool()->change( YMode::ModeCommand );
    }

    for( YKeySequence::const_iterator parsePos = inputs.begin() ; parsePos != inputs.end() && state != CmdStopped && state != CmdError; ++parsePos ) {
        if ( view->modePool()->current()->mapMode() & MapCmdline ) {
            if ( *parsePos == Qt::Key_Escape
                 || *parsePos == Qt::Key_Enter
                 || *parsePos == Qt::Key_Return
                 || *parsePos == Qt::Key_Up
                 || *parsePos == Qt::Key_Down ) {
                state = sendKey( view, *parsePos );
                continue;
            }
            else {
                view->guiSetCommandLineText( view->guiGetCommandLineText() + parsePos->toString() );
                continue;
            }
            
        }
        state = sendKey( view, *parsePos );
    }
    return state;
}

CmdState YSession::sendKey( YView * view, YKey _key)
{
    dbg() << "sendKey( " << view << ", key=" << _key.toString() << ")" << endl;
    CmdState state;

    // Don't respond to pure modifier keys
    if ( _key.key() == Qt::Key_Shift || _key.key() == Qt::Key_Control
         || _key.key() == Qt::Key_Alt )
        return CmdOk;

    QList<QChar> reg = view->registersRecorded();
    if ( reg.count() > 0 ) {
        for ( int ab = 0 ; ab < reg.size(); ++ab ) {
            QString newReg = _key.toString();
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
    if ( _key == a ) _key.setKey( b );        \
    else if ( _key == b ) _key.setKey( a );
    SWITCH_KEY( Qt::Key_Right, Qt::Key_Left );
    SWITCH_KEY( Qt::Key_H, Qt::Key_L );
    }

//    if ( modifiers.contains ("<SHIFT>")) { //useful?
//        key = key.toUpper();
//        modifiers.remove( "<SHIFT>" );
//    }

//    view->appendInputBuffer( modifiers + key );
    view->setPaintAutoCommit( false );
    state = view->modePool()->sendKey( _key );
    view->commitPaintEvent();

    return state;
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
    mYzisinfo->updateJumpList( mCurBuffer, currentView()->getLinePositionCursor());
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
    dbg().SPrintf("YSession::delete( %p )", p );
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
    dbg().SPrintf("Unrecognised option: %s", qp(opt) );
}

