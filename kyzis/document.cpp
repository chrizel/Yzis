/*
    Copyright (c) 2003,2004 Mickael Marchand <marchand@kde.org>
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

#include <kfiledialog.h>
#include <kmessagebox.h>
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

#include "configdialog.h"

KYZisDoc::KYZisDoc (int kId, QWidget *parentWidget, const char *, QObject *parent, const char *name)
	: KTextEditor::Document(parent,name), YZBuffer(KYZisFactory::s_self) {

		setInstance(KYZisFactory::instance());
		m_parent = parentWidget;
		mkId = kId;

		setupActions();
}

KYZisDoc::~KYZisDoc () {
}


KTextEditor::View *KYZisDoc::createView ( QWidget *parent, const char *) {
	//backport to Factory ? XXX
	KYZisView *v = new KYZisView (this, parent);
	//FIXME : two lists
	addView(v);
	_views.append( v );
	return v;
}

void KYZisDoc::setupActions() {
}

/*void KYZisDoc::configureEditor() {

	if ( KConfigDialog::showDialog( "configure_editor" ) )
		return;

	KYZisConfigDialog *dialog = new KYZisConfigDialog( m_parent, "configure_editor", Settings::self(), KDialogBase::TreeList );
	connect( dialog, SIGNAL( settingsChanged() ), KYZisFactory::s_self, SLOT( writeConfig() ) );
	dialog->show();
}*/

void KYZisDoc::removeView( KTextEditor::View * v ) {
	yzDebug() << "Document : removeView" << endl;
	if ( !v )
		return;

	_views.removeRef( v );
}

bool KYZisDoc::openFile () {
	yzDebug() << "openFile " << m_file << endl;
	load(m_file);
	return true;
}

bool KYZisDoc::saveFile () {
	return YZBuffer::save();
}

/* Implementation of KTextEditor */

QString KYZisDoc::text() const
{
	return getWholeText();
}


uint KYZisDoc::numLines() const 
{
	return lineCount();
}

bool KYZisDoc::insertLine(unsigned int line, const QString &s)
{
	YZBuffer::insertLine(s, line);

	/* YZBuffer::insertLine() is a void function, apparently it can't fail.
	 * Return true
	 */

	return true;
}

bool KYZisDoc::removeLine(unsigned int line)
{
	deleteLine(line);

	/* YZBuffer::deleteLine() also void function */

	return true;
}

QString KYZisDoc::textLine(unsigned int line) const
{
	//Quanta crashes when it asks for the last line, I am not sure whether our lineCount() is wrong
	//or if it's just quanta which does not count properly (it asks textLine from 0 to 218 in my test file, 
	//whereas I said it the file have 218 lines)
	if ( line >= lineCount() ) return QString::null;
	return textline(line);
}

uint KYZisDoc::length() const
{
	uint textlength = getWholeTextLength();

	return textlength;
}

int KYZisDoc::lineLength(unsigned int line) const
{
	uint length = textline(line).length();

	return length;
}

bool KYZisDoc::clear()
{
	clearText();
	return true;
}

bool KYZisDoc::insertText( uint line, uint col, const QString &s)
{
	insertChar(col, line, s);

	return true;
}

QString KYZisDoc::text (  uint startLine, uint startCol, uint endLine, uint endCol ) const {
	QString content = "";
	if ( startLine == endLine ) {
		content = textline( startLine ).mid(startCol, endCol-startCol);
		return content;
	}
	
	for ( unsigned int i = startLine; i <= endLine; i++ ) {
		if ( i == startLine ) 
			content+=textline(i).mid( startCol );
		else if ( i == endLine )
			content+=textline(i).left( endCol );
		else
			content+=textline( i );
	}
	return content;
}

bool KYZisDoc::setText (  const QString &text ) {
	clearText();
	QStringList list = QStringList::split( "\n", text );
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
		appendLine(*it);
	}
	return true;
}

bool KYZisDoc::removeText (  uint startLine, uint startCol, uint endLine, uint endCol) {
	YZView *v = dynamic_cast<YZView*>( _views.first() );
	( ( YZBuffer* )( this ) )->action()->deleteArea( v, YZCursor( v, startCol, startLine ), YZCursor(v,endCol, endLine ), QValueList<QChar>());
	return true;
}

bool KYZisDoc::popupFileSaveAs() {
	KURL url =	KFileDialog::getSaveURL();
	if ( url.isEmpty() ) return false;//canceled
	else if ( !url.isLocalFile() ) {
		KMessageBox::sorry(m_parent, tr("Yzis is not able to save remote files for now" ), tr( "Remote files"));
		return false;
	} else if ( ! url.isEmpty() ) {
		setPath( url.path() );
		return true;
	}
	return false;
}

void KYZisDoc::filenameChanged() {
	DCOPClient *client = kapp->dcopClient();
	KTextEditor::View *it;
	for ( it = _views.first(); it; it = _views.next() ) {
		KYZisView *yv = static_cast<KYZisView*>(it);
		int id = yv->getkid();
		QByteArray data;
		QDataStream arg(data, IO_WriteOnly);
		arg << id << fileName();
		yzDebug() << "filenameChanged : view " << id << " " << fileName() << endl;

		bool w = client->send(client->appId(), "Kyzis", "setCaption(int,QString)", data);
		if (w) {
			yzDebug() << "DCOP call successful for " << client->appId() << " to change caption " << endl;
		} else {
			yzDebug() << "DCOP call failed for " << client->appId() << endl;
			//popupMessage( "DCOP communication is broken ! KYzis is not able to change the caption of tabs" );
			return; //we failed
		}
	}
}

unsigned int KYZisDoc::hlMode () {
	return YzisHlManager::self()->findHl( highlight() );
}

bool KYZisDoc::setHlMode (unsigned int mode) {
	setHighLight( mode );
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
	YZView *v = dynamic_cast<YZView*>( _views.first() );
	undoBuffer()->undo(v);		
}

void KYZisDoc::redo() {
	YZView *v = dynamic_cast<YZView*>( _views.first() );
	undoBuffer()->redo(v);		
}

void KYZisDoc::clearUndo() {
	undoBuffer()->clearUndo();	
}

void KYZisDoc::clearRedo() {
	undoBuffer()->clearRedo();	
}

unsigned int KYZisDoc::undoCount() const {
	return undoBuffer()->undoCount();	
}

unsigned int KYZisDoc::redoCount() const {
	return undoBuffer()->redoCount();	
}

unsigned int KYZisDoc::undoSteps() const {
	return 0;	
}

void KYZisDoc::setUndoSteps(unsigned int ) {
	//nothing
}

//KTextEditor::ConfigInterface implementation

void KYZisDoc::readConfig() {
	KYZisFactory::s_self->readConfig();
}

void KYZisDoc::writeConfig() {
	KYZisFactory::s_self->writeConfig();
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
	connect( dialog, SIGNAL( settingsChanged() ), KYZisFactory::s_self, SLOT( applyConfig() ) );
	dialog->exec();
}

void KYZisDoc::setModified( bool modified ) {
	for (QPtrList<KTextEditor::View>::const_iterator it = _views.constBegin();
		it != _views.constEnd(); ++it)
	{
		KYZisView *kv = dynamic_cast<KYZisView *>(*it);
		if (kv)
			kv->emitNewStatus();
	}
	KTextEditor::Document::setModified(modified);
}



//KTextEditor::MarkInterface slots

uint KYZisDoc::mark( uint line ) {
	return docMarks()->get(line);
}

void KYZisDoc::setMark( uint line, uint markType ) {
	docMarks()->add(line, markType); 
}

void KYZisDoc::clearMark( uint line ) {
	docMarks()->del(line);
}

void KYZisDoc::addMark( uint line, uint markType ) {
	docMarks()->add(line, markType); 
}

void KYZisDoc::removeMark( uint line, uint markType ) {
	docMarks()->del(line, markType); 
}

QPtrList<KTextEditor::Mark> KYZisDoc::marks() {
	QPtrList<KTextEditor::Mark> marks;
	const YZDocMarker &marker = docMarks()->getMarker();
	for (YZDocMarker::const_iterator it = marker.constBegin(); it != marker.constEnd(); ++it)
	{
		KTextEditor::Mark *m = new KTextEditor::Mark;
		m->line = it.key();
		m->type = it.data();
		marks.append(m);
	}
}

void KYZisDoc::clearMarks() {
	docMarks()->clear();
}

#include "document.moc"
