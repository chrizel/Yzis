 /* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Yzis Team <yzis-dev@yzis.org>
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

#include <unistd.h>
#include <string>
#include "philcppunit/PhilAsserts.h"
using namespace CppUnit;
using namespace std;

#include "libyzis/debug.h"

#include "testDebugBackend.h"


// register the suite so that it is run in the runner
CPPUNIT_TEST_SUITE_REGISTRATION( TestDebugBackend );

/* ========================================================================= */

void TestDebugBackend::testOutput()
{
    /* Sorry but this test is in "Guru checks output" mode */

    // all output should show
    yzDebug() << "Some debug output" << endl;
    yzWarning() << "Some warning output" << endl;
    yzError() << "Some error output" << endl;
    yzFatal() << "Some fatal output" << endl;
    printf("4 lines of output\n\n");

    // now with components
    yzDebug("1") << "Some debug output" << endl;
    yzWarning("2") << "Some warning output" << endl;
    printf("2 lines of output\n\n");

    // disable debug and warning
    YZDebugBackend::instance()->setDebugLevel( YZ_ERROR_LEVEL );
    yzDebug("1") << "debug output SHOULD NOT SHOW UP" << endl;
    yzWarning("2") << "warning output SHOULD NOT SHOW UP" << endl;
    yzError("3") << "Some error output" << endl;
    yzFatal("4") << "Some fatal output" << endl;
    printf("2 lines of output\n\n");

    // disable some components
    YZDebugBackend::instance()->enableDebugArea( "1", false );
    YZDebugBackend::instance()->enableDebugArea( "2", false );
    yzError("1") << "area 1 output SHOULD NOT SHOW UP" << endl;
    yzError("2") << "area 2 output SHOULD NOT SHOW UP" << endl;
    yzError("3") << "Some error output" << endl;
    printf("1 line of output\n\n");

    // enable warning again
    YZDebugBackend::instance()->setDebugLevel( YZ_WARNING_LEVEL );
    yzDebug("1") << "debug output SHOULD NOT SHOW UP" << endl;
    yzDebug("3") << "debug output SHOULD NOT SHOW UP" << endl;
    yzWarning("2") << "warning output SHOULD NOT SHOW UP" << endl;
    yzWarning("3") << "some warning output" << endl;
    yzError("2") << "area 2 output SHOULD NOT SHOW UP" << endl;
    yzError("3") << "Some error output" << endl;
    yzFatal("4") << "Some fatal output" << endl;
    printf("3 lines of output\n\n");

    // enable components again
    YZDebugBackend::instance()->enableDebugArea( "2", true );
    yzWarning("2") << "some warning output" << endl;
    printf("1 line of output\n\n");
}

void TestDebugBackend::testParseRcFile()
{
    const char * fname = "debugrc";
    FILE * f = fopen(fname, "w" );
    fprintf(f, "enable:1\n" );
    fprintf(f, "disable:2\n" );
    fprintf(f, "disable:3 asdl;fj \n" );
    fclose(f);

    YZDebugBackend * dbi = YZDebugBackend::instance();
    dbi->init();

    phCheckEquals( dbi->isAreaEnabled("0"), true );    
    phCheckEquals( dbi->isAreaEnabled("1"), true );    
    phCheckEquals( dbi->isAreaEnabled("2"), true );    
    phCheckEquals( dbi->isAreaEnabled("3"), true );    

    dbi->parseRcfile( fname );
    phCheckEquals( dbi->isAreaEnabled("0"), true );    
    phCheckEquals( dbi->isAreaEnabled("1"), true );    
    phCheckEquals( dbi->isAreaEnabled("2"), false );    
    phCheckEquals( dbi->isAreaEnabled("3"), false );    

    unlink(fname);
}
