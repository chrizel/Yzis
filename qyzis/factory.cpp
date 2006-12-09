/*  This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <mikmak@yzis.org>,
 *  Copyright (C) 2003-2004 Lucijan Busch <lucijan@kde.org>
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

#include <assert.h>
#include <unistd.h>
#include <qapplication.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <qtextcodec.h>
#include <QClipboard>
#include <QSettings>
#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include "factory.h"
#include "viewwidget.h"
#include "debug.h"
#include "yzis.h"
#include "qyzis.h"
#include "debug.h"

QYZisSession* QYZisSession::m_instance = NULL;

QYZisSession::QYZisSession() :
		YZSession("qyzis"),
		lastView(0),
		m_viewParent(0)
{
	m_instance = this;
	guiStarted();
}

QYZisSession::~QYZisSession() {
	yzDebug() << "QYZisSession gets destroyed !" << endl;
	m_instance = NULL;
}

QYZisSession* QYZisSession::self() {
	if ( !m_instance ) {
		new QYZisSession();
	}
	return m_instance;
}

void QYZisSession::setClipboardText( const QString& text, Clipboard::Mode mode ) {
#ifdef Q_WS_X11
	if ( QX11Info::display() )
#endif
		QApplication::clipboard()->setText( text, mode == Clipboard::Clipboard ? QClipboard::Clipboard : QClipboard::Selection );
}

bool QYZisSession::quit( int /*errorCode*/ ) {
	if (Qyzis::me) {
		qApp->quit();
	} else if ( currentView() && currentView()->modePool()->currentType() == YZMode::MODE_EX 
				&& !currentView()->getCommandLineText().isEmpty() ) {
		return false;
	}


	return true;
}

void QYZisSession::applyConfig() {
	QSettings settings;
	// apply new configuration to all views
	YZBufferList::ConstIterator it = buffers().begin();
	YZBufferList::ConstIterator end = buffers().end();
	for ( ; it != end; ++it ) {
		YZBuffer *b = *it;
		YZList<YZView*> l = b->views();
		for ( YZList<YZView*>::Iterator itr = l.begin(); itr != l.end(); ++itr ) {
			QYZisView* yv = static_cast<QYZisView*>( *itr );
			yv->applyConfig( settings );
		}
	}
}

void QYZisSession::changeCurrentView( YZView* view ) {
	yzDebug() << "QyzisFactory : setCurrentView " << view->getId() << endl;
	QYZisView *v = static_cast<QYZisView*>(view);
	v->activateWindow();
	v->setFocus();
}

YZView* QYZisSession::doCreateView( YZBuffer *buffer ) {
	QYZisView *view;

	view = new QYZisView( buffer, m_viewParent, QString(buffer->fileName() + "-view").toUtf8().constData() );
	buffer->addView( view );

	if ( Qyzis::me ) {
		Qyzis::me->embedPartView( view, buffer->fileName(), buffer->fileName() );
	} else {
		//?			view->setMdiChildView( 0 );
		yzError() << "unhandled case in YZView* QYZisSession::doCreateView( YZBuffer *buffer ) " ;
	}

	view->setFocus();
	view->show();

	return view;
}

YZBuffer *QYZisSession::doCreateBuffer() {
	return new YZBuffer;
}

void QYZisSession::popupMessage( const QString& message ) {
	QYZisView *v = static_cast<QYZisView*>(currentView());
	QMessageBox::information(v, _( "Error" ), message);
}

void QYZisSession::closeView() {
	if (Qyzis::me) {
		Qyzis::me->closeTab();
	} else {
		lastView->close();
	}
	lastView = 0;
}

void QYZisSession::doDeleteView( YZView *view ) {
	QYZisView *kview = dynamic_cast<QYZisView*>(view);
	if ( kview ) {
		lastView = kview;
		QTimer::singleShot(0, this, SLOT( closeView() ));
	}
}

void QYZisSession::deleteBuffer(YZBuffer* b) {
	delete b;
}

void QYZisSession::setFocusMainWindow() {
	QYZisView *yv = static_cast<QYZisView*>( currentView() );
	yv->setFocusMainWindow();
}

void QYZisSession::setFocusCommandLine() {
	QYZisView *yv = static_cast<QYZisView*>( currentView() );
	yv->setFocusCommandLine();
}

bool QYZisSession::promptYesNo(const QString& title, const QString& message) {
	int v = QMessageBox::question(static_cast<QYZisView*>(currentView()), title, message, _("Yes"), _("No"));
	if (v == 0) return true;
	else return false;
}

int QYZisSession::promptYesNoCancel(const QString& title, const QString& message) {
	return QMessageBox::question(static_cast<QYZisView*>(currentView()), title, message, _("Yes"), _("No"), _("Cancel"));
}

void QYZisSession::splitHorizontally(YZView* /*view*/) {
}


