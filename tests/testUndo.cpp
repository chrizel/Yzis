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
#include "PhilAsserts.h"
using namespace CppUnit;
using namespace std;

#include "libyzis/undo.h"
#include "libyzis/debug.h"

#include "testUndo.h"
#include "TYZSession.h"


// register the suite so that it is run in the runner
CPPUNIT_TEST_SUITE_REGISTRATION( TestUndo );

/* ========================================================================= */

void TestUndo::setUp()
{
    mSession = new TYZSession();
    mBuf = new YZBuffer( mSession );
}

void TestUndo::tearDown()
{
    delete mBuf;
    delete mSession;
}

void TestUndo::testUndoBufferCreation()
{
    UndoBuffer * ub = mBuf->undoBuffer();
    phCheckEquals( ub->mayRedo(), false );
    phCheckEquals( ub->mayUndo(), false );
    ub->undo();
    phCheckEquals( ub->mayRedo(), false );
    phCheckEquals( ub->mayUndo(), false );
    ub->redo();
    phCheckEquals( ub->mayRedo(), false );
    phCheckEquals( ub->mayUndo(), false );
}

void TestUndo::testUndoCharOperation()
{
    UndoBuffer * ub = mBuf->undoBuffer();
    QStringList textHistory;
    textHistory.append( mBuf->getWholeText() );

    mBuf->addChar( 0, 0, "0" );
    textHistory.append( mBuf->getWholeText() );
    ub->commitUndoItem();
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    yzDebug() << "test " << ub->toString() << endl;

    mBuf->addChar( 1, 0, "1" );
    mBuf->addChar( 0, 0, "A" );
    textHistory.append( mBuf->getWholeText() );
    ub->commitUndoItem();
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), false );

    ub->undo();
    phCheckEquals( ub->mayUndo(), true );
    phCheckEquals( ub->mayRedo(), true );
    phCheckEquals( mBuf->getWholeText(), textHistory[1] );

    ub->undo();
    phCheckEquals( ub->mayUndo(), false );
    phCheckEquals( ub->mayRedo(), true );
    phCheckEquals( mBuf->getWholeText(), textHistory[0] );

}

/* ========================================================================= */

