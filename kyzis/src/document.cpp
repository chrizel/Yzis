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

KYZisDoc::KYZisDoc(YZBuffer *buffer, QWidget *parentWidget, const char *, QObject *parent, const char *name)
	: KTextEditor::Document(parent,name) {
		m_buffer = buffer;
		setInstance(KYZisFactory::self()->instance());
		m_parent = parentWidget;

		setupActions();
}

KYZisDoc::~KYZisDoc () {
}


KTextEditor::View *KYZisDoc::createView ( QWidget *, const char * ) {
	KYZisView *kview = dynamic_cast<KYZisView*>(YZSession::me->createView( m_buffer ));
	return kview;
}

void KYZisDoc::setupActions() {
}

/*void KYZisDoc::configureEditor() {

	if ( KConfigDialog::showDialog( "configure_editor" ) )
		return;

	KYZisConfigDialog *dialog = new KYZisConfigDialog( m_parent, "configure_editor", Settings::self(), KDialogBase::TreeList );
	connect( dialog, SIGNAL( settingsChanged() ), KYZisFactory::self(), SLOT( writeConfig() ) );
	dialog->show();
}*/

void KYZisDoc::removeView( KTextEditor::View * v ) {
//	yzDebug() << "Document : removeView" << endl;
	if ( !v )
		return;

	KYZisView *kview = static_cast<KYZisView*>( v );
	YZSession::me->deleteView( kview->getId() );
}

bool KYZisDoc::openFile () {
	yzDebug() << "openFile " << m_file << endl;
	m_buffer->load(m_file);
	return true;
}

bool KYZisDoc::saveFile () {
	return m_buffer->save();
}

/* Implementation of KTextEditor */

QString KYZisDoc::text() const
{
	return m_buffer->getWholeText();
}


uint KYZisDoc::numLines() const
{
	return m_buffer->lineCount();
}

bool KYZisDoc::insertLine(unsigned int line, const QString &s)
{
	m_buffer->insertLine(s, line);

	/* YZBuffer::insertLine() is a void function, apparently it can't fail.
	 * Return true
	 */

	return true;
}

bool KYZisDoc::removeLine(unsigned int line)
{
	m_buffer->deleteLine(line);

	/* YZBuffer::deleteLine() also void function */

	return true;
}

QString KYZisDoc::textLine(unsigned int line) const
{
	//Quanta crashes when it asks for the last line, I am not sure whether our lineCount() is wrong
	//or if it's just quanta which does not count properly (it asks textLine from 0 to 218 in my test file,
	//whereas I said it the file have 218 lines)
	if ( line >= m_buffer->lineCount() ) return QString::null;
	return m_buffer->textline(line);
}

uint KYZisDoc::length() const
{
	uint textlength = m_buffer->getWholeTextLength();

	return textlength;
}

int KYZisDoc::lineLength(unsigned int line) const
{
	uint length = m_buffer->textline(line).length();

	return length;
}

bool KYZisDoc::clear()
{
	m_buffer->clearText();
	return true;
}

bool KYZisDoc::insertText( uint line, uint col, const QString &s)
{
	m_buffer->insertChar(col, line, s);

	return true;
}

QString KYZisDoc::text (  uint startLine, uint startCol, uint endLine, uint endCol ) const {
	return m_buffer->getText( YZCursor(startCol,startLine), YZCursor(endCol,endLine) ).join("\n");
}

bool KYZisDoc::setText (  const QString &text ) {
	QString content = text;
	m_buffer->loadText( &content );
	return true;
}

bool KYZisDoc::removeText (  uint startLine, uint startCol, uint endLine, uint endCol) {
	YZView *v = m_buffer->firstView();
	m_buffer->action()->deleteArea( v, YZCursor( startCol, startLine ), YZCursor( endCol, endLine ), QValueList<QChar>());
	return true;
}

unsigned int KYZisDoc::hlMode () {
	return YzisHlManager::self()->findHl( m_buffer->highlight() );
}

bool KYZisDoc::setHlMode (unsigned int mode) {
	m_buffer->setHighLight( mode );
	return true;
}

unsigned int KYZisDoc::hlModeCount () {
	return YzisHlManager::self()->highlights();
}

QString KYZisDoc::hlModeName (unsigned int mode) {
	return YzisHlManager::self()->hlName( mode );
}

QString KYZisDoc::hlModeSectionName (unsigned int mode) {
	return YzisHlManager::self()->hlSection( mode );
}

void KYZisDoc::highlightingChanged( ) {
	emit hlChanged();
}

void KYZisDoc::undo() {
	YZView *v = m_buffer->firstView();
	m_buffer->undoBuffer()->undo(v);
}

void KYZisDoc::redo() {
	YZView *v = m_buffer->firstView();
	m_buffer->undoBuffer()->redo(v);
}

void KYZisDoc::clearUndo() {
	m_buffer->undoBuffer()->clearUndo();
}

void KYZisDoc::clearRedo() {
	m_buffer->undoBuffer()->clearRedo();
}

unsigned int KYZisDoc::undoCount() const {
	return m_buffer->undoBuffer()->undoCount();
}

unsigned int KYZisDoc::redoCount() const {
	return m_buffer->undoBuffer()->redoCount();
}

unsigned int KYZisDoc::undoSteps() const {
	return 0;
}

void KYZisDoc::setUndoSteps(unsigned int ) {
	//nothing
}

//KTextEditor::ConfigInterface implementation

void KYZisDoc::readConfig() {
	KYZisFactory::self()->readConfig();
}

void KYZisDoc::writeConfig() {
	KYZisFactory::self()->writeConfig();
}

void KYZisDoc::readConfig( KConfig */*config*/ ) {
	//how to use another config file with kconfigxt?
}

void KYZisDoc::writeConfig( KConfig */*config*/ ) {
	//how to use another config file with kconfigxt?
}

void KYZisDoc::readSessionConfig( KConfig *config ) {
	KURL url(config->readEntry("URL"));
	if (!url.isEmpty() && url.isValid())
		openURL(url);
}

void KYZisDoc::writeSessionConfig( KConfig *config ) {
	config->writeEntry("URL", url().prettyURL());
}

void KYZisDoc::configDialog() {
	if ( KConfigDialog::showDialog( "configure_editor" ) )
		return;

	KYZisConfigDialog *dialog = new KYZisConfigDialog( m_parent, "configure_editor", Settings::self(), KDialogBase::TreeList );
	connect( dialog, SIGNAL( settingsChanged() ), KYZisFactory::self(), SLOT( applyConfig() ) );
	dialog->exec();
}

void KYZisDoc::setModified( bool modified ) {
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
void KYZisDoc::emitSelectionChanged() {
	emit selectionChanged();
}


//KTextEditor::MarkInterface slots

uint KYZisDoc::mark( uint line ) {
	return m_buffer->docMarks()->get(line);
}

void KYZisDoc::setMark( uint line, uint markType ) {
	m_buffer->docMarks()->add(line, markType);
}

void KYZisDoc::clearMark( uint line ) {
	m_buffer->docMarks()->del(line);
}

void KYZisDoc::addMark( uint line, uint markType ) {
	m_buffer->docMarks()->add(line, markType);
}

void KYZisDoc::removeMark( uint line, uint markType ) {
	m_buffer->docMarks()->del(line, markType);
}

QPtrList<KTextEditor::Mark> KYZisDoc::marks() {
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

void KYZisDoc::clearMarks() {
	m_buffer->docMarks()->clear();
}

/*
 * KTextEditor::MarkInterfaceExtension
 */
void KYZisDoc::setPixmap(MarkInterface::MarkTypes, const QPixmap &) {
}
void KYZisDoc::setDescription(MarkInterface::MarkTypes, const QString &){
}
void KYZisDoc::setMarksUserChangable(uint ) {
}
void KYZisDoc::markChanged (KTextEditor::Mark , KTextEditor::MarkInterfaceExtension::MarkChangeAction ) {
}


uint KYZisDoc::configPages() const {
	return 1;	
}

KTextEditor::ConfigPage *KYZisDoc::configPage ( uint /*number*/, QWidget *parent, const char */*name*/ ) {
	return new YzisSchemaConfigPage( parent, this );
}

QString KYZisDoc::configPageName ( uint /*number*/ ) const {
	return i18n("Syntax Highlighting");
}

QString KYZisDoc::configPageFullName ( uint /*number*/) const {
	return i18n("Syntax Highlighting");
}

QPixmap KYZisDoc::configPagePixmap ( uint /*number*/, int /*size*/ ) const {
	return 0;
}

/*
 * KTextEditor::SelectionInterface
 */
bool KYZisDoc::setSelection( unsigned int startLine, unsigned int startCol, unsigned int endLine, unsigned int endCol ) {
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
bool KYZisDoc::clearSelection() {
	YZView *v = m_buffer->firstView();
	YZASSERT_MSG( v->modePool()->current()->isSelMode(), "There is no selection" );
	v->modePool()->pop();
	return true;
}
bool KYZisDoc::hasSelection() const {
	YZView *v = m_buffer->firstView();
	return !(v->getSelectionPool()->visual()->isEmpty());
}
QString KYZisDoc::selection() const {
	YZView *v = m_buffer->firstView();
	YZASSERT_MSG( v->modePool()->current()->isSelMode(), "There is no selection" );
	QValueList<QChar> regs;
	YZInterval i = dynamic_cast<YZModeVisual*>( v->modePool()->current() )->interval( YZCommandArgs(NULL,v,regs,1,false) );
	return ((YZBuffer*)this)->getText( i ).join("\n");
}
bool KYZisDoc::removeSelectedText() {
	YZView *v = m_buffer->firstView();
	YZASSERT_MSG( v->modePool()->current()->isSelMode(), "There is no selection" );
	dynamic_cast<YZModeVisual*>( v->modePool()->current() )->execCommand( v, "d" );
	return true;
}
bool KYZisDoc::selectAll() {
	if ( m_buffer->lineCount() == 0 )
		return true;
	return setSelection( 0, 0, m_buffer->lineCount() - 1, qMax( (int)(m_buffer->textline( m_buffer->lineCount() - 1 ).length() - 1), 0 ) );
}

/*
 * KTextEditor::SelectionInterfaceSel
 */
int KYZisDoc::selStartLine() {
	YZView *v = m_buffer->firstView();
	YZDoubleSelection* visual = v->getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].fromPos().y();
}
int KYZisDoc::selStartCol() {
	YZView *v = m_buffer->firstView();
	YZDoubleSelection* visual = v->getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].fromPos().x();
}
int KYZisDoc::selEndLine() {
	YZView *v = m_buffer->firstView();
	YZDoubleSelection* visual = v->getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].toPos().y();
}
int KYZisDoc::selEndCol() {
	YZView *v = m_buffer->firstView();
	YZDoubleSelection* visual = v->getSelectionPool()->visual();
	YZASSERT_MSG( visual->isEmpty(), "There is no selection" );
	return visual->bufferMap()[ 0 ].toPos().x();
}

QPtrList<KTextEditor::View> KYZisDoc::views() const
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
