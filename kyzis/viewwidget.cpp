/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2004 Mickael Marchand <marchand@kde.org>
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
 * $Id$
 */
#include <qlayout.h>
#include <qevent.h>
#include <kapplication.h>
#include <kstdaction.h>
#include <viewcursor.h>
#include "viewwidget.h"
#include "factory.h"
#include "debug.h"
#include <qtimer.h>

#include "settings.h"

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name), YZView(doc, KYZisFactory::s_self, 10)
{
	m_editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	command = new KYZisCommand (this, "command");
	mVScroll = new QScrollBar( this, "vscroll" );
	connect( mVScroll, SIGNAL(valueChanged(int)), this, SLOT(scrolled(int)) );

	status->insertItem(mode(YZ_VIEW_MODE_LAST),0,1);
	status->setItemAlignment(0,Qt::AlignLeft);

	status->insertItem("",80,80,0);
	status->setItemAlignment(0,Qt::AlignLeft);

	status->insertItem("",90,1);
	status->setItemAlignment(0,Qt::AlignRight);

	status->insertItem("",99,0,true);
	status->setItemAlignment(99,Qt::AlignRight);

	QGridLayout *g = new QGridLayout(this,1,1);
	g->addWidget(m_editor,0,0);
	g->addWidget(mVScroll,0,1);
	g->addMultiCellWidget(command,1,1,0,1);
	g->addMultiCellWidget(status,2,2,0,1);
	
	setXMLFile( "kyzispart/kyzispart.rc" );
	setupActions();
	
	buffer = doc;
	m_editor->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	m_editor->show();
	status->show();
	m_editor->setFocus();
	setFocusProxy( m_editor );
	mBuffer->statusChanged();
	mVScroll->setMaxValue( buffer->lineCount() );

	applyConfig();
}

KYZisView::~KYZisView () {
	yzDebug() << "KYZisView::~KYZisView" << endl;
	if ( buffer ) buffer->removeView(this);
}

void KYZisView::setCommandLineText( const QString& text ) {
	command->setText( text );
}

QString KYZisView::getCommandLineText() const {
	return command->text();
}

void KYZisView::setFocusMainWindow() {
	m_editor->setFocus();
}

void KYZisView::setFocusCommandLine() {
	command->setFocus();
}

void KYZisView::scrollDown( int n ) {
	m_editor->scrollDown( n );
}

void KYZisView::scrollUp( int n ) {
	m_editor->scrollUp( n );
}

void KYZisView::paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh ) {
	mVScroll->setMaxValue( buffer->lineCount() );
	m_editor->paintEvent( curx, cury, curw, curh );
}
unsigned int KYZisView::stringWidth( const QString& str ) const {
	return m_editor->fontMetrics().width( str );
}
unsigned int KYZisView::charWidth( const QChar& ch ) const {
	return m_editor->fontMetrics().width( ch );
}

void KYZisView::wheelEvent( QWheelEvent * e ) {
	int n = ( e->delta() * mVScroll->lineStep() ) / 40; // WHEEL_DELTA(120) / 3 XXX 
	mVScroll->setValue( mVScroll->value() - n  );
	scrolled( mVScroll->value() );
}

void KYZisView::modeChanged (void) {
	yzDebug() << "switching to mode: " << mMode << "; old mode is: " <<
		mPrevMode << endl;
	if (mBuffer->introShown() )  {
			status->changeItem(mode(YZ_VIEW_MODE_LAST), 0);
			return;
	}
	switch (mMode) {
		case YZ_VIEW_MODE_INSERT: // insert
			status->changeItem(mode( YZ_VIEW_MODE_INSERT ), 0);
			break;
		case YZ_VIEW_MODE_REPLACE: // replace
			status->changeItem(mode( YZ_VIEW_MODE_REPLACE ), 0);
			break;
		case YZ_VIEW_MODE_COMMAND: // normal
			status->changeItem(mode( YZ_VIEW_MODE_COMMAND), 0);
			break;
		case YZ_VIEW_MODE_EX: //script
			status->changeItem(mode(YZ_VIEW_MODE_EX), 0);
			break;
		case YZ_VIEW_MODE_SEARCH: //search mode
			status->changeItem( mode( YZ_VIEW_MODE_SEARCH ), 0);
			break;
		case YZ_VIEW_MODE_OPEN: //open
			status->changeItem(mode(YZ_VIEW_MODE_OPEN), 0);
			break;
		case YZ_VIEW_MODE_VISUAL: //visual
			status->changeItem(mode(YZ_VIEW_MODE_VISUAL), 0);
			break;
		case YZ_VIEW_MODE_VISUAL_LINE : 
			status->changeItem( mode(YZ_VIEW_MODE_VISUAL_LINE), 0 );
	};
}

void KYZisView::syncViewInfo() {
//	yzDebug() << "KYZisView::updateCursor" << viewInformation.c1 << " " << viewInformation.c2 << endl;
	m_editor->setCursor( mainCursor->screenX(), mainCursor->screenY() );
	if (viewInformation.c1!=viewInformation.c2)
		status->changeItem( QString("%1,%2-%3 (%4)").arg(viewInformation.l+1 ).arg( viewInformation.c1+1 ).arg( viewInformation.c2+1 ).arg( viewInformation.percentage),99 );
	else
		status->changeItem( QString("%1,%2 (%3)").arg(viewInformation.l+1 ).arg( viewInformation.c1+1 ).arg( viewInformation.percentage),99 );

	QString fileInfo;
	fileInfo +=( mBuffer->fileIsNew() )?"N":" ";
	fileInfo +=( mBuffer->fileIsModified() )?"M":" ";
	buffer->setModified( mBuffer->fileIsModified() );

	status->changeItem(fileInfo, 90);
	mVScroll->setValue(getBufferCursor()->getY() );
	modeChanged();
}

void KYZisView::refreshScreen () {
	mVScroll->setMaxValue( buffer->lineCount() );
	m_editor->repaint( false );
}

void KYZisView::setupActions() {
	KStdAction::save(this, SLOT(fileSave()), actionCollection());
	KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
}


void KYZisView::applyConfig( bool refresh ) {
	m_editor->setFont( Settings::font() );
	m_editor->setBackgroundMode( PaletteBase );
	m_editor->setBackgroundColor( Settings::colorBG() );
	m_editor->setPaletteForegroundColor( Settings::colorFG() );
	m_editor->setTransparent( Settings::transparency() );
	if ( refresh ) {
		m_editor->updateArea( );
	}
}

void KYZisView::fileSave() {
	mBuffer->save();
}

void KYZisView::fileSaveAs() {
	if ( mBuffer->popupFileSaveAs() )
		mBuffer->save();
}

/* Implementation of KTextEditor::ViewCursorInterface */

QPoint KYZisView::cursorCoordinates()
{
	unsigned int line = 0, col = 0;
	line = getCursor()->getY();
	col  = getCursor()->getX();

	QPoint cursorPosition( col * fontMetrics().maxWidth(), line * fontMetrics().lineSpacing() );
	
	return cursorPosition;
}

void KYZisView::cursorPosition ( unsigned int *line, unsigned int *col )
{
	*line = getCursor()->getY();
	*col  = getCursor()->getX();
}

void KYZisView::cursorPositionReal ( unsigned int *line, unsigned int *col )
{
	*line = getBufferCursor()->getY();
	*col  = getBufferCursor()->getX();
} 

bool KYZisView::setCursorPosition ( unsigned int line, unsigned int col)
{
	gotodxdy( line, col );
	return true;
}

bool KYZisView::setCursorPositionReal ( unsigned int line, unsigned int col)
{
	gotoxy( col, line );
	return true;
}

unsigned int KYZisView::cursorLine()
{
	return getCursor()->getY();
}

unsigned int KYZisView::cursorColumn()
{
	return getCursor()->getX();
}

unsigned int KYZisView::cursorColumnReal()
{
	return getBufferCursor()->getX();
}

void KYZisView::cursorPositionChanged()
{
}

void KYZisView::resetInfo() {
	status->changeItem("", 80);
}

void KYZisView::displayInfo( const QString& info ) {
	status->changeItem(info, 80);
	//clean the info 2 seconds later
	QTimer::singleShot(2000, this, SLOT( resetInfo() ) );
}

void KYZisView::scrolled( int value ) {
//	yzDebug() << "Scrolled to " << value << endl;
	mVScroll->setMaxValue( buffer->lineCount() );
	gotoxy(getBufferCursor()->getX(), value);
}

#include "viewwidget.moc"