 /* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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

#include "libyzis/debug.h"

#include "testSearch.h"
#include "TSession.h"
#include "TBuffer.h"

#define CHECK_CURSOR_POS( view, col, line ) { phCheckEquals( view->getCursorLine(), line ); phCheckEquals( view->getCursorCol(), col ); }

// register the suite so that it is run in the runner
CPPUNIT_TEST_SUITE_REGISTRATION( TestSearch );

/* ========================================================================= */

void TestSearch::setUp()
{
    mLines = 5;
    mSession = new TYZSession();
    mBuf = new TYZBuffer( mSession );
    mView = new TYZView( mBuf, mSession, mLines );
}

void TestSearch::tearDown()
{
	delete mView;
    delete mBuf;
    delete mSession;
}

void TestSearch::testSearchOneLine()
{
	QString s1 = "one simple string !";
	mBuf->appendLine(s1);
	mView->gotoxy( 2, 1 );
	mView->doSearch( "simple" );
    CHECK_CURSOR_POS( mView, 4, 1 );
}

void TestSearch::testSearchBeginningOfLine()
{
	QString s1 = "simple string, very simple !";
	mBuf->appendLine(s1);
	mView->gotoxy( 0, 1 );
	mView->doSearch( "simple" );
    CHECK_CURSOR_POS( mView, 20, 1 );
}

/* ========================================================================= */

