/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
 *  Copyright (C) 2005 Erlend Hamberg <ehamberg@online.no>
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

/**
 * $Id: viewwidget.cpp 2072 2005-09-01 11:01:40Z mikmak $
 */

#include <QEvent>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QSettings>
#include <QTimer>
#include <qapplication.h>
#include <viewcursor.h>

#include <debug.h>

#include "viewwidget.h"

#include "yzis.h"
#include "mode_visual.h"
#include "factory.h"
#include "debug.h"
#include "qyzis.h"

QYZisView::QYZisView ( YZBuffer *_buffer, QWidget *, const char *)
	: YZView( _buffer, QYZisFactory::self(), 10 ), buffer( _buffer ), m_popup( 0 )

{
	m_editor = new QYZisEdit( this );
	status = new QStatusBar (this);
	command = new QYZisCommand (this);
	mVScroll = new QScrollBar( this);
	connect( mVScroll, SIGNAL(sliderMoved(int)), this, SLOT(scrollView(int)) );
	//connect( mVScroll, SIGNAL(prevLine()), this, SLOT(scrollLineUp()) );
	//connect( mVScroll, SIGNAL(nextLine()), this, SLOT(scrollLineDown()) );

	l_mode = new QLabel( _("QYzis Ready") );
	m_central = new QLabel();
	l_fileinfo = new QLabel();
	l_linestatus = new QLabel();

	status->addWidget(l_mode, 1); // was : status->insertItem(_("Yzis Ready"),0,1);
	//status->setItemAlignment(0,Qt::AlignLeft);

	status->addWidget(m_central,100);
//	status->insertItem("",80,80,0);
//	status->setItemAlignment(80,Qt::AlignLeft);

	status->addWidget(l_fileinfo, 1); // was  status->insertItem("",90,1);
//	status->setItemAlignment(90,Qt::AlignRight);

	status->addWidget(l_linestatus, 0); // was status->insertItem("",99,0,true);
//	status->setItemAlignment(99,Qt::AlignRight);

	g = new QGridLayout(this);
	g->addWidget(m_editor,0,0);
	g->addWidget(mVScroll,0,1);
	g->addWidget(command,1,0,1,2);
	g->addWidget(status,2,0,1,2);

//	setupActions();

	m_editor->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	m_editor->show();
	status->show();
	m_editor->setFocus();
	setFocusProxy( m_editor );
	myBuffer()->statusChanged();
	mVScroll->setMaximum( buffer->lineCount() - 1 );

//	setupCodeCompletion();

	setupKeys();

	QSettings settings;
	applyConfig( settings ); // XXX factory role
}

QYZisView::~QYZisView () {
//	yzDebug() << "QYZisView::~QYZisView" << endl;
//	if ( buffer ) buffer->removeView(this);
}

void QYZisView::setCommandLineText( const QString& text ) {
	command->setText( text );
}

QString QYZisView::getCommandLineText() const {
	return command->text();
}

void QYZisView::setFocusMainWindow() {
	m_editor->setFocus();
}

void QYZisView::setFocusCommandLine() {
	command->setFocus();
}

void QYZisView::scrollDown( int n ) {
	m_editor->scrollDown( n );
}

void QYZisView::scrollUp( int n ) {
	m_editor->scrollUp( n );
}

void QYZisView::refreshScreen() {
	if ( m_editor->marginLeft > 0 && !getLocalBooleanOption("number") )
		m_editor->marginLeft = 0;
	YZView::refreshScreen();
}
void QYZisView::preparePaintEvent( int min_y, int max_y ) {
	yzDebug() << "QYZisView::preparePaintEvent" << endl;
	m_painter = new QPainter( m_editor );
	m_drawBuffer.setCallbackArgument( m_painter );
	m_editor->drawMarginLeft( min_y, max_y, m_painter );
}
void QYZisView::endPaintEvent() {
	delete m_painter;
	yzDebug() << "QYZisView::endPaintEvent" << endl;
}
void QYZisView::paintEvent( const YZSelection& drawMap ) {
	if ( m_editor->m_insidePaintEvent ) {
		YZView::paintEvent( drawMap );
	} else {
		m_editor->paintEvent( drawMap );
	}
}
void QYZisView::drawCell( int x, int y, const YZDrawCell& cell, void* arg ) {
	m_editor->drawCell( x, y, cell, (QPainter*)arg );
}
void QYZisView::drawClearToEOL( int x, int y, const QChar& clearChar ) {
	m_editor->drawClearToEOL( x, y, clearChar, m_painter );
}
void QYZisView::drawSetMaxLineNumber( int max ) {
	mVScroll->setMaximum( max );
	m_editor->drawSetMaxLineNumber( max );
}
void QYZisView::drawSetLineNumber( int y, int n, int h ) {
	m_editor->drawSetLineNumber( y, n, h, m_painter );
}
int QYZisView::stringWidth( const QString& str ) const {
	return m_editor->fontMetrics().width( str );
}
int QYZisView::charWidth( const QChar& ch ) const {
	return m_editor->fontMetrics().width( ch );
}
QChar QYZisView::currentChar() const {
	return myBuffer()->textline( viewCursor().bufferY() ).at( viewCursor().bufferX() );
}

void QYZisView::wheelEvent( QWheelEvent * e ) {
	if ( e->orientation() == Qt::Vertical ) {
		int n = - ( e->delta() * mVScroll->singleStep() ) / 40; // WHEEL_DELTA(120) / 3 XXX
		scrollView( getCurrentTop() + n );
	} else {
		// TODO : scroll horizontally
	}
	e->accept();
}

void QYZisView::modeChanged (void) {
	m_editor->updateCursor();
	l_mode->setText( mode() );
}

void QYZisView::syncViewInfo() {
//	yzDebug() << "QYZisView::updateCursor" << viewInformation.c1 << " " << viewInformation.c2 << endl;
	m_editor->setCursor( viewCursor().screenX(), viewCursor().screenY() );
	l_linestatus->setText( getLineStatusString() );

	QString fileInfo;
	fileInfo +=( myBuffer()->fileIsNew() )?"N":" ";
	fileInfo +=( myBuffer()->fileIsModified() )?"M":" ";

	l_fileinfo->setText( fileInfo );
	if (mVScroll->value() != (int)getCurrentTop() && !mVScroll->isSliderDown())
		mVScroll->setValue( getCurrentTop() );
	emit cursorPositionChanged();
	modeChanged();
}

/*
void QYZisView::setupActions() {
	KStdAction::save(this, SLOT(fileSave()), actionCollection());
	KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
}
*/

void QYZisView::registerModifierKeys( const QString& keys ) {
	m_editor->registerModifierKeys( keys );
}
void QYZisView::unregisterModifierKeys( const QString& keys ) {
	m_editor->unregisterModifierKeys( keys );
}

void QYZisView::applyConfig( const QSettings& settings, bool refresh ) {

	m_editor->setFont( settings.value("appearance/font", QFont("Fixed")).value<QFont>() );
	QPalette default_palette;
	default_palette.setColor( QPalette::Window, QColor(0,0,0));
	default_palette.setColor( QPalette::WindowText, Qt::white );
	QPalette my_palette = settings.value("appearance/palette",default_palette).value<QPalette>();
	qreal opacity = settings.value("appearance/opacity",0.5).value<qreal>();
	m_editor->setPalette( my_palette, opacity );

	YzisHighlighting *yzis = myBuffer()->highlight();
	if (yzis) {
		myBuffer()->makeAttribs();
		repaint();
	} else if ( refresh ) {
		m_editor->updateArea( );
	}
}

void QYZisView::fileSave() {
	myBuffer()->save();
}

void QYZisView::fileSaveAs() {
	if ( popupFileSaveAs() )
		myBuffer()->save();
}

void QYZisView::filenameChanged() {
	if (Qyzis::me) {
		//Qyzis::me->setCaption(getId(), myBuffer()->fileName());
		Qyzis::me->setWindowTitle( myBuffer()->fileName());
	} else
		yzWarning() << "QYZisView::filenameChanged : couldn't find Qyzis::me.. is that ok ?";

}

void QYZisView::highlightingChanged() {
	sendRefreshEvent();
}

bool QYZisView::popupFileSaveAs() {
	QString url =	QFileDialog::getSaveFileName();
	if ( url.isEmpty() ) return false;//canceled

	if ( ! url.isEmpty() ) {
		myBuffer()->setPath( url );
		return true;
	}
	return false;
}



void QYZisView::displayInfo( const QString& info ) {
	m_central->setText(info);
	//clean the info 2 seconds later
	QTimer::singleShot(2000, this, SLOT( resetInfo() ) );
}

// scrolls the _view_ on a buffer and moves the cursor it scrolls off the screen
void QYZisView::scrollView( int value ) {
	if ( value < 0 ) value = 0;
	else if ( value > buffer->lineCount() - 1 )
		value = buffer->lineCount() - 1;

	// only redraw if the view actually moves
	if (value != getCurrentTop()) {
		alignViewBufferVertically( value );

		if (!mVScroll->isSliderDown())
			mVScroll->setValue( value );
	}
}




