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

#include "testYZBuffer.h"

#include <string>
using namespace CppUnit;
using namespace std;

#include "PhilAsserts.h"

#include "libyzis/line.h"
#include "libyzis/debug.h"

/* ========================================================================= */

// register the suite so that it is run in the runner
CPPUNIT_TEST_SUITE_REGISTRATION( TestYZBuffer );

void TestYZBuffer::setUp()
{
    mSession = new YZSession( "TestSession" );
    mBuf = new YZBuffer( mSession );
}

void TestYZBuffer::tearDown()
{
    delete mBuf;
    delete mSession;
}

void TestYZBuffer::testCreateEmptyBuffer()
{
    YZBuffer * buf;
    buf = new YZBuffer( mSession );

    phCheckEquals( buf->views().count(), 0 );
    phCheckEquals( buf->firstView(), NULL );
    // an empty buffer has one line ?
    phCheckEquals( buf->lineCount(), 1 );
    phCheckEquals( mBuf->data( 0 ), "" );
    // there should be a tmp filename
    phCheckNotEquals( buf->fileName(), "" );

    delete buf;
}

void TestYZBuffer::testLineMethods()
{
    QString s1 = "0123456789";
    QString s2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // what happens if I delete a line that does not exist ?
	// nothing because it does exist... we always have at least one line
    mBuf->deleteLine(0);
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->data( 0 ), "" );

    // add 3 lines
    mBuf->addLine( s1 );
    phCheckEquals( mBuf->lineCount(), 2 );
    phCheckEquals( mBuf->data( 1 ), s1 );
    phCheckEquals( mBuf->at(1)->data(), s1 );

    mBuf->addLine( s2 );
    phCheckEquals( mBuf->lineCount(), 3 );
    phCheckEquals( mBuf->data(2), s2 );
    phCheckEquals( mBuf->at(2)->data(), s2 );

    mBuf->addLine( s1 );
    phCheckEquals( mBuf->lineCount(), 4 );
    phCheckEquals( mBuf->data( 3 ), s1 );
    phCheckEquals( mBuf->at(3)->data(), s1 );

    // delete middle line
    mBuf->deleteLine( 2 );
    phCheckEquals( mBuf->lineCount(), 3 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->data( 1 ), s1 );
    phCheckEquals( mBuf->data( 2 ), s1 );

    // delete first line twice
    mBuf->deleteLine( 1 );
    phCheckEquals( mBuf->lineCount(), 2 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->data( 1 ), s1 );
    mBuf->deleteLine( 1 );
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->data( 0 ), "" );

    // buffer is empty again
    mBuf->deleteLine(0);
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->data( 0 ), "" );

    // add 3 lines again
    mBuf->addLine( s1 );
    mBuf->addLine( s2 );
    mBuf->addLine( s1 );
    phCheckEquals( mBuf->lineCount(), 4 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->data( 1 ), s1 );
    phCheckEquals( mBuf->data( 2 ), s2 );
    phCheckEquals( mBuf->data( 3 ), s1 );

    // now delete the lines using the last line
    mBuf->deleteLine(3);
    phCheckEquals( mBuf->lineCount(), 3 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->data( 1 ), s1 );
    phCheckEquals( mBuf->data( 2 ), s2 );
    mBuf->deleteLine(2);
    phCheckEquals( mBuf->lineCount(), 2 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->data( 1 ), s1 );
    mBuf->deleteLine(1);
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->data( 0 ), "" );

    // add 3 lines again
    mBuf->addLine( s1 );
    mBuf->addLine( s2 );
    mBuf->addLine( s1 );
    phCheckEquals( mBuf->lineCount(), 4 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->data( 1 ), s1 );
    phCheckEquals( mBuf->data( 2 ), s2 );
    phCheckEquals( mBuf->data( 3 ), s1 );

    // now, the nasty tests

    // delete non existing lines should not segfault and simply do nothing
    mBuf->deleteLine( 10 );
    phCheckEquals( mBuf->lineCount(), 4 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->data( 1 ), s1 );
    phCheckEquals( mBuf->data( 2 ), s2 );
    phCheckEquals( mBuf->data( 3 ), s1 );
}

void TestYZBuffer::testGetWholeText()
{
    QString s1 = "0123456789";
    QString s2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString tmp;

    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->getWholeText(), "" );

    mBuf->addLine( s1 );
    mBuf->addLine( s2 );
    phCheckEquals( mBuf->lineCount(), 3 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->data( 1 ), s1 );
    phCheckEquals( mBuf->getWholeText(), "\n" + s1 + "\n" + s2 );
}

void TestYZBuffer::testCharMethods()
{
    // what happens with an empty buffer ?
    QString s1 = "0123456789";
    QString s2 = "ABCD";
    QString text = "";

    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->data( 0 ), "" );
    phCheckEquals( mBuf->getWholeText(), text );

    // what happens if I delete a line that does not exist ?
    mBuf->addChar( 10, 10, QString("Z") );
    phCheckEquals( mBuf->getWholeText(), text );

    mBuf->chgChar( 10, 10, QString("Z") );
    phCheckEquals( mBuf->getWholeText(), text );

    mBuf->delChar( 10, 10, 1 );
    phCheckEquals( mBuf->getWholeText(), text );

    mBuf->addLine( s1 );
    mBuf->addLine( s2 );
    mBuf->deleteLine( 0 );
    text = s1 + "\n" + s2;
    phCheckEquals( mBuf->getWholeText(), text );

    // add/delete/chg char on a non existent column
    mBuf->addChar( 11, 0, QString("Z") );
    phCheckEquals( mBuf->getWholeText(), text );
    mBuf->chgChar( 10, 0, QString("Z") );
    phCheckEquals( mBuf->getWholeText(), text );
    mBuf->delChar( 10, 0, 1 );
    phCheckEquals( mBuf->getWholeText(), text );

    // now the test on the real feature
    mBuf->addChar( 10, 0, QString("Z") );
    mBuf->addChar( 0, 0, QString("Z") );
    phCheckEquals( mBuf->data(0), "Z0123456789Z" );

    mBuf->chgChar( 0, 1, QString("Z") );
    mBuf->chgChar( 3, 1, QString("Z") );
    phCheckEquals( mBuf->data(1), "ZBCZ" );

    mBuf->delChar( 0, 0, 1 );
    mBuf->delChar( 10, 0, 1 );
    phCheckEquals( mBuf->data(0), "0123456789" );

    mBuf->delChar( 1, 1, 10 );
    phCheckEquals( mBuf->data(1), "Z" );
}

void TestYZBuffer::testViewAllocation()
{

}

void TestYZBuffer::testAssertion()
{
    YZASSERT( 1 , "*** ASSERTION_ERROR ***, ASSERTION SYSTEM NOT WORKING" );
    YZASSERT( 1 + 2 + 3 + 4 / 2 == 0,"Ok, assertion system is working" );
    YZASSERT( "true" == "false", "Ok, assertion system is working" );
}


/* ========================================================================= */
