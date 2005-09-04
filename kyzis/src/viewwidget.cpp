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
 *  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

/**
 * $Id$
 */
#include <qevent.h>
#include <kapplication.h>
#include <kstdaction.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
#include <viewcursor.h>
#include <qtimer.h>
#include <QMenu>

#include <kdebug.h>

#include "viewwidget.h"

#include "settings.h"
#include "yzis.h"
#include "mode_visual.h"
#include "factory.h"
#include "debug.h"
#include "kyziscodecompletion.h"
#include "kyzis.h"

KYZisView::KYZisView ( KYZTextEditorIface *doc, QWidget *parent, const char *)
	: KTextEditor::View (parent), YZView(doc->getBuffer(), KYZisFactory::self(), 10), m_popup(0)
{
	m_part = 0;
	buffer = doc;
	
	m_editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	command = new KYZisCommand (this, "command");
	mVScroll = new QScrollBar( this, "vscroll" );
	connect( mVScroll, SIGNAL(sliderMoved(int)), this, SLOT(scrollView(int)) );
	connect( mVScroll, SIGNAL(prevLine()), this, SLOT(scrollLineUp()) );
	connect( mVScroll, SIGNAL(nextLine()), this, SLOT(scrollLineDown()) );

	status->insertItem(_("Yzis Ready"),0,1);
	status->setItemAlignment(0,Qt::AlignLeft);

	m_central = new KSqueezedTextLabel(this);
	status->addWidget(m_central,100);
//	status->insertItem("",80,80,0);
//	status->setItemAlignment(80,Qt::AlignLeft);

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

	m_editor->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
	m_editor->show();
	status->show();
	m_editor->setFocus();
	setFocusProxy( m_editor );
	myBuffer()->statusChanged();
	mVScroll->setMaxValue( buffer->lines() - 1 );

	setupCodeCompletion();

	applyConfig();
	setupKeys();

}

KYZisView::~KYZisView () {
//	delete m_editor;
//	yzDebug() << "KYZisView::~KYZisView" << endl;
//	if ( buffer ) buffer->removeView(this);
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

void KYZisView::paintEvent( const YZSelection& drawMap ) {
	mVScroll->setMaxValue( buffer->lines() - 1 );
	m_editor->paintEvent( drawMap );
}
unsigned int KYZisView::stringWidth( const QString& str ) const {
	return m_editor->fontMetrics().width( str );
}
unsigned int KYZisView::charWidth( const QChar& ch ) const {
	return m_editor->fontMetrics().width( ch );
}
QChar KYZisView::currentChar() const {
	return myBuffer()->textline( viewCursor().bufferY() ).at( viewCursor().bufferX() );
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
	m_editor->setCursor( viewCursor().screenX(), viewCursor().screenY() );
	status->changeItem( getLineStatusString(), 99 );

	QString fileInfo;
	fileInfo +=( myBuffer()->fileIsNew() )?"N":" ";
	fileInfo +=( myBuffer()->fileIsModified() )?"M":" ";

	status->changeItem(fileInfo, 90);
	if (mVScroll->value() != (int)getCurrentTop() && !mVScroll->draggingSlider())
		mVScroll->setValue( getCurrentTop() );
	emit cursorPositionChanged();
	modeChanged();
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
	m_editor->setBackgroundMode( Qt::PaletteBase );
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
	myBuffer()->save();
}

void KYZisView::fileSaveAs() {
	if ( popupFileSaveAs() )
		myBuffer()->save();
}

/* Implementation of KTextEditor::ViewCursorInterface */
QPoint KYZisView::cursorPositionCoordinates() const {
	return m_editor->cursorCoordinates();
}

void KYZisView::cursorPosition ( int &line, int &col ) const {
	line = getBufferCursor().y();
	col  = getCursor().x();
}

void KYZisView::cursorPositionReal ( int &line, int &col ) const {
	line = getBufferCursor().y();
	col  = getBufferCursor().x();
}

bool KYZisView::setCursorPosition (const KTextEditor::Cursor &position) {
	gotodxy( position.column(), position.line() );
	return true;
}

void KYZisView::resetInfo() {
	m_central->setText("");
}

void KYZisView::displayInfo( const QString& info ) {
	m_central->setText(info);
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
	else if ( (unsigned int)value > buffer->lines() - 1 )
		value = buffer->lines() - 1;

	// only redraw if the view actually moves
	if ((unsigned int)value != getCurrentTop()) {
		alignViewBufferVertically( value );

		if (!mVScroll->draggingSlider())
			mVScroll->setValue( value );
	}
}

//KTextEditor::MenuInterface implementation
void KYZisView::setContextMenu( QMenu *rmb_Menu ) {
	m_popup = rmb_Menu;
}

QMenu *KYZisView::contextMenu () {
  return m_popup;
}

void KYZisView::contextMenuEvent( QContextMenuEvent * e ) {
	QMenu * popup = 0;
	if (m_popup)
		popup =  m_popup;
	else
		popup = dynamic_cast<QMenu*>( factory()->container("ktexteditor_popup", this ) );
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

#if 0
void KYZisView::showCompletionBox( Q3ValueList< KTextEditor::CompletionEntry > complList, int offset, bool casesensitive ) {
	m_codeCompletion->showCompletionBox(complList, offset, casesensitive);
}

void KYZisView::setupCodeCompletion() {
	m_codeCompletion = new KYZisCodeCompletion(this);
	connect( m_codeCompletion, SIGNAL(completionAborted()), this, SIGNAL(completionAborted()));
	connect( m_codeCompletion, SIGNAL(completionDone()), this, SIGNAL(completionDone()));
	connect( m_codeCompletion, SIGNAL(argHintHidden()), this, SIGNAL(argHintHidden()));
	connect( m_codeCompletion, SIGNAL(completionDone(KTextEditor::CompletionEntry)), this, SIGNAL(completionDone(KTextEditor::CompletionEntry)));
	connect( m_codeCompletion, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString*)), this, SIGNAL(filterInsertString(KTextEditor::CompletionEntry*,QString*)));
}
#endif

QFontMetrics KYZisView::editorFontMetrics( ) {
	return m_editor->fontMetrics();
}

/*
 * KTextEditor::SelectionInterface
 */
bool KYZisView::setSelection( const KTextEditor::Range& r ) {
	setPaintAutoCommit( false );
	if ( modePool()->current()->isSelMode() ) // leave it
		modePool()->pop();
	gotoxy( r.start().column(), r.start().line());
	modePool()->push( YZMode::MODE_VISUAL );
	gotoxy( r.end().column(), r.end().line() );
	commitPaintEvent();
	return true;
}
bool KYZisView::clearSelection() {
	YZASSERT_MSG( modePool()->current()->isSelMode(), "There is no selection" );
	modePool()->pop();
	return true;
}
bool KYZisView::selection() const {
	return !getSelectionPool()->visual()->isEmpty();
}

const KTextEditor::Range& KYZisView::selectionRange() const {
	YZInterval i = getSelectionPool()->visual()->bufferMap()[ 0 ];
	return KTextEditor::Range(i.fromPos().x(), i.fromPos().y(), i.toPos().x(), i.toPos().y());
}

QString KYZisView::selectionText() const {
	YZASSERT_MSG( modePool()->current()->isSelMode(), "There is no selection" );
	QList<QChar> regs;
	//that one is not const (due to YZCommandArgs constructor)
//	YZInterval i = dynamic_cast<YZModeVisual*>( modePool()->current() )->interval( YZCommandArgs(NULL,this,regs,1,false) );
	//so use the direct Visual mode case, assuming we only have selections in VISUAL mode anyway....
	YZInterval i = getSelectionPool()->visual()->bufferMap()[ 0 ];
	return myBuffer()->getText( i ).join("\n");
}
bool KYZisView::removeSelectedText() {
	YZASSERT_MSG( modePool()->current()->isSelMode(), "There is no selection" );
	dynamic_cast<YZModeVisual*>( modePool()->current() )->execCommand( this, "d" );
	return true;
}
bool KYZisView::selectAll() {
	if ( buffer->lines() == 0 )
		return true;
	return setSelection( KTextEditor::Range(KTextEditor::Cursor(0, 0),KTextEditor::Cursor(buffer->lines() - 1, qMax( (int)(myBuffer()->textline( buffer->lines() - 1 ).length() - 1), 0 ))));
}
bool KYZisView::popupFileSaveAs() {
	KURL url =	KFileDialog::getSaveURL();
	if ( url.isEmpty() ) return false;//canceled
	else if ( !url.isLocalFile() ) {
		KMessageBox::sorry(parentWidget(), tr("Yzis is not able to save remote files for now" ), tr( "Remote files"));
		return false;
	} else if ( ! url.isEmpty() ) {
		myBuffer()->setPath( url.path() );
		return true;
	}
	return false;
}

/*
 * KTextEditor::SelectionInterface
 */
int KYZisView::selectionStartLine() const {
	YZDoubleSelection* visual = getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].fromPos().y();
}
int KYZisView::selectionStartColumn() const {
	YZDoubleSelection* visual = getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].fromPos().x();
}
int KYZisView::selectionEndLine() const {
	YZDoubleSelection* visual = getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].toPos().y();
}
int KYZisView::selectionEndColumn() const {
	YZDoubleSelection* visual = getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].toPos().x();
}

const KTextEditor::Cursor& KYZisView::cursorPosition() const {

}

KTextEditor::Cursor KYZisView::cursorPositionVirtual() const {

}

bool KYZisView::removeSelection() {

}

bool KYZisView::removeSelectionText() {

}

const KTextEditor::Cursor& KYZisView::selectionStart() const {

}

const KTextEditor::Cursor& KYZisView::selectionEnd() const {

}

void KYZisView::filenameChanged() {
	if (Kyzis::me) {
		Kyzis::me->setCaption(getId(), myBuffer()->fileName());
	}
}

void KYZisView::highlightingChanged() {
	sendRefreshEvent();
}

#include "viewwidget.moc"
