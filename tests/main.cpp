/* ============================================================================
   Project Name : jayaCard
   Module Name  : inkit - main test file
   Version      : $Id: main.cpp,v 1.3 2004/01/09 16:16:08 philippe Exp $

	Description: main for all the tests

    The Original Code is inKit code, the contactless library of the jayacard
    project (http://www.jayacard.org).

    The contents of this file are subject to the Mozilla Public License Version
    1.1 (the "License"); you may not use this file except in compliance with
    the License. You may obtain a copy of the License at
    http://www.mozilla.org/MPL/

    Software distributed under the License is distributed on an "AS IS" basis,
    WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
    for the specific language governing rights and limitations under the
    License.

    The Initial Developer of the Original Code is inSeal SAS and the authors
    Gilles Dumortier and Philippe Fremy.

    Portions created by the Initial Developer are Copyright (C) 1996-2003 the
    Initial Developer. All Rights Reserved.

    Alternatively, the contents of this file may be used under the terms of
    either the GNU General Public License Version 2 or later (the "GPL"), or
    the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
    in which case the provisions of the GPL or the LGPL are applicable instead
    of those above. If you wish to allow use of your version of this file only
    under the terms of either the GPL or the LGPL, and not to allow others to
    use your version of this file under the terms of the MPL, indicate your
    decision by deleting the provisions above and replace them with the notice
    and other provisions required by the GPL or the LGPL. If you do not delete
    the provisions above, a recipient may use your version of this file under
    the terms of any one of the MPL, the GPL or the LGPL.

    inKit is also available under commercial license. For pricing and ordering
    information, send an email to sales@inseal.com

   History Rev	Description
   020403  phf	wrote it from scratch
   ============================================================================
*/

#include "testYZBuffer.h"
#include "PhilTestRunner.h"

int main(int argc, char ** argv)
{
	bool doWait = false;
#ifdef WIN32
	doWait = true;
#endif
	int ret;
	PhilTestRunner runner;
	runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );

	if (argc > 1) {
		for( int i=1; i<argc; i++) {
			ret = runner.run( argv[i], doWait, true, true ) && ret;
		}
	} else {
		runner.run( "", doWait, true, true );
	}
	return 0;
}




