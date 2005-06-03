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


#ifndef TYZ_SESSION_H
#define TYZ_SESSION_H

#include "TView.h"
#include "TBuffer.h"

 /**
 * $Id: session.h 388 2004-03-03 23:44:13Z mikmak $
 */

#include "libyzis/session.h"

class TYZSession : public YZSession
{
public:
    TYZSession( const QString & sessionName="TestSession" )
    : YZSession( sessionName )
    {}

    virtual YZView* createView ( YZBuffer* buf) {
        yzDebug( AREA_TESTS) << "TYZSession::createView" << endl;
        TYZView * view = new TYZView( buf, YZSession::me );
        buf->addView( view );
        return view;
    }

    virtual	YZBuffer *createBuffer(const QString& path=QString::null) {
        yzDebug( AREA_TESTS) << "TYZSession::createBuffer " << path << endl;
        TYZBuffer * buf = new TYZBuffer( YZSession::me );
        setCurrentView( createView( buf ) );
        buf->load( path );
        currentView()->refreshScreen();
        return buf;
    }

    virtual void popupMessage( const QString& message) {
        yzDebug( AREA_TESTS) << "TYZSession::popupMessage: '" << message << "' \n";
		printf("popupMessage:\n");
		printf("============\n");
		printf("%s\n\n", message.latin1() );
    }

    virtual void quit(bool /*savePopup=true */) {
        yzDebug( AREA_TESTS) << "TYZSession::quit" << endl;
        qApp->closeAllWindows();
        qApp->exit(0);
    }

    virtual void deleteView ( int  ) {
        yzDebug( AREA_TESTS) << "TYZSession::deleteView" << endl;
    }
    virtual void deleteBuffer ( YZBuffer * ) {
        yzDebug( AREA_TESTS) << "TYZSession::deleteBuffer" << endl;
    }
    virtual void changeCurrentView( YZView* ) {
        // notification
        yzDebug( AREA_TESTS) << "TYZSession::changeCurrentView" << endl;
    }
    virtual void setFocusCommandLine( ) {
        yzDebug( AREA_TESTS) << "TYZSession::setFocusCommandLine" << endl;
    }
    virtual void setFocusMainWindow( ) {
        yzDebug( AREA_TESTS) << "TYZSession::setFocusMainWindow" << endl;
    }
	virtual bool quit(int) {
        yzDebug( AREA_TESTS) << "TYZSession::quit" << endl;
        qApp->closeAllWindows();
        qApp->exit(0);
		return true;
    }
	virtual bool promptYesNo(const QString&, const QString&) {
	    yzDebug( AREA_TESTS) << "TYZSession::promptYesNo" << endl;
		return true;
	}
	virtual int promptYesNoCancel( const QString&, const QString& ) {
	    yzDebug( AREA_TESTS) << "TYZSession::promptYesNoCancel" << endl;
		return 0;
	}
	virtual void splitHorizontally(YZView*) {
        yzDebug( AREA_TESTS) << "TYZSession::splitHorizontally" << endl;
	}
};

#endif // TYZ_SESSION_H
