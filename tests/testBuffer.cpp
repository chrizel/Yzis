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

#include <qfile.h>
#include <qfileinfo.h>
#include <string>
#include "philcppunit/PhilAsserts.h"
using namespace CppUnit;
using namespace std;

#include "libyzis/line.h"
#include "libyzis/debug.h"

#include "testBuffer.h"
#include "TSession.h"
#include "TBuffer.h"
#include "TView.h"

/* ========================================================================= */

// register the suite so that it is run in the runner
CPPUNIT_TEST_SUITE_REGISTRATION( TestYZBuffer );

void TestYZBuffer::setUp()
{
    mSession = new TYZSession();
    mBuf = new TYZBuffer( mSession );
    mBuf->clearIntro();
}

void TestYZBuffer::tearDown()
{
    delete mBuf;
    delete mSession;
}

void TestYZBuffer::testCreateEmptyBuffer()
{
    YZBuffer * buf;
    buf = new TYZBuffer( mSession );
    buf->clearIntro();

    phCheckEquals( buf->views().count(), 0 );
    phCheckEquals( buf->firstView(), NULL );
    // an empty buffer has one line ?
    phCheckEquals( buf->lineCount(), 1 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    // there should be a tmp filename
    phCheckNotEquals( buf->fileName(), "" );

    delete buf;
}

void TestYZBuffer::testInsertNewLine()
{
    QString s1 = "0123456789";
    QString s2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    mBuf->replaceLine( s1, 0 );
    phCheckEquals( mBuf->getWholeText(), s1 + "\n" );
    mBuf->insertNewLine( 0, 0);
    phCheckEquals( mBuf->getWholeText(), "\n" + s1 + "\n" );
    mBuf->insertNewLine( 5, 1);
    phCheckEquals( mBuf->getWholeText(), "\n01234\n56789\n" );
    mBuf->insertNewLine( 5, 1);
    phCheckEquals( mBuf->getWholeText(), "\n01234\n\n56789\n" );
    mBuf->insertNewLine( 0, 2);
    phCheckEquals( mBuf->getWholeText(), "\n01234\n\n\n56789\n" );
    mBuf->insertNewLine( 5, 4);
    phCheckEquals( mBuf->getWholeText(), "\n01234\n\n\n56789\n\n" );

    mBuf->clearText();
    phCheckEquals( mBuf->getWholeText(), "" );
    mBuf->insertNewLine( 0, 0);
    phCheckEquals( mBuf->getWholeText(), "\n\n" );
    mBuf->insertNewLine( 0, 0);
    phCheckEquals( mBuf->getWholeText(), "\n\n\n" );
    mBuf->insertNewLine( 0, 1);
    phCheckEquals( mBuf->getWholeText(), "\n\n\n\n" );
}

void TestYZBuffer::testLineMethods()
{
    QString s1 = "0123456789";
    QString s2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    // what happens if I delete a line that does not exist ?
	// nothing because it does exist... we always have at least one line
    mBuf->deleteLine(0);
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->textline( 0 ), "" );

    // add 3 lines
    mBuf->appendLine( s1 );
    phCheckEquals( mBuf->lineCount(), 2 );
    phCheckEquals( mBuf->textline( 1 ), s1 );

    mBuf->appendLine( s2 );
    phCheckEquals( mBuf->lineCount(), 3 );
    phCheckEquals( mBuf->textline(2), s2 );

    mBuf->appendLine( s1 );
    phCheckEquals( mBuf->lineCount(), 4 );
    phCheckEquals( mBuf->textline( 3 ), s1 );

    // delete middle line
    mBuf->deleteLine( 2 );
    phCheckEquals( mBuf->lineCount(), 3 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->textline( 1 ), s1 );
    phCheckEquals( mBuf->textline( 2 ), s1 );

    // delete first line twice
    mBuf->deleteLine( 1 );
    phCheckEquals( mBuf->lineCount(), 2 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->textline( 1 ), s1 );
    mBuf->deleteLine( 1 );
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->textline( 0 ), "" );

    // buffer is empty again
    mBuf->deleteLine(0);
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->textline( 0 ), "" );

    // add 3 lines again
    mBuf->appendLine( s1 );
    mBuf->appendLine( s2 );
    mBuf->appendLine( s1 );
    phCheckEquals( mBuf->lineCount(), 4 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->textline( 1 ), s1 );
    phCheckEquals( mBuf->textline( 2 ), s2 );
    phCheckEquals( mBuf->textline( 3 ), s1 );

    // now delete the lines using the last line
    mBuf->deleteLine(3);
    phCheckEquals( mBuf->lineCount(), 3 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->textline( 1 ), s1 );
    phCheckEquals( mBuf->textline( 2 ), s2 );
    mBuf->deleteLine(2);
    phCheckEquals( mBuf->lineCount(), 2 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->textline( 1 ), s1 );
    mBuf->deleteLine(1);
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->textline( 0 ), "" );

    // add 3 lines again
    mBuf->appendLine( s1 );
    mBuf->appendLine( s2 );
    mBuf->appendLine( s1 );
    phCheckEquals( mBuf->lineCount(), 4 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->textline( 1 ), s1 );
    phCheckEquals( mBuf->textline( 2 ), s2 );
    phCheckEquals( mBuf->textline( 3 ), s1 );

    // now, the nasty tests

    // delete non existing lines should not segfault and simply do nothing
    mBuf->deleteLine( 10 );
    phCheckEquals( mBuf->lineCount(), 4 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->textline( 1 ), s1 );
    phCheckEquals( mBuf->textline( 2 ), s2 );
    phCheckEquals( mBuf->textline( 3 ), s1 );

	// empty the buffer
    mBuf->deleteLine( 3 );
    mBuf->deleteLine( 2 );
    mBuf->deleteLine( 1 );
    mBuf->deleteLine( 0 );
    phCheckEquals( mBuf->getWholeText(), "" );
	
	//replace a non existing line should not do anything
	mBuf->replaceLine( s2 , 10);
    phCheckEquals( mBuf->getWholeText(), "" );

	//replace first line
	mBuf->replaceLine( s2 , 0);
    phCheckEquals( mBuf->textline( 0 ), s2 );
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->getWholeText(), s2+"\n" );

    // empty buffer
    mBuf->deleteLine( 0 );
    phCheckEquals( mBuf->getWholeText(), "" );

    // insertNewLine on non existing line should not do anything
    mBuf->insertNewLine( 10, 10 ); 
    phCheckEquals( mBuf->getWholeText(), "" );

    // add new line to an empty buffer
    mBuf->insertNewLine( 0, 0 ); 
    phCheckEquals( mBuf->getWholeText(), "\n\n" );

    // empty buffer
    mBuf->deleteLine( 0 );
    phCheckEquals( mBuf->getWholeText(), "" );

    mBuf->replaceLine( s1 , 0);
    phCheckEquals( mBuf->getWholeText(), s1+"\n");

    // add new line on non existing column should not do anything
    mBuf->insertNewLine( 30, 0 ); 
    phCheckEquals( mBuf->getWholeText(), s1+"\n");

    // add new line in the middle of the existing line
    mBuf->insertNewLine( 5, 0 ); 
    phCheckEquals( mBuf->getWholeText(), "01234\n56789\n" );
    mBuf->insertNewLine( 5, 0 ); 
    phCheckEquals( mBuf->getWholeText(), "01234\n\n56789\n" );
    mBuf->insertNewLine( 0, 0 ); 
    phCheckEquals( mBuf->getWholeText(), "\n01234\n\n56789\n" );

}

void TestYZBuffer::testGetWholeText()
{
    QString s0 = "1";
    QString s1 = "0123456789";
    QString s2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    QString tmp;

    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->getWholeText(), "" );

    mBuf->replaceLine( s0, 0 );
    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->textline( 0 ), s0 );
    //phCheckEquals( mBuf->textline( 1 ), QString::null );
    //phCheckEquals( mBuf->textline( 1 ), QString::null );
    phCheckEquals( mBuf->getWholeText(), s0 + "\n" );

    mBuf->appendLine( s1 );
    mBuf->appendLine( s2 );
    phCheckEquals( mBuf->lineCount(), 3 );
    phCheckEquals( mBuf->textline( 0 ), s0 );
    phCheckEquals( mBuf->textline( 1 ), s1 );
    phCheckEquals( mBuf->textline( 2 ), s2 );
    phCheckEquals( mBuf->getWholeText(), s0 + "\n" + s1 + "\n" + s2 + "\n" );
}

void TestYZBuffer::testCharMethods()
{
    // what happens with an empty buffer ?
    QString s1 = "0123456789";
    QString s2 = "ABCD";
    QString text = "";

    phCheckEquals( mBuf->lineCount(), 1 );
    phCheckEquals( mBuf->textline( 0 ), "" );
    phCheckEquals( mBuf->getWholeText(), text );

    // try to work on non-existing char
    mBuf->insertChar( 10, 10, QString("Z") );
    phCheckEquals( mBuf->getWholeText(), text );

    mBuf->chgChar( 10, 10, QString("Z") );
    phCheckEquals( mBuf->getWholeText(), text );

    mBuf->delChar( 10, 10, 1 );
    phCheckEquals( mBuf->getWholeText(), text );

    mBuf->appendLine( s1 );
    mBuf->appendLine( s2 );
    mBuf->deleteLine( 0 );
    text = s1 + "\n" + s2 + "\n";
    phCheckEquals( mBuf->getWholeText(), text );

    // add/delete/chg char on a non existent column
    mBuf->insertChar( 11, 0, QString("Z") );
    phCheckEquals( mBuf->getWholeText(), text );
    mBuf->chgChar( 10, 0, QString("Z") );
    phCheckEquals( mBuf->getWholeText(), text );
    mBuf->delChar( 10, 0, 1 );
    phCheckEquals( mBuf->getWholeText(), text );

    // now the test on the real feature
    mBuf->insertChar( 10, 0, QString("Z") );
    mBuf->insertChar( 0, 0, QString("Z") );
    phCheckEquals( mBuf->textline(0), "Z0123456789Z" );

    mBuf->chgChar( 0, 1, QString("Z") );
    mBuf->chgChar( 3, 1, QString("Z") );
    phCheckEquals( mBuf->textline(1), "ZBCZ" );

    mBuf->delChar( 0, 0, 1 );
    mBuf->delChar( 10, 0, 1 );
    phCheckEquals( mBuf->textline(0), "0123456789" );

    mBuf->delChar( 1, 1, 10 );
    phCheckEquals( mBuf->textline(1), "Z" );
}

void TestYZBuffer::testAssertion()
{
    YZASSERT_MSG( 1 , "*** ASSERTION_ERROR ***, ASSERTION SYSTEM NOT WORKING" );
    YZASSERT_MSG( 1 + 2 + 3 + 4 / 2 == 0,"Ok, assertion system is working" );
    YZASSERT_MSG( "true" == "false", "Ok, assertion system is working" );
}

void TestYZBuffer::writeFile( QString fname, QString content )
{
    QFile f( fname );
    phCheckEquals( f.open( IO_WriteOnly ), true );
    QTextStream ts( &f );
    ts << content;
    f.close();
}

QString TestYZBuffer::readFile( QString fname )
{
    QFile f( fname );
    phCheckEquals( f.open( IO_ReadOnly ), true );
    QTextStream ts( &f );
    QString content = ts.read();
    f.close();
    return content;
}

void TestYZBuffer::testLoadSave()
{
    QString text;
    QString fname1 = "test1.txt";

    // save empty buffer and check the file content
    phCheckEquals( mBuf->getWholeText(), "" );
    mBuf->save();
    phCheckEquals( QFileInfo( mBuf->fileName() ).size(), 0 );

    // save non empty buffer and check the file content
    mBuf->replaceLine( "0", 0 );
    mBuf->appendLine( "1" );
    mBuf->appendLine( "2" );
    text = mBuf->getWholeText();
    mBuf->save();
    phCheckEquals( readFile( mBuf->fileName() ), text );

    // load an empty file and check the buffer content
    text = "";
    writeFile( fname1, text );
    phCheckEquals( QFileInfo( fname1 ).size(), 0 );
    mBuf->load( fname1 );
    phCheckEquals( mBuf->getWholeText(), text );

    // load an non-empty file and check the buffer content
    text = "1\n2\n3\n";
    writeFile( fname1, text );
    mBuf->load( fname1 );
    phCheckEquals( mBuf->getWholeText(), text );

    // add a new line at the end of the file if the file does not have one
    // this means that loading a file and saving it might modify it although
    // nothing was done to the file. Gvim does it too so it should be ok.
    text = "1";
    writeFile( fname1, text );
    mBuf->load( fname1 );
    phCheckEquals( mBuf->getWholeText(), text+"\n" );
    mBuf->save();
    phCheckEquals( readFile( mBuf->fileName() ), text+"\n" );

    // save empty buffer and check the file content on a different file
    mBuf->deleteLine(0);
    phCheckEquals( mBuf->getWholeText(), "" );
    mBuf->save();
    phCheckEquals( QFileInfo( mBuf->fileName() ).size(), 0 );

    phCheckEquals( QFile( mBuf->fileName() ).remove(), true );
}

void TestYZBuffer::testFirstNonBlankChar()
{
    phCheckEquals( mBuf->firstNonBlankChar( 10 ), 0 );
    phCheckEquals( mBuf->firstNonBlankChar( 0 ), 0 );
    mBuf->replaceLine( "   123" , 0);
    mBuf->appendLine( "\t\tabc" );
    mBuf->appendLine( " \t abc" );
    phCheckEquals( mBuf->firstNonBlankChar( 0 ), 3 );
    phCheckEquals( mBuf->firstNonBlankChar( 1 ), 2 );
    phCheckEquals( mBuf->firstNonBlankChar( 2 ), 3 );
}

void TestYZBuffer::testGetWholeTextLength()
{
    phCheckEquals( mBuf->getWholeTextLength(), 0 );

    mBuf->replaceLine( "1", 0 );
    phCheckEquals( mBuf->getWholeTextLength(), 2 );
    phCheckEquals( mBuf->getWholeTextLength(), mBuf->getWholeText().length() );

    mBuf->appendLine( "2" );
    phCheckEquals( mBuf->getWholeTextLength(), 4 );
    phCheckEquals( mBuf->getWholeTextLength(), mBuf->getWholeText().length() );
}

void TestYZBuffer::testClearText()
{
    phCheckEquals( mBuf->getWholeTextLength(), 0 );
    mBuf->clearText();
    phCheckEquals( mBuf->getWholeTextLength(), 0 );
    mBuf->replaceLine( "1", 0 );
    phCheckEquals( mBuf->getWholeTextLength(), 2 );
    mBuf->clearText();
    phCheckEquals( mBuf->getWholeTextLength(), 0 );
}

/* ========================================================================= */
