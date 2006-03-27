/*
    Copyright (c) 2003-2005 Mickael Marchand <marchand@kde.org>
    Copyright (c) 2004 Alexander Dymo <adymo@mksat.net>
    Copyright (c) 2004 Scott Newton <scottn@ihug.co.nz>

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

/**
 * $Id$
 */

#include <klocale.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kstdaction.h>
#include <kaction.h>

#include "ktexteditoriface.h"
#include "viewwidget.h"
#include "factory.h"
#include "debug.h"
#include "mark.h"
#include "mode_visual.h"
#include "mode_ex.h"
#include "undo.h"
#include "buffer.h"
#include "kyzis.h"

//#include "configdialog.h"
//#include "hlconfig.h"

KYZTextEditorIface::KYZTextEditorIface(YZBuffer *buff, QWidget *parentWidget, const char *, QObject *parent, const char* /*name*/)
	: KTextEditor::Document(parent) {
		if (!buff) {
			buffer();
		} else {
			m_buffer = buff;
		}
		setInstance(KYZisFactory::self()->instance());
		m_parent = parentWidget;

		setupActions();
}

KYZTextEditorIface::~KYZTextEditorIface () {
}

KDocument::View *KYZTextEditorIface::createView ( QWidget *parent) {
	// if we're running as a KPart, remember the parent widget for this view
	if ( !Kyzis::me ) {
		KYZisFactory *factory = dynamic_cast<KYZisFactory*>(YZSession::me);
		factory->setViewParentWidget( parent );
	}
	
	KYZisView *kview = dynamic_cast<KYZisView*>(YZSession::me->createView( buffer() ));
	return kview;
}

void KYZTextEditorIface::setupActions() {
}

/*void KYZTextEditorIface::configureEditor() {

	if ( KConfigDialog::showDialog( "configure_editor" ) )
		return;

	KYZisConfigDialog *dialog = new KYZisConfigDialog( m_parent, "configure_editor", Settings::self(), KDialogBase::TreeList );
	connect( dialog, SIGNAL( settingsChanged() ), KYZisFactory::self(), SLOT( writeConfig() ) );
	dialog->show();
}*/

void KYZTextEditorIface::removeView( KTextEditor::View * v ) {
//	yzDebug() << "Document : removeView" << endl;
	if ( !v )
		return;

	KYZisView *kview = static_cast<KYZisView*>( v );
	YZSession::me->deleteView( kview->getId() );
}

YZBuffer *KYZTextEditorIface::buffer()
{
	if ( !m_buffer ) {
		m_buffer = YZSession::me->createBuffer();
	}
	
	return m_buffer;
}

bool KYZTextEditorIface::openFile () {
	yzDebug() << "openFile " << m_file << endl;
	buffer()->load(m_file);
	return true;
}

bool KYZTextEditorIface::saveFile () {
	return m_buffer->save();
}

/* Implementation of KTextEditor */

QString KYZTextEditorIface::text() const {
	return m_buffer->getWholeText();
}


bool KYZTextEditorIface::insertLine(unsigned int line, const QString &s) {
	m_buffer->insertLine(s, line);

	/* YZBuffer::insertLine() is a void function, apparently it can't fail.
	 * Return true
	 */

	return true;
}

bool KYZTextEditorIface::insertLines(int line, const QStringList &s) {
	for( int i = 0; i < s.size(); ++i ) {
		m_buffer->insertLine(s.at(i), line);
		++line;
	}

	/* YZBuffer::insertLine() is a void function, apparently it can't fail.
	 * Return true
	 */

	return true;
}

bool KYZTextEditorIface::removeLine(unsigned int line) {
	m_buffer->deleteLine(line);

	/* YZBuffer::deleteLine() also void function */

	return true;
}

QString KYZTextEditorIface::textLine(unsigned int line) const {
	//Quanta crashes when it asks for the last line, I am not sure whether our lineCount() is wrong
	//or if it's just quanta which does not count properly (it asks textLine from 0 to 218 in my test file,
	//whereas I said it the file have 218 lines)
	if ( line >= m_buffer->lineCount() )
		return QString::null;
	return m_buffer->textline(line);
}

int KYZTextEditorIface::length() const {
	uint textlength = m_buffer->getWholeTextLength();

	return textlength;
}

int KYZTextEditorIface::lineLength(int line) const {
	uint length = m_buffer->textline(line).length();

	return length;
}

bool KYZTextEditorIface::clear() {
	m_buffer->clearText();
	return true;
}

bool KYZTextEditorIface::insertText( const KTextEditor::Cursor &position, const QString &s, bool block) {
	//block ??????
	m_buffer->insertChar(position.column(), position.line(), s);

	return true;
}

bool KYZTextEditorIface::insertText( const KTextEditor::Cursor &position, const QStringList &s, bool block) {
	for( int i = 0; i < s.size(); ++ i) {
		//block ??????
		m_buffer->insertChar(position.column(), position.line(), s.at(i));
	}

	return true;
}

QString KYZTextEditorIface::text(const KTextEditor::Range& r, bool visual) const {
	if (visual) { //not sure that's what Kate people meant ...
		YZInterval i = m_buffer->firstView()->getSelectionPool()->visual()->bufferMap()[ 0 ];
		return m_buffer->getText(i).join("\n");
	}
	return m_buffer->getText( YZCursor(r.start().column(),r.start().line()), YZCursor(r.end().column(),r.end().line()) ).join("\n");
}

QStringList KYZTextEditorIface::textLines(const KTextEditor::Range& r, bool visual) const {
	if (visual) { //not sure that's what Kate people meant ...
		YZInterval i = m_buffer->firstView()->getSelectionPool()->visual()->bufferMap()[ 0 ];
		return m_buffer->getText(i);
	}
	
	return m_buffer->getText( YZCursor(r.start().column(),r.start().line()), YZCursor(r.end().column(),r.end().line()) );
}

bool KYZTextEditorIface::setText (  const QString &text ) {
	QString content = text;
	m_buffer->loadText( &content );
	return true;
}

bool KYZTextEditorIface::setText( const QStringList &text ) {
	for( int i = 0; i < text.size(); ++i ) {
		QString content = text.at(i);
		m_buffer->loadText( &content );
	}
	
	return true;
}

bool KYZTextEditorIface::removeText(const KTextEditor::Range& r, bool remove) {
	YZView *v = m_buffer->firstView();
	if (remove) { //even more suspicious to me ...
		KTextEditor::View* vi = dynamic_cast<KTextEditor::View*>(v);
		vi->removeSelectionText();
	}
	m_buffer->action()->deleteArea( v, YZCursor( r.start().column(), r.start().line()), YZCursor( r.end().column(), r.end().line()), QList<QChar>());
	return true;
}

unsigned int KYZTextEditorIface::hlMode () {
	return YzisHlManager::self()->findHl( m_buffer->highlight() );
}

bool KYZTextEditorIface::setHlMode (unsigned int mode) {
	m_buffer->setHighLight( mode );
	return true;
}

unsigned int KYZTextEditorIface::hlModeCount () {
	return YzisHlManager::self()->highlights();
}

QString KYZTextEditorIface::hlModeName (unsigned int mode) {
	return YzisHlManager::self()->hlName( mode );
}

QString KYZTextEditorIface::hlModeSectionName (unsigned int mode) {
	return YzisHlManager::self()->hlSection( mode );
}

void KYZTextEditorIface::highlightingChanged( ) {
	emit hlChanged();
}

void KYZTextEditorIface::undo() {
	YZView *v = m_buffer->firstView();
	m_buffer->undoBuffer()->undo(v);
}

void KYZTextEditorIface::redo() {
	YZView *v = m_buffer->firstView();
	m_buffer->undoBuffer()->redo(v);
}

void KYZTextEditorIface::clearUndo() {
	m_buffer->undoBuffer()->clearUndo();
}

void KYZTextEditorIface::clearRedo() {
	m_buffer->undoBuffer()->clearRedo();
}

unsigned int KYZTextEditorIface::undoCount() const {
	return m_buffer->undoBuffer()->undoCount();
}

unsigned int KYZTextEditorIface::redoCount() const {
	return m_buffer->undoBuffer()->redoCount();
}

unsigned int KYZTextEditorIface::undoSteps() const {
	return 0;
}

void KYZTextEditorIface::setUndoSteps(unsigned int ) {
	//nothing
}

void KYZTextEditorIface::setModified( bool modified ) {
	if ( KTextEditor::Document::isModified() != modified ) {
		KTextEditor::Document::setModified( modified );
		YZList<YZView*> views = m_buffer->views();
		for ( YZList<YZView*>::const_iterator it = views.begin(); it != views.end(); ++it ) { 
			KYZisView *kv = dynamic_cast<KYZisView *>(*it);
			if (kv)
				kv->emitNewStatus();
		}
	}
	if ( modified )
		emit textChanged();
}

//KTextEditor::MarkInterface slots
uint KYZTextEditorIface::mark( int line ) {
	return m_buffer->docMarks()->get(line);
}

void KYZTextEditorIface::setMark( int line, uint markType ) {
	m_buffer->docMarks()->add(line, markType);
}

void KYZTextEditorIface::clearMark( int line ) {
	m_buffer->docMarks()->del(line);
}

void KYZTextEditorIface::addMark( int line, uint markType ) {
	m_buffer->docMarks()->add(line, markType);
}

void KYZTextEditorIface::removeMark( int line, uint markType ) {
	m_buffer->docMarks()->del(line, markType);
}

QHash<int, KTextEditor::Mark*> & KYZTextEditorIface::marks() {
	qDeleteAll(m_marks);
	const YZDocMarker &marker = m_buffer->docMarks()->getMarker();
	for (YZDocMarker::const_iterator it = marker.constBegin(); it != marker.constEnd(); ++it)
	{
		KTextEditor::Mark *m = new KTextEditor::Mark;
		m->line = it.key();
		m->type = it.data();
		m_marks.insert(m->line, m);
	}
	return m_marks;
}

void KYZTextEditorIface::clearMarks() {
	m_buffer->docMarks()->clear();
}


void KYZTextEditorIface::setMarkPixmap(KTextEditor::MarkInterface::MarkTypes /*type*/, const QPixmap &/*pix*/) {
} //TODO
void KYZTextEditorIface::setMarkDescription(KTextEditor::MarkInterface::MarkTypes /*type*/, const QString &/*d*/) {
} //TODO
void KYZTextEditorIface::markChanged ( KTextEditor::Document* /*document*/, KTextEditor::Mark /*mark*/, KTextEditor::MarkInterface::MarkChangeAction /*action*/ ) {
} //TODO
void KYZTextEditorIface::marksChanged (KTextEditor::Document* /*document*/ ) {
} //TODO

QString KYZTextEditorIface::mimeType() {
	KMimeType::Ptr result = KMimeType::defaultMimeTypePtr();
	  
	// if the document has a URL, try KMimeType::findByURL
	if ( ! m_url.isEmpty() )
		result = KMimeType::findByURL( m_url );
	else if ( m_url.isEmpty() || ! m_url.isLocalFile() )
		result = mimeTypeForContent();
	                
	return result->name();
}

KMimeType::Ptr KYZTextEditorIface::mimeTypeForContent() {
	QByteArray buf (1024);
	uint bufpos = 0;
    
	for (int i=0; i < lines(); ++i)
	{
		QString line = this->line( i );
		uint len = line.length() + 1;
             
		if (bufpos + len > 1024)
		len = 1024 - bufpos;
                        
		QString ld (line + QChar::fromAscii('\n'));
		memcpy(buf.data() + bufpos, ld.latin1(), len);
                                
		bufpos += len;
                                  
		if (bufpos >= 1024)
			break;
	}
	buf.resize( bufpos );
		                                                    
	int accuracy = 0;
	return KMimeType::findByContent( buf, &accuracy );
}

bool KYZTextEditorIface::setEncoding( const QString&enc ) { 
	//use YZBuffer::setEncoding when it actually does something ?
	YZExCommandArgs ex ( YZSession::me->currentView(), QString::null, "set", "encoding=" + enc, 0, 0, true );
	YZSession::me->getExPool()->set( ex );
	return true; 
}
	
bool KYZTextEditorIface::documentReload() {
	return false; 
}

bool KYZTextEditorIface::documentSave() {
	return saveFile();
}

QString KYZTextEditorIface::text( const KTextEditor::Cursor& start, const KTextEditor::Cursor& end ) const {
	QStringList l = m_buffer->getText( YZCursor( start.column(), start.line() ), YZCursor( end.column(), end.line() ) );
	return l.join("\n");
}

QString KYZTextEditorIface::line( int i ) const {
	return m_buffer->textline( i );
}

bool KYZTextEditorIface::removeText( const KTextEditor::Cursor& start, const KTextEditor::Cursor& end ) {
	m_buffer->action()->deleteArea( YZSession::me->currentView(), 
			YZCursor( start.column(), start.line() ), 
			YZCursor( end.column(), end.line() ), QList<QChar>() );
	return true;
}

bool KYZTextEditorIface::insertLine( int i, const QString& line ) {
	m_buffer->action()->insertLine( YZSession::me->currentView(), YZCursor( 0, i ), line );
	return true;
}

bool KYZTextEditorIface::removeLine( int i ) {
	m_buffer->action()->deleteLine( YZSession::me->currentView(), YZCursor( 0, i ), 1, QList<QChar>() );
	return true; //it can't fail for now
}

bool KYZTextEditorIface::documentSaveAs() {
	YZView *v = m_buffer->firstView();
	if (v)
		return v->popupFileSaveAs();
	return false;
}

#include "ktexteditoriface.moc"
