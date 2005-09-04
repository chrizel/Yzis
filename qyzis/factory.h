/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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


#ifndef QYZIS_FACTORY_H
#define QYZIS_FACTORY_H

#include <QMap>

#include "session.h"
#include "viewid.h"

class YZBuffer;
class QYZisView;
class YZViewId;

class QYZisFactory : public YZSession
{
	Q_OBJECT
public:
	QYZisFactory();
	virtual ~QYZisFactory();


	static QYZisFactory* self();

	//GUI interface
	bool quit(int errorCode);
	void popupMessage( const QString& message );
	void deleteBuffer ( YZBuffer *b );
	void setFocusCommandLine();
	void setFocusMainWindow();
	void splitHorizontally(YZView *view);
	bool promptYesNo(const QString& title, const QString& message);
	int promptYesNoCancel(const QString& title, const QString& message);
	virtual void setClipboardText( const QString& text, Clipboard::Mode mode );
	
	/**
	 * Sets the parent widget for all created QYZisView
	 * This is an ugly hack to get around the deep calls into
	 * YZSession in order to create Views.  These calls cannot
	 * have parent information passed around.  createPartObject
	 * can look in this field to get a parent
	 */
	void setViewParentWidget( QWidget *viewParent ) { m_viewParent = viewParent; }
		//Editor Interface
	YZView *doCreateView( YZBuffer* buffer );
	void doDeleteView( YZView *view );
	
	/**
	 * Opens a new buffer
	 * @param path file to which the buffer is linked
	 * @return the filename of the actual created buffer
	 */
	YZBuffer* doCreateBuffer();


	void writeConfig();
	void readConfig(); 
//	void writeConfig(KConfig *) {}
//	void readConfig(KConfig *) {}
	void configDialog ( QWidget * ) { } //for now FIXME TODO FIXME TODO
	bool configDialogSupported() const { return false;} 
	int configPages () const { return 0; }
//	QString configPageName ( int /*number*/ ) const { return ""; } 
//	QString configPageFullName ( int /*number*/ ) const { return ""; }
//	QPixmap configPagePixmap ( int /*number*/, int /*size*/ = KIcon::SizeSmall ) const { return QPixmap(); }
	

public slots:
	void applyConfig();
	void closeView();

private:
	
	void changeCurrentView( YZView* );

public:
	QYZisView *lastView;
	QWidget *m_viewParent;
};

#endif
