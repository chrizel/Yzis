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
#include "qsession.h"
#include "debug.h"
#include "qyzis.h"
#include "linenumbers.h"

QYZisView::QYZisView ( YZBuffer *_buffer, QWidget *, const char *)
	: YZView( _buffer, QYZisSession::self(), 10 ), buffer( _buffer ), m_popup( 0 )

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

	m_lineNumbers = new QYZisLineNumbers(this);

	QHBoxLayout* editorLayout = new QHBoxLayout();
	editorLayout->setMargin(0);
	editorLayout->setSpacing(0);
	editorLayout->addWidget( m_lineNumbers );
	editorLayout->addWidget( m_editor );
	editorLayout->addWidget( mVScroll );

	QVBoxLayout* viewLayout = new QVBoxLayout( this );
	viewLayout->setMargin(0);
	viewLayout->setSpacing(0);
	viewLayout->addLayout( editorLayout );
	viewLayout->addWidget( command );
	viewLayout->addWidget( status );

//	setupActions();
	setupKeys();

	m_editor->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

	QSettings settings;
	applyConfig( settings ); // XXX factory role

	m_editor->show();
	status->show();
	m_editor->setFocus();
	setFocusProxy( m_editor );
	myBuffer()->statusChanged();
	mVScroll->setMaximum( buffer->lineCount() - 1 );

//	setupCodeCompletion();
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

void QYZisView::Scroll( int dx, int dy ) {
	m_editor->scroll( dx, dy );
	m_lineNumbers->scroll( dy );
}

void QYZisView::setVisibleArea( int columns, int lines ) {
	m_lineNumbers->setLineCount( lines );
	YZView::setVisibleArea( columns, lines );
}

void QYZisView::refreshScreen() {
	bool o_number = getLocalBooleanOption("number");
	if ( o_number != m_lineNumbers->isVisible() ) {
		m_lineNumbers->setVisible(o_number);
	}
	YZView::refreshScreen();
}

void QYZisView::notifyContentChanged( const YZSelection& s ) {
	// content has changed, ask qt to repaint changed parts

	YZSelectionMap m = s.map();
	// convert each interval to QWidget coordinates and update
	for( int i = 0; i < m.size(); ++i ) {
		YZInterval interval = m[i] - getScreenPosition();
		QRect r;
		if ( interval.fromPos().y() == interval.toPos().y() ) {
			r = interval.boundingRect();
			r.setBottom( r.bottom() + 1 );
			r.setRight( r.right() + 1 );
		} else {
			// XXX optimise : split into multiple qrect
			r.setTop( interval.fromPos().y() );
			r.setBottom( interval.toPos().y() + 1 );
			r.setLeft( 0 );
			r.setRight( getColumnsVisible() );
		}
//		yzDebug() << "notifiyContentChanged: interval=" << interval.fromPos() << "," << interval.toPos() 
//					<< ", r=" << r.topLeft() << "," << r.bottomRight();
		r.setBottomRight( m_editor->translatePositionToReal( r.bottomRight() ) );
		r.setTopLeft( m_editor->translatePositionToReal( r.topLeft() ) );
//		yzDebug() << " => " << r.topLeft() << "," << r.bottomRight() << endl;
		m_editor->update( r );
	}
}

void QYZisView::preparePaintEvent( int min_y, int max_y ) {
//	yzDebug() << "QYZisView::preparePaintEvent" << endl;
	m_painter = new QPainter( m_editor );
	m_drawBuffer.setCallbackArgument( m_painter );
	//m_editor->drawMarginLeft( min_y, max_y, m_painter );
}
void QYZisView::endPaintEvent() {
	delete m_painter;
//	yzDebug() << "QYZisView::endPaintEvent" << endl;
}

void QYZisView::paintEvent( const YZSelection& s ) {
	YZView::paintEvent( s );
}

/*
 * View painting methods
 */
void QYZisView::drawCell( int x, int y, const YZDrawCell& cell, void* arg ) {
	m_editor->drawCell( x, y, cell, (QPainter*)arg );
}
void QYZisView::drawClearToEOL( int x, int y, const QChar& clearChar ) {
	m_editor->drawClearToEOL( x, y, clearChar, m_painter );
}
void QYZisView::drawSetMaxLineNumber( int max ) {
	mVScroll->setMaximum( max );
	m_lineNumbers->setMaxLineNumber( max );
}
void QYZisView::drawSetLineNumber( int y, int n, int h ) {
	m_lineNumbers->setLineNumber( y, h, n );
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

	QFont default_font;
	default_font.setStyleHint(QFont::TypeWriter);
	default_font.setFamily("Courier");
	QFont user_font = settings.value("appearance/font", default_font).value<QFont>();
	m_editor->setFont( user_font );
	m_lineNumbers->setFont( user_font );

	QPalette default_palette;
	default_palette.setColor( QPalette::Window, Qt::black );
	default_palette.setColor( QPalette::WindowText, Qt::white );
	QPalette my_palette = settings.value("appearance/palette",default_palette).value<QPalette>();
	qreal opacity = settings.value("appearance/opacity",1.).value<qreal>();
	m_editor->setPalette( my_palette, opacity );

	if ( refresh ) {
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




