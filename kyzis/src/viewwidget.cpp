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
 * $Id$
 */
#include <qevent.h>
#include <qpopupmenu.h>
#include <kapplication.h>
#include <kstdaction.h>
#include <kxmlguifactory.h>
#include <viewcursor.h>
#include "viewwidget.h"
#include "factory.h"
#include "debug.h"
#include "kyziscodecompletion.h"
#include <qtimer.h>

#include <kdebug.h>

#include "settings.h"

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name), YZView(doc, KYZisFactory::s_self, 10), m_popup(0)
{
	m_editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	command = new KYZisCommand (this, "command");
	mVScroll = new QScrollBar( this, "vscroll" );
	connect( mVScroll, SIGNAL(sliderMoved(int)), this, SLOT(scrollView(int)) );
	connect( mVScroll, SIGNAL(prevLine()), this, SLOT(scrollLineUp()) );
	connect( mVScroll, SIGNAL(nextLine()), this, SLOT(scrollLineDown()) );

	status->insertItem(_("Yzis Ready"),0,1);
	status->setItemAlignment(0,Qt::AlignLeft);

	status->insertItem("",80,80,0);
	status->setItemAlignment(80,Qt::AlignLeft);

	status->insertItem("",90,1);
	status->setItemAlignment(90,Qt::AlignRight);

	status->insertItem("",99,0,true);
	status->setItemAlignment(99,Qt::AlignRight);

	g = new QGridLayout(this,1,1);
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
	mVScroll->setMaxValue( buffer->lineCount() - 1 );

	setupCodeCompletion();

	applyConfig();
	setupKeys();

}

KYZisView::~KYZisView () {
	delete m_editor;
//	yzDebug() << "KYZisView::~KYZisView" << endl;
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
	mVScroll->setMaxValue( buffer->lineCount() - 1 );
	m_editor->paintEvent( curx, cury, curw, curh );
}
unsigned int KYZisView::stringWidth( const QString& str ) const {
	return m_editor->fontMetrics().width( str );
}
unsigned int KYZisView::charWidth( const QChar& ch ) const {
	return m_editor->fontMetrics().width( ch );
}
QChar KYZisView::currentChar() const {
	return mBuffer->textline( mainCursor->bufferY() ).at( mainCursor->bufferX() );
}

void KYZisView::wheelEvent( QWheelEvent * e ) {
	if ( e->orientation() == Qt::Vertical ) {
		int n = - ( e->delta() * mVScroll->lineStep() ) / 40; // WHEEL_DELTA(120) / 3 XXX
		scrollView( getCurrentTop() + n );
	} else {
		// TODO : scroll horizontally
	}
	e->accept();
}

void KYZisView::modeChanged (void) {
	m_editor->updateCursor();
	status->changeItem(mode(), 0);
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

	status->changeItem(fileInfo, 90);
	if (mVScroll->value() != (int)getCurrentTop() && !mVScroll->draggingSlider())
		mVScroll->setValue( getCurrentTop() );
	emit cursorPositionChanged();
	modeChanged();
}
void KYZisView::emitSelectionChanged() {
	buffer->emitSelectionChanged();
}

void KYZisView::refreshScreen () {
	mVScroll->setMaxValue( buffer->lineCount() -1 );
	abortPaintEvent();
	m_editor->repaint( false );
}

void KYZisView::setupActions() {
	KStdAction::save(this, SLOT(fileSave()), actionCollection());
	KStdAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
}

void KYZisView::registerModifierKeys( const QString& keys ) {
	m_editor->registerModifierKeys( keys );
}
void KYZisView::unregisterModifierKeys( const QString& keys ) {
	m_editor->unregisterModifierKeys( keys );
}

void KYZisView::applyConfig( bool refresh ) {
	m_editor->setFont( Settings::font() );
	m_editor->setBackgroundMode( PaletteBase );
	m_editor->setBackgroundColor( Settings::colorBG() );
	m_editor->setPaletteForegroundColor( Settings::colorFG() );
	m_editor->setTransparent( Settings::transparency(), (double)Settings::opacity() / 100., Settings::colorBG() );
	YzisHighlighting *yzis = myBuffer()->highlight();
	if (yzis) {
		myBuffer()->makeAttribs();
		repaint(true);
	}
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
	return m_editor->cursorCoordinates();
}

void KYZisView::cursorPosition ( unsigned int *line, unsigned int *col )
{
	*line = getBufferCursor()->y();
	*col  = getCursor()->x();
}

void KYZisView::cursorPositionReal ( unsigned int *line, unsigned int *col )
{
	*line = getBufferCursor()->y();
	*col  = getBufferCursor()->x();
}

bool KYZisView::setCursorPosition ( unsigned int line, unsigned int col)
{
	gotodxy( col, line );
	return true;
}

bool KYZisView::setCursorPositionReal ( unsigned int line, unsigned int col)
{
//	centerViewVertically(line); gotoxy auto-scroll the view
	gotoxy( col, line );
	return true;
}

unsigned int KYZisView::cursorLine()
{
	return getBufferCursor()->y();
}

unsigned int KYZisView::cursorColumn()
{
	return getCursor()->x();
}

unsigned int KYZisView::cursorColumnReal()
{
	return getBufferCursor()->x();
}

void KYZisView::resetInfo() {
	status->changeItem("", 80);
}

void KYZisView::displayInfo( const QString& info ) {
	status->changeItem(info, 80);
	//clean the info 2 seconds later
	QTimer::singleShot(2000, this, SLOT( resetInfo() ) );
}

void KYZisView::scrollLineUp() {
	scrollView( getCurrentTop() - 1 );
}

void KYZisView::scrollLineDown() {
	scrollView( getCurrentTop() + 1 );
}

// scrolls the _view_ on a buffer and moves the cursor it scrolls off the screen

void KYZisView::scrollView( int value ) {
	if ( value < 0 ) value = 0;
	else if ( (unsigned int)value > buffer->lineCount() - 1 )
		value = buffer->lineCount() - 1;

	// only redraw if the view actually moves
	if ((unsigned int)value != getCurrentTop()) {
		alignViewBufferVertically( value );

		if (!mVScroll->draggingSlider())
			mVScroll->setValue( value );
	}
}

//KTextEditor::PopupMenuInterface implementation

void KYZisView::installPopup( QPopupMenu *rmb_Menu ) {
	m_popup = rmb_Menu;
}

void KYZisView::contextMenuEvent( QContextMenuEvent * e ) {
	QPopupMenu * popup = 0;
	if (m_popup)
		popup =  m_popup;
	else
		popup = dynamic_cast<QPopupMenu*>( factory()->container("ktexteditor_popup", this ) );
	if (popup/* && popup->count() > 0*/)
	{
		e->accept();
		popup->exec(e->globalPos());
	}
}

void KYZisView::emitNewStatus() {
	emit newStatus();
}


//KTextEditor::CodeCompletionInterface and support functions

void KYZisView::showArgHint( QStringList functionList, const QString & strWrapping, const QString & strDelimiter ) {
	m_codeCompletion->showArgHint(functionList, strWrapping, strDelimiter);
}

void KYZisView::showCompletionBox( QValueList< KTextEditor::CompletionEntry > complList, int offset, bool casesensitive ) {
	m_codeCompletion->showCompletionBox(complList, offset, casesensitive);
}

void KYZisView::setupCodeCompletion() {
	m_codeCompletion = new KYZisCodeCompletion(this);
	connect( m_codeCompletion, SIGNAL(completionAborted()),
		this, SIGNAL(completionAborted()));
	connect( m_codeCompletion, SIGNAL(completionDone()),
		this, SIGNAL(completionDone()));
	connect( m_codeCompletion, SIGNAL(argHintHidden()),
		this, SIGNAL(argHintHidden()));
	connect( m_codeCompletion, SIGNAL(completionDone(KTextEditor::CompletionEntry)),
		this, SIGNAL(completionDone(KTextEditor::CompletionEntry)));
	connect( m_codeCompletion, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString*)),
		this,             SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString*)));
}

QFontMetrics KYZisView::editorFontMetrics( ) {
	return m_editor->fontMetrics();
}


#include "viewwidget.moc"
