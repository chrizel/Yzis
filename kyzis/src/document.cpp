/*
    Copyright (c) 2003-2005 Mickael Marchand <marchand@kde.org>
    Copyright (c) 2004 Alexander Dymo <adymo@mksat.net>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/**
 * $Id$
 */

#include <klocale.h>
#include <dcopclient.h>
#include <qcstring.h>
#include <kapplication.h>
#include <kstdaction.h>
#include <kaction.h>

#include "document.h"
#include "viewwidget.h"
#include "factory.h"
#include "debug.h"
#include "undo.h"
#include "mark.h"
#include "mode_visual.h"
#include "buffer.h"

#include "configdialog.h"
#include "hlconfig.h"

KYZTextEditorIface::KYZTextEditorIface(YZBuffer *buffer, QWidget *parentWidget, const char *, QObject *parent, const char *name)
	: KTextEditor::Document(parent,name) {
		m_buffer = buffer;
		setInstance(KYZisFactory::self()->instance());
		m_parent = parentWidget;

		setupActions();
}

KYZTextEditorIface::~KYZTextEditorIface () {
}


KTextEditor::View *KYZTextEditorIface::createView ( QWidget *, const char * ) {
	KYZisView *kview = dynamic_cast<KYZisView*>(YZSession::me->createView( m_buffer ));
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

bool KYZTextEditorIface::openFile () {
	yzDebug() << "openFile " << m_file << endl;
	m_buffer->load(m_file);
	return true;
}

bool KYZTextEditorIface::saveFile () {
	return m_buffer->save();
}

/* Implementation of KTextEditor */

QString KYZTextEditorIface::text() const
{
	return m_buffer->getWholeText();
}


uint KYZTextEditorIface::numLines() const
{
	return m_buffer->lineCount();
}

bool KYZTextEditorIface::insertLine(unsigned int line, const QString &s)
{
	m_buffer->insertLine(s, line);

	/* YZBuffer::insertLine() is a void function, apparently it can't fail.
	 * Return true
	 */

	return true;
}

bool KYZTextEditorIface::removeLine(unsigned int line)
{
	m_buffer->deleteLine(line);

	/* YZBuffer::deleteLine() also void function */

	return true;
}

QString KYZTextEditorIface::textLine(unsigned int line) const
{
	//Quanta crashes when it asks for the last line, I am not sure whether our lineCount() is wrong
	//or if it's just quanta which does not count properly (it asks textLine from 0 to 218 in my test file,
	//whereas I said it the file have 218 lines)
	if ( line >= m_buffer->lineCount() ) return QString::null;
	return m_buffer->textline(line);
}

uint KYZTextEditorIface::length() const
{
	uint textlength = m_buffer->getWholeTextLength();

	return textlength;
}

int KYZTextEditorIface::lineLength(unsigned int line) const
{
	uint length = m_buffer->textline(line).length();

	return length;
}

bool KYZTextEditorIface::clear()
{
	m_buffer->clearText();
	return true;
}

bool KYZTextEditorIface::insertText( uint line, uint col, const QString &s)
{
	m_buffer->insertChar(col, line, s);

	return true;
}

QString KYZTextEditorIface::text (  uint startLine, uint startCol, uint endLine, uint endCol ) const {
	return m_buffer->getText( YZCursor(startCol,startLine), YZCursor(endCol,endLine) ).join("\n");
}

bool KYZTextEditorIface::setText (  const QString &text ) {
	QString content = text;
	m_buffer->loadText( &content );
	return true;
}

bool KYZTextEditorIface::removeText (  uint startLine, uint startCol, uint endLine, uint endCol) {
	YZView *v = m_buffer->firstView();
	m_buffer->action()->deleteArea( v, YZCursor( startCol, startLine ), YZCursor( endCol, endLine ), QValueList<QChar>());
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

//KTextEditor::ConfigInterface implementation

void KYZTextEditorIface::readConfig() {
	KYZisFactory::self()->readConfig();
}

void KYZTextEditorIface::writeConfig() {
	KYZisFactory::self()->writeConfig();
}

void KYZTextEditorIface::readConfig( KConfig */*config*/ ) {
	//how to use another config file with kconfigxt?
}

void KYZTextEditorIface::writeConfig( KConfig */*config*/ ) {
	//how to use another config file with kconfigxt?
}

void KYZTextEditorIface::readSessionConfig( KConfig *config ) {
	KURL url(config->readEntry("URL"));
	if (!url.isEmpty() && url.isValid())
		openURL(url);
}

void KYZTextEditorIface::writeSessionConfig( KConfig *config ) {
	config->writeEntry("URL", url().prettyURL());
}

void KYZTextEditorIface::configDialog() {
	if ( KConfigDialog::showDialog( "configure_editor" ) )
		return;

	KYZisConfigDialog *dialog = new KYZisConfigDialog( m_parent, "configure_editor", Settings::self(), KDialogBase::TreeList );
	connect( dialog, SIGNAL( settingsChanged() ), KYZisFactory::self(), SLOT( applyConfig() ) );
	dialog->exec();
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
void KYZTextEditorIface::emitSelectionChanged() {
	emit selectionChanged();
}


//KTextEditor::MarkInterface slots

uint KYZTextEditorIface::mark( uint line ) {
	return m_buffer->docMarks()->get(line);
}

void KYZTextEditorIface::setMark( uint line, uint markType ) {
	m_buffer->docMarks()->add(line, markType);
}

void KYZTextEditorIface::clearMark( uint line ) {
	m_buffer->docMarks()->del(line);
}

void KYZTextEditorIface::addMark( uint line, uint markType ) {
	m_buffer->docMarks()->add(line, markType);
}

void KYZTextEditorIface::removeMark( uint line, uint markType ) {
	m_buffer->docMarks()->del(line, markType);
}

QPtrList<KTextEditor::Mark> KYZTextEditorIface::marks() {
	QPtrList<KTextEditor::Mark> marks;
	const YZDocMarker &marker = m_buffer->docMarks()->getMarker();
	for (YZDocMarker::const_iterator it = marker.constBegin(); it != marker.constEnd(); ++it)
	{
		KTextEditor::Mark *m = new KTextEditor::Mark;
		m->line = it.key();
		m->type = it.data();
		marks.append(m);
	}
	return marks;
}

void KYZTextEditorIface::clearMarks() {
	m_buffer->docMarks()->clear();
}

/*
 * KTextEditor::MarkInterfaceExtension
 */
void KYZTextEditorIface::setPixmap(MarkInterface::MarkTypes, const QPixmap &) {
}
void KYZTextEditorIface::setDescription(MarkInterface::MarkTypes, const QString &){
}
void KYZTextEditorIface::setMarksUserChangable(uint ) {
}
void KYZTextEditorIface::markChanged (KTextEditor::Mark , KTextEditor::MarkInterfaceExtension::MarkChangeAction ) {
}


uint KYZTextEditorIface::configPages() const {
	return 1;	
}

KTextEditor::ConfigPage *KYZTextEditorIface::configPage ( uint /*number*/, QWidget *parent, const char */*name*/ ) {
	return new YzisSchemaConfigPage( parent, this );
}

QString KYZTextEditorIface::configPageName ( uint /*number*/ ) const {
	return i18n("Syntax Highlighting");
}

QString KYZTextEditorIface::configPageFullName ( uint /*number*/) const {
	return i18n("Syntax Highlighting");
}

QPixmap KYZTextEditorIface::configPagePixmap ( uint /*number*/, int /*size*/ ) const {
	return 0;
}

/*
 * KTextEditor::SelectionInterface
 */
bool KYZTextEditorIface::setSelection( unsigned int startLine, unsigned int startCol, unsigned int endLine, unsigned int endCol ) {
	YZView *v = m_buffer->firstView();
	v->setPaintAutoCommit( false );
	if ( v->modePool()->current()->isSelMode() ) // leave it
		v->modePool()->pop();
	v->gotoxy( startCol, startLine );
	v->modePool()->push( YZMode::MODE_VISUAL );
	v->gotoxy( endCol, endLine );
	v->commitPaintEvent();
	return true;
}
bool KYZTextEditorIface::clearSelection() {
	YZView *v = m_buffer->firstView();
	YZASSERT_MSG( v->modePool()->current()->isSelMode(), "There is no selection" );
	v->modePool()->pop();
	return true;
}
bool KYZTextEditorIface::hasSelection() const {
	YZView *v = m_buffer->firstView();
	return !(v->getSelectionPool()->visual()->isEmpty());
}
QString KYZTextEditorIface::selection() const {
	YZView *v = m_buffer->firstView();
	YZASSERT_MSG( v->modePool()->current()->isSelMode(), "There is no selection" );
	QValueList<QChar> regs;
	YZInterval i = dynamic_cast<YZModeVisual*>( v->modePool()->current() )->interval( YZCommandArgs(NULL,v,regs,1,false) );
	return ((YZBuffer*)this)->getText( i ).join("\n");
}
bool KYZTextEditorIface::removeSelectedText() {
	YZView *v = m_buffer->firstView();
	YZASSERT_MSG( v->modePool()->current()->isSelMode(), "There is no selection" );
	dynamic_cast<YZModeVisual*>( v->modePool()->current() )->execCommand( v, "d" );
	return true;
}
bool KYZTextEditorIface::selectAll() {
	if ( m_buffer->lineCount() == 0 )
		return true;
	return setSelection( 0, 0, m_buffer->lineCount() - 1, qMax( (int)(m_buffer->textline( m_buffer->lineCount() - 1 ).length() - 1), 0 ) );
}

/*
 * KTextEditor::SelectionInterfaceSel
 */
int KYZTextEditorIface::selStartLine() {
	YZView *v = m_buffer->firstView();
	YZDoubleSelection* visual = v->getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].fromPos().y();
}
int KYZTextEditorIface::selStartCol() {
	YZView *v = m_buffer->firstView();
	YZDoubleSelection* visual = v->getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].fromPos().x();
}
int KYZTextEditorIface::selEndLine() {
	YZView *v = m_buffer->firstView();
	YZDoubleSelection* visual = v->getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].toPos().y();
}
int KYZTextEditorIface::selEndCol() {
	YZView *v = m_buffer->firstView();
	YZDoubleSelection* visual = v->getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].toPos().x();
}

QPtrList<KTextEditor::View> KYZTextEditorIface::views() const
{
	YZList<YZView*> views = m_buffer->views();
	QPtrList<KTextEditor::View> result;
	
	for ( YZList<YZView*>::Iterator itr = views.begin(); itr != views.end(); ++itr ) {
		KYZisView *kview = static_cast<KYZisView*>(*itr);
		result.append( kview );
	}
	
	return result;
}

#include "document.moc"
