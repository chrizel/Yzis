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
#include "viewwidget.h"
#include "factory.h"
#include "debug.h"

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name), YZView(doc, KYZisFactory::s_self, 10)
{
	editor = new KYZisEdit (this,"editor");
	status = new KStatusBar (this, "status");
	command = new KYZisCommand ( this, "command");

	status->insertItem("Yzis Ready",0,1);
	status->setItemAlignment(0,Qt::AlignLeft);

	status->insertItem("",80,80,0);
	status->setItemAlignment(0,Qt::AlignLeft);

/*	status->insertItem("Yzis Ready",0,1);
	status->setItemAlignment(0,Qt::AlignRight);
*/
	status->insertItem("",99,0,true);
	status->setItemAlignment(99,Qt::AlignRight);

	QVBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(editor);
	l->addWidget( command );
	l->addWidget(status);

	KYZisFactory::registerView( this );

	buffer = doc;
	editor->show();
	status->show();
	editor->setFocus();
}

KYZisView::~KYZisView () {
	yzDebug() << "KYZisView::~KYZisView" << endl;
	if ( buffer ) buffer->removeView(this);
	KYZisFactory::deregisterView( this );
}

void KYZisView::setCommandLineText( const QString& text ) {
	command->setText( text );
}

QString KYZisView::getCommandLineText() const {
	return command->text();
}

void KYZisView::setFocusMainWindow() {
	editor->setFocus();
}

void KYZisView::setFocusCommandLine() {
	command->setFocus();
}

void KYZisView::scrollDown( int lines ) {
	editor->scrollBy(0, lines * editor->fontMetrics().lineSpacing());
	editor->update();
}

void KYZisView::scrollUp ( int lines ) {
	editor->scrollBy(0, -1 * lines * editor->fontMetrics().lineSpacing());
	editor->update();
}

void KYZisView::invalidateLine ( unsigned int line ) {
	editor->setTextLine( line, buffer->data( line ) );
}

void KYZisView::setStatusBar ( const QString& text ) {
	status->changeItem(text, 0);
}

void KYZisView::updateCursor ( unsigned int line, unsigned int x1, unsigned int x2, const QString& percentage ) {
	editor->setCursor(x1, line);
	if (x1!=x2)
		status->changeItem( QString("%1,%2-%3 (%4)").arg(line+1 ).arg( x1+1 ).arg( x2+1 ).arg( percentage),99 );
	else
		status->changeItem( QString("%1,%2 (%3)").arg(line+1 ).arg( x1+1 ).arg( percentage),99 );
}

void KYZisView::refreshScreen () {
	editor->updateContents( );
}

#include "viewwidget.moc"
