/* This file is part of the Yzis libraries
 *  Copyright (C) 2003 Yzis Team <yzis-dev@yzis.org>
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
#include "viewwidget.h"
#include "factory.h"
#include <qlayout.h>
#include <qevent.h>
#include <kapplication.h>
#include "debug.h"

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name), YZView(doc, KYZisFactory::sess, 10)
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
	if ( buffer ) buffer->removeView(this);
	KYZisFactory::deregisterView( this );
}

//DROP ME XXX
void KYZisView::showEvent ( QShowEvent* ) {
	yzDebug() << "View " << myId << " got activated" << endl;
	KYZisFactory::s_self->setCurrentView( this );
}

#include "viewwidget.moc"
