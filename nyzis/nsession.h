/*
	Copyright (c) 2003-2004 Thomas Capricelli <orzel@freehackers.org>
	Copyright (c) 2004-2005 Mickael Marchand <marchand@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef NYZ_SESSION_H
#define NYZ_SESSION_H

/**
 * NYZSession - Ncurses main class for the whole session
 */

/* yzis */
#include "viewwidget.h"
#include "session.h"

class YZViewId;

class NYZSession : public QObject, public YZSession
{
	Q_OBJECT

public:
	static NYZSession *self;
	/**
	 * Constructor. Give a session name to identify/save/load sessions.
	 */
	NYZSession(const char *session_name = "default_nyzis_session", const QString& keys = QString::null );
	virtual ~NYZSession( );

	/*
	 * YZSession interface :
	 */
	virtual bool quit ( int errorCode ) ;
	virtual void popupMessage( const QString& message );
	virtual void deleteBuffer ( YZBuffer *b );
	virtual void setFocusCommandLine();
	virtual void setFocusMainWindow();
	virtual bool promptYesNo( const QString& title, const QString& message );
	virtual int promptYesNoCancel( const QString& title, const QString& message );
	virtual void splitHorizontally( YZView *view );
	virtual void setClipboardText( const QString& text, Clipboard::Mode mode );
	
protected:
	virtual YZView* doCreateView( YZBuffer* buffer );
	virtual void doDeleteView( YZView *view );
	virtual	YZBuffer *doCreateBuffer();

private:
	/**
	 * Fill the map of keycodes ncurses->Qt
	 */
	void initialiseKeycodes();
	
	virtual void changeCurrentView ( YZView * );
	
	/**
	  * mapping ncurses->qt for keycodes
	  */
	static QMap<int,QString> keycodes; // map Ncurses to Qt codes
	QString m_initialCommand;

public Q_SLOTS:
	void init();
	bool processInput( int );

};

#endif // NYZ_SESSION_H

