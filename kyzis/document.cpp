/*
	  Copyright (c) 2003 Yzis Team <yzis-dev@yzis.org>

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
#include "document.h"
#include "viewwidget.h"
#include "factory.h"
#include "debug.h"

KYZisDoc::KYZisDoc (bool bSingleViewMode, bool bBrowserView, bool bReadOnly, QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name)
	: KTextEditor::Document(parent,name), YZBuffer(KYZisFactory::s_self,QString::null) {
		setInstance(KYZisFactory::instance());
		KYZisFactory::registerDocument( this );
		m_parent = parentWidget;
}

KYZisDoc::~KYZisDoc () {
	KYZisFactory::deregisterDocument( this );
}

KTextEditor::View *KYZisDoc::createView ( QWidget *parent, const char *) {
	//backport to Factory ? XXX
	KYZisView *v = new KYZisView (this, parent);
	//FIXME : two lists
	addView(v);
	_views.append( v );
	return v;
}

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
	YZBuffer::save();
	return true;
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
	QString s = textline(line);
	return s;
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

/* Fixme */
bool KYZisDoc::removeText (  uint startLine, uint startCol, uint endLine, uint endCol ) {
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

#include "document.moc"
