 /* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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

#include <string>
#include "philcppunit/PhilAsserts.h"
using namespace CppUnit;
using namespace std;

#include "libyzis/undo.h"
#include "libyzis/debug.h"

#include "testUndo.h"
#include "TSession.h"
#include "TBuffer.h"
#include "TView.h"


// register the suite so that it is run in the runner
CPPUNIT_TEST_SUITE_REGISTRATION( TestUndo );

/* ========================================================================= */

void TestUndo::setUp()
{
    mSession = new TYZSession();
    mBuf = new TYZBuffer( mSession );
    mBuf->clearIntro();
    mView = new TYZView( mBuf, mSession, 5 );
}

void TestUndo::tearDown()
{
    delete mBuf;
    delete mSession;
}

void TestUndo::testUndoBufferCreation()
{
    YZUndoBuffer * ub = mBuf->undoBuffer();
    phCheckEquals( ub->mayRedo(), false );
    phCheckEquals( ub->mayUndo(), false );
    ub->undo(mView);
    phCheckEquals( ub->mayRedo(), false );
    phCheckEquals( ub->mayUndo(), false );
    ub->redo(mView);
    phCheckEquals( ub->mayRedo(), false );
    phCheckEquals( ub->mayUndo(), false );
}

void TestUndo::performUndoRedo( QStringList & textHistory, bool commandUndo )
{
    YZUndoBuffer * ub = mBuf->undoBuffer();
    uint i=textHistory.count()-2;

    if (commandUndo) {
        mView->sendMultipleKey("<ESC>");
    }

    yzDebug( "TestUndo" ) << "========== Starting Undo ===========" << endl;
    while( i > 0 ) {
        if (commandUndo) {
            mView->sendMultipleKey("u");
        } else {
            ub->undo(mView);
        }
        yzDebug( "TestUndo" ) << "buffer " << i << ": '" << mBuf->getWholeText() << "'" << endl;
        phCheckEquals( mBuf->getWholeText(), textHistory[i] );
        phCheckEquals( ub->mayUndo(), true );
        phCheckEquals( ub->mayRedo(), true );
        i--;
    }

    if (commandUndo) {
        mView->sendMultipleKey("u");
    } else {
        ub->undo( mView );
    }
    yzDebug( "TestUndo" ) << ub->toString() << endl;
    yzDebug( "TestUndo" ) << "buffer " << i << ": '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( mBuf->getWholeText(), textHistory[i] );
    phCheckEquals( ub->mayUndo(), false );
    phCheckEquals( ub->mayRedo(), true );

    yzDebug( "TestUndo" ) << "========== Starting Redo ===========" << endl;

    while( ++i < textHistory.count()-1 ) {
        if (commandUndo) {
            mView->sendMultipleKey("<CTRL>R");
        } else {
            ub->redo(mView);
        }
        yzDebug( "TestUndo" ) << "buffer " << i << ": '" << mBuf->getWholeText() << "'" << endl;
        phCheckEquals( mBuf->getWholeText(), textHistory[i] );
        phCheckEquals( ub->mayUndo(), true );
        phCheckEquals( ub->mayRedo(), true );
    }

    if (commandUndo) {
        mView->sendMultipleKey("<CTRL>R");
    } else {
        ub->redo(mView);
    }
    yzDebug( "TestUndo" ) << "buffer " << i << ": '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( mBuf->getWholeText(), textHistory[i] );
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );
}

void TestUndo::testUndoCharOperation()
{
    YZUndoBuffer * ub = mBuf->undoBuffer();
    QStringList textHistory;
    textHistory.append( mBuf->getWholeText() );

    mBuf->appendLine( "11111111" );
    mBuf->appendLine( "22222222" );
    mBuf->appendLine( "33333333" );
    mBuf->insertChar( 0, 0, "A" );
    mBuf->insertChar( 8, 3, "A" );
    mBuf->insertChar( 8, 3, "A" );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    mBuf->chgChar( 0, 0, "B" );
    mBuf->chgChar( 2, 2, "B" );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    mBuf->delChar( 0, 0, 3 );
    mBuf->chgChar( 2, 2, "Z" );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    performUndoRedo( textHistory );
    performUndoRedo( textHistory );
}

void TestUndo::testUndoLineOperation()
{
    YZUndoBuffer * ub = mBuf->undoBuffer();
    QStringList textHistory;
    textHistory.append( mBuf->getWholeText() );

    mBuf->appendLine( "1111" );
    mBuf->appendLine( "2222" );
    mBuf->appendLine( "3333" );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    mBuf->insertNewLine( 0, 0 );
    mBuf->insertNewLine( 0, 2 );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    mBuf->insertLine( "5555", 0 );
    mBuf->insertLine( "6666", 5 );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    mBuf->deleteLine( 6 );
    mBuf->deleteLine( 0 );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    mBuf->replaceLine( "7777", 0 );
    mBuf->replaceLine( "8888", 4 );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    performUndoRedo( textHistory );
    performUndoRedo( textHistory );
}

void TestUndo::testUndoInsertLine()
{
    YZUndoBuffer * ub = mBuf->undoBuffer();
    QStringList textHistory;
    textHistory.append( mBuf->getWholeText() );

    mBuf->replaceLine("1111", 0 );
    mBuf->insertLine( "5555", 0 );
    mBuf->insertLine( "6666", 1 );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    performUndoRedo( textHistory );
    performUndoRedo( textHistory );
}
 
void TestUndo::testUndoDeleteLine()
{
    YZUndoBuffer * ub = mBuf->undoBuffer();
    QStringList textHistory;
    textHistory.append( mBuf->getWholeText() );

    mBuf->replaceLine("1111", 0 );
    mBuf->appendLine( "2222" );
    mBuf->appendLine( "3333" );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    mBuf->deleteLine( 2 );
    mBuf->deleteLine( 0 );
    ub->commitUndoItem(0,0);
    textHistory.append( mBuf->getWholeText() );
    yzDebug( "TestUndo" ) << "buffer : '" << mBuf->getWholeText() << "'" << endl;
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    performUndoRedo( textHistory );
    performUndoRedo( textHistory );
}

 
void TestUndo::testRedoRemovesUndo()
{
    YZUndoBuffer * ub = mBuf->undoBuffer();
    QStringList textHistory;
    textHistory.append( mBuf->getWholeText() );

    yzDebug("TestUndo") << "step 1" << endl;
    mView->sendMultipleKey("i123<ENTER>456<ENTER>789<ESC>");
    textHistory.append( mBuf->getWholeText() );
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    yzDebug("TestUndo") << "step 2" << endl;
    mView->sendMultipleKey("u");
    phCheckEquals( ub->mayUndo(), false );
    phCheckEquals( ub->mayRedo(), true );
    textHistory.pop_back();

    yzDebug("TestUndo") << "step 3" << endl;
    mView->sendMultipleKey("iaaa<ENTER>bbb<ESC>");
    textHistory.append( mBuf->getWholeText() );
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    yzDebug("TestUndo") << "step 4" << endl;
    mView->sendMultipleKey("iccc<ENTER>ddd<ESC>");
    textHistory.append( mBuf->getWholeText() );
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    performUndoRedo( textHistory, true );

    yzDebug("TestUndo") << "step 5" << endl;
    mView->sendMultipleKey("u");
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), true );
    textHistory.pop_back();

    yzDebug("TestUndo") << "step 6" << endl;
    mView->sendMultipleKey("ieee<ENTER>fff<ESC>");
    textHistory.append( mBuf->getWholeText() );
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    performUndoRedo( textHistory, true );
}

void TestUndo::testCommandUndo()
{
    YZUndoBuffer * ub = mBuf->undoBuffer();
    QStringList textHistory;
    textHistory.append( mBuf->getWholeText() );

    mView->sendMultipleKey("i123<ENTER>456<ENTER>789<ESC>");
    textHistory.append( mBuf->getWholeText() );
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    performUndoRedo( textHistory, true );
    return;

    performUndoRedo( textHistory, true );

    return;

    mView->sendMultipleKey("i123<ENTER>456<ENTER>789<ESC>");
    textHistory.append( mBuf->getWholeText() );
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    performUndoRedo( textHistory, true );
    performUndoRedo( textHistory, true );
}



/* ========================================================================= */

