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

#include "document.h"
#include "viewwidget.h"
#include "factory.h"
#include "debug.h"

KYZisDoc::KYZisDoc (bool bSingleViewMode, bool bBrowserView, bool bReadOnly, QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name)
	: KTextEditor::Document(parent,name), YZBuffer(KYZisFactory::sess,QString::null) {
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
	//backport to Factory ? XXX
	if ( !v )
		return;

	_views.removeRef( v );
}

bool KYZisDoc::openFile () {
	yzDebug() << "openFile " << m_file << endl;
	mPath = m_file;
	load();
	return true;
}

bool KYZisDoc::saveFile () {
	YZBuffer::save();
	return true;
}

#include "document.moc"
