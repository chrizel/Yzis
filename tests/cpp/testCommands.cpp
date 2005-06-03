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
#include <string>
#include "philcppunit/PhilAsserts.h"
using namespace CppUnit;
using namespace std;

#include "libyzis/line.h"
#include "libyzis/debug.h"

#include "testCommands.h"
#include "TSession.h"
#include "TView.h"
#include "TBuffer.h"

// register the suite so that it is run in the runner
CPPUNIT_TEST_SUITE_REGISTRATION( TestYZCommands );

// some useful macro
#define CHECK_MODE_INSERT( view ) phCheckEquals( view->modePool()->currentType(), YZMode::MODE_INSERT );
#define CHECK_MODE_COMMAND( view ) phCheckEquals( view->modePool()->currentType(), YZMode::MODE_COMMAND );
#define CHECK_CURSOR_POS( view, line, col ) { phCheckEquals( view->getCursorLine(), line ); phCheckEquals( view->getCursorCol(), col ); }

/* ========================================================================= */
void TestYZCommands::setUp()
{
    mLines = 5;
    mSession = new TYZSession();
    mBuf = new TYZBuffer( mSession );
    mView = new TYZView( mBuf, mSession, mLines );
	mBuf->addView( mView );
}

void TestYZCommands::tearDown()
{
    delete mView;
    delete mBuf;
    delete mSession;
}

void TestYZCommands::testCreateSession()
{
    phCheckEquals( mBuf->views().count(), 1 );
    phCheckEquals( mBuf->firstView(), (YZView *) mView );
    phCheckEquals( mBuf->getWholeText(), "" );
    CHECK_MODE_COMMAND( mView );
    CHECK_CURSOR_POS( mView, 0, 0 );
}

void TestYZCommands::testInsertMode()
{
    mView->sendMultipleKey( "i" );
    phCheckEquals( mBuf->getWholeText(), "" );
    CHECK_MODE_INSERT( mView );
    CHECK_CURSOR_POS( mView, 0, 0 );

    mView->sendMultipleKey( "i23" );
    phCheckEquals( mBuf->getWholeText(), "i23\n" );
    CHECK_MODE_INSERT( mView );
    CHECK_CURSOR_POS( mView, 0, 3 );

    mView->sendMultipleKey( "<ENTER>456" );
    phCheckEquals( mBuf->getWholeText(), "i23\n456\n" );
    CHECK_MODE_INSERT( mView );
    CHECK_CURSOR_POS( mView, 1, 3 );

    mView->sendMultipleKey( "<ESC>" );
    phCheckEquals( mBuf->getWholeText(), "i23\n456\n" );
    CHECK_MODE_COMMAND( mView );
    CHECK_CURSOR_POS( mView, 1, 2 );

    mView->sendMultipleKey( "<ESC>" );
    phCheckEquals( mBuf->getWholeText(), "i23\n456\n" );
    CHECK_MODE_COMMAND( mView );
    CHECK_CURSOR_POS( mView, 1, 2 );
}

void TestYZCommands::testCharMovement()
{
    mView->sendMultipleKey( "i0123<ENTER>4567<ENTER>89AB<ENTER>CDEF<ESC>" );
    phCheckEquals( mBuf->getWholeText(), "0123\n4567\n89AB\nCDEF\n" );
    CHECK_MODE_COMMAND( mView );
    CHECK_CURSOR_POS( mView, 3, 3 );
    yzDebug("testCharMovement") << "buffer : '" << mBuf->getWholeText() << "'" << endl;

    mView->sendMultipleKey( "<RIGHT>" );
    mView->sendMultipleKey( "<DOWN>" );
    CHECK_CURSOR_POS( mView, 3, 3 );

    mView->sendMultipleKey( "<LEFT>" );
    CHECK_CURSOR_POS( mView, 3, 2 );

    mView->sendMultipleKey( "<UP>" );
    CHECK_CURSOR_POS( mView, 2, 2 );

    mView->sendMultipleKey( "<UP>" );
    mView->sendMultipleKey( "<UP>" );
    mView->sendMultipleKey( "<UP>" );
    CHECK_CURSOR_POS( mView, 0, 2 );
    mView->sendMultipleKey( "<UP>" );
    CHECK_CURSOR_POS( mView, 0, 2 );

    mView->sendMultipleKey( "<LEFT>" );
    mView->sendMultipleKey( "<LEFT>" );
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey( "<LEFT>" );
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey( "<DOWN>" );
    CHECK_CURSOR_POS( mView, 1, 0 );

    mView->sendMultipleKey( "2<RIGHT>" );
    mView->sendMultipleKey( "<RIGHT>" );
    mView->sendMultipleKey( "2<DOWN>" );
    CHECK_CURSOR_POS( mView, 3, 3 );

    mView->sendMultipleKey( "10<LEFT>" );
    CHECK_CURSOR_POS( mView, 3, 0 );
    mView->sendMultipleKey( "10<UP>" );
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey( "10<RIGHT>" );
    CHECK_CURSOR_POS( mView, 0, 3 );
    mView->sendMultipleKey( "10<DOWN>" );
    CHECK_CURSOR_POS( mView, 3, 3 );

    // now with hjkl

    mView->sendMultipleKey( "l" );
    mView->sendMultipleKey( "j" );
    CHECK_CURSOR_POS( mView, 3, 3 );

    mView->sendMultipleKey( "h" );
    CHECK_CURSOR_POS( mView, 3, 2 );

     mView->sendMultipleKey( "k" );
    CHECK_CURSOR_POS( mView, 2, 2 );

    mView->sendMultipleKey( "3k" );
    CHECK_CURSOR_POS( mView, 0, 2 );
    mView->sendMultipleKey( "k" );
    CHECK_CURSOR_POS( mView, 0, 2 );

    mView->sendMultipleKey( "2h" );
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey( "h" );
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey( "j" );
    CHECK_CURSOR_POS( mView, 1, 0 );

    mView->sendMultipleKey( "3l" );
    mView->sendMultipleKey( "2j" );
    CHECK_CURSOR_POS( mView, 3, 3 );
}

void TestYZCommands::testBeginEndCharMovement()
{
	YZSession::mOptions->setGroup("Global");
	YZSession::mOptions->getOption("cindent")->setBoolean(false);
    mView->sendMultipleKey( "i<TAB>0123<ENTER>4567<ENTER>  89AB <ESC>" );
    CHECK_CURSOR_POS( mView, 2, 6 );

    // test beginning and end of line movements
    mView->sendMultipleKey("^");
    CHECK_CURSOR_POS( mView, 2, 2 );
    mView->sendMultipleKey("0");
    CHECK_CURSOR_POS( mView, 2, 0 );
    mView->sendMultipleKey("$");
    CHECK_CURSOR_POS( mView, 2, 6 );

    mView->sendMultipleKey("<UP>");
    mView->sendMultipleKey("0");
    CHECK_CURSOR_POS( mView, 1, 0 );
    mView->sendMultipleKey("^");
    CHECK_CURSOR_POS( mView, 1, 0 );
    mView->sendMultipleKey("$");
    CHECK_CURSOR_POS( mView, 1, 3 );

    mView->sendMultipleKey("<UP>");
    mView->sendMultipleKey("0");
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey("^");
    CHECK_CURSOR_POS( mView, 0, 1 );
    mView->sendMultipleKey("$");
    CHECK_CURSOR_POS( mView, 0, 4 );
}

void TestYZCommands::testLineMovement()
{
	//we test 'gg' like commands, make sure :set startofline=true first
	YZSession::mOptions->setGroup("Global");
	YZSession::mOptions->getOption("startofline")->setBoolean(true);
    mView->sendMultipleKey( "i<TAB><TAB>0123<ENTER>4567<ENTER>89AB<ENTER> CDEF<ESC>" );
    phCheckEquals( mBuf->getWholeText(), "\t\t0123\n4567\n89AB\n CDEF\n" );
    CHECK_MODE_COMMAND( mView );
    CHECK_CURSOR_POS( mView, 3, 4 );

    mView->sendMultipleKey( "gg" );
    CHECK_CURSOR_POS( mView, 0, 2 ); //depends on :set startofline value
    mView->sendMultipleKey( "<RIGHT>" );
    CHECK_CURSOR_POS( mView, 0, 3 );
    mView->sendMultipleKey( "gg" );
    CHECK_CURSOR_POS( mView, 0, 2 );

    mView->sendMultipleKey( "G" );
    CHECK_CURSOR_POS( mView, 3, 1 );
    mView->sendMultipleKey( "<RIGHT>" );
    CHECK_CURSOR_POS( mView, 3, 2 );
    mView->sendMultipleKey( "G" );
    CHECK_CURSOR_POS( mView, 3, 1 );

    mView->sendMultipleKey("0gg");
    CHECK_CURSOR_POS( mView, 0, 2 );
    mView->sendMultipleKey( "300G" );
    CHECK_CURSOR_POS( mView, 3, 1 );
    mView->sendMultipleKey("2gg");
    CHECK_CURSOR_POS( mView, 1, 0 );
    mView->sendMultipleKey( "300gg" );
    CHECK_CURSOR_POS( mView, 3, 1 );
    mView->sendMultipleKey("3G");
    CHECK_CURSOR_POS( mView, 2, 0 );

    mView->sendMultipleKey("$");
    CHECK_CURSOR_POS( mView, 2, 3 );
    mView->sendMultipleKey("0");
    CHECK_CURSOR_POS( mView, 2, 0 );

    //tests with startofline to false now
	YZSession::mOptions->getOption("startofline")->setBoolean(false);
    mView->sendMultipleKey("gg");
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey("G");
    CHECK_CURSOR_POS( mView, 3, 0 );
}

void TestYZCommands::testMotionMovement() {
	YZSession::mOptions->getOption("startofline")->setBoolean(true);
    //tests with spaces on one line
    mView->sendMultipleKey( "iword1 word02 word03 word4 word05<ESC>" );
    phCheckEquals( mBuf->getWholeText(), "word1 word02 word03 word4 word05\n" );
    mView->sendMultipleKey("gg");
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey("w");
    CHECK_CURSOR_POS( mView, 0, 6 );
    mView->sendMultipleKey("w");
    CHECK_CURSOR_POS( mView, 0, 13 );
    mView->sendMultipleKey("b");
    CHECK_CURSOR_POS( mView, 0, 6 );
    mView->sendMultipleKey("2w");
    CHECK_CURSOR_POS( mView, 0, 20);
    mView->sendMultipleKey("2b");
    CHECK_CURSOR_POS( mView, 0, 6);
    mView->sendMultipleKey("99b");
    CHECK_CURSOR_POS( mView, 0, 0);
    mView->sendMultipleKey("99w");
    CHECK_CURSOR_POS( mView, 0, 31);
    mView->sendMultipleKey( "dd" );

    //tests with delimiters on one line
    mView->sendMultipleKey( "itest/function(test)/method()/test();<ESC>" );
    phCheckEquals( mBuf->getWholeText(), "test/function(test)/method()/test();\n" );
    mView->sendMultipleKey("gg");
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey("w");
    CHECK_CURSOR_POS( mView, 0, 4 );
    mView->sendMultipleKey("w");
    CHECK_CURSOR_POS( mView, 0, 5 );
    mView->sendMultipleKey("b");
    CHECK_CURSOR_POS( mView, 0, 4 );
    mView->sendMultipleKey("2w");
    CHECK_CURSOR_POS( mView, 0, 13);
    mView->sendMultipleKey("2b");
    CHECK_CURSOR_POS( mView, 0, 4);
    mView->sendMultipleKey("99b");
    CHECK_CURSOR_POS( mView, 0, 0);
    mView->sendMultipleKey("9w");
    CHECK_CURSOR_POS( mView, 0, 33 );
    mView->sendMultipleKey("w");
    CHECK_CURSOR_POS( mView, 0, 35 );
    mView->sendMultipleKey( "dd" );

    //tests with spaces on multiple lines
    mView->sendMultipleKey( "itest1 test02 test003 test04 test5<ENTER>test1 test002 test03 test004 test5<ENTER> test1 test2 <ESC>" );
    phCheckEquals( mBuf->getWholeText(), "test1 test02 test003 test04 test5\ntest1 test002 test03 test004 test5\n test1 test2 \n" );
    mView->sendMultipleKey("gg");
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey("5w");
    CHECK_CURSOR_POS( mView, 1, 0 );
    mView->sendMultipleKey("gg");
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey("10w");
    CHECK_CURSOR_POS( mView, 2, 1 );
    mView->sendMultipleKey("gg");
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey("11w");
    CHECK_CURSOR_POS( mView, 2, 7 );
    mView->sendMultipleKey("w");
    CHECK_CURSOR_POS( mView, 2, 12 );
    //test backward
    mView->sendMultipleKey("G$");
    CHECK_CURSOR_POS( mView, 2, 12 );
    mView->sendMultipleKey("5b");
    CHECK_CURSOR_POS( mView, 1, 14 );
    mView->sendMultipleKey("G$");
    CHECK_CURSOR_POS( mView, 2, 12 );
    mView->sendMultipleKey("10b");
    CHECK_CURSOR_POS( mView, 0, 13 );
    mView->sendMultipleKey("G$");
    CHECK_CURSOR_POS( mView, 2, 12 );
    mView->sendMultipleKey("11b");
    CHECK_CURSOR_POS( mView, 0, 6 );
    mView->sendMultipleKey("b");
    CHECK_CURSOR_POS( mView, 0, 0 );

    //tests with delimiters on multiple lines
	YZSession::mOptions->getOption("cindent")->setBoolean(false);
    mView->sendMultipleKey( "ggVGd" );
    mView->sendMultipleKey( "itest/function(test)/class::method()/<ENTER><TAB>void yzis::method(test()){<ENTER><TAB><TAB>printf(truc);<ESC>" );
    phCheckEquals( mBuf->getWholeText(), "test/function(test)/class::method()/\n\tvoid yzis::method(test()){\n\t\tprintf(truc);\n" );
    mView->sendMultipleKey("gg");
    CHECK_CURSOR_POS( mView, 0, 0 );
    mView->sendMultipleKey("10w");
    CHECK_CURSOR_POS( mView, 1, 1 );
    mView->sendMultipleKey("8w");
    CHECK_CURSOR_POS( mView, 2, 8 );
    mView->sendMultipleKey("5b");
    CHECK_CURSOR_POS( mView, 1, 12 );
    //cleanup
    mView->sendMultipleKey( "ggVGd" );
}

/* ========================================================================= */
