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
/**
 * $Id: session.h 388 2004-03-03 23:44:13Z mikmak $
 */

#include <libyzis/session.h>

class TYZSession : public YZSession
{
public:
    TYZSession( const QString & sessionName="TestSession" )
    : YZSession( sessionName ) 
    {}

    virtual YZView* createView ( YZBuffer* ) { 
        yzDebug( AREA_TESTS) << "TYZSession::createView" << endl;
        return NULL; 
    }

    virtual	YZBuffer *createBuffer(const QString& path=QString::null) { 
        yzDebug( AREA_TESTS) << "TYZSession::createBuffer " << path << endl;
        return NULL; 
    }

    virtual void popupMessage( const QString& message) {
        yzDebug( AREA_TESTS) << "TYZSession::popupMessage: '" << message << "' \n";
    }

    virtual void quit(bool /*savePopup=true */) {
        yzDebug( AREA_TESTS) << "TYZSession::quit" << endl;
    }
    virtual void deleteView ( ) {
        yzDebug( AREA_TESTS) << "TYZSession::deleteView" << endl;
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
};

#endif // TYZ_SESSION_H
