/* This file is part of the Yzis libraries
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
 *  Copyright (C) 2005 Erlend Hamberg <hamberg@stud.ntnu.no>
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

#include "kteview.h"
#include "ktedocument.h"
#include "kyzisview.h"
#include "kyzissession.h"

#include <libyzis/buffer.h>

#include <QGridLayout>
#include <QMenu>

#include <kstatusbar.h>
#include <ksqueezedtextlabel.h>
#include <kxmlguifactory.h>


KTEView::KTEView( KTEDocument* doc, QWidget* parent )
	: KTextEditor::View(parent), m_doc(doc), m_popup(0)
{
	m_view = static_cast<KYZisView*>(KYZisSession::self()->createView(doc->buffer()));
	m_view->setParent( this );
	KStatusBar* status = new KStatusBar( this );

	status->insertItem(QString("Yzis Ready"),0,1);
	status->setItemAlignment(0,Qt::AlignLeft);

	m_central = new KSqueezedTextLabel( this );
	status->addWidget( m_central, 100 );

	status->insertItem("",90,1);
	status->setItemAlignment(90,Qt::AlignRight);

	status->insertItem("",99,0);
	status->setItemAlignment(99,Qt::AlignRight);

	QGridLayout* g = new QGridLayout( this );
	g->addWidget( m_view, 0, 0 );
	g->addWidget( status, 1, 0 );

	setXMLFile( "yzis_kpart/yzis_kpart.rc" );

	m_view->show();
	status->show();
}

KTEView::~KTEView()
{
	delete m_view;
}

KTextEditor::Document* KTEView::document() const
{
	return m_doc;
}

QString KTEView::viewMode() const 
{
	// TODO: implement
	return QString();
}

enum KTextEditor::View::EditMode KTEView::viewEditMode() const
{
	// TODO: implement
	return KTextEditor::View::EditInsert;
}

void KTEView::setContextMenu( QMenu* menu )
{
	m_popup = menu;
}

QMenu* KTEView::contextMenu() const
{
	return m_popup;
}

QMenu* KTEView::defaultContextMenu( QMenu* menu ) const
{
        QMenu * popup = 0;
        if (m_popup)
                popup =  m_popup;
        else
		popup = menu;
        
	return popup;
}

bool KTEView::setCursorPosition( KTextEditor::Cursor position )
{
	kDebug() << "KTEView::setCursorPosition x: " << position.column() << " y: " << position.line() << "\n";
	m_view->gotoxy( position.column(), position.line() );
	return true;
}

KTextEditor::Cursor KTEView::cursorPosition() const
{
	return KTextEditor::Cursor( m_view->getBufferCursor().y(), m_view->getBufferCursor().x() );	
}

KTextEditor::Cursor KTEView::cursorPositionVirtual() const
{
	return KTextEditor::Cursor( m_view->getBufferCursor().y(), m_view->getBufferCursor().x() );	
}

QPoint KTEView::cursorToCoordinate( const KTextEditor::Cursor& cursor ) const
{
	return QPoint( cursor.column(), cursor.line() );
}

QPoint KTEView::cursorPositionCoordinates() const
{
	return cursorToCoordinate( cursorPosition() );
}

bool KTEView::mouseTrackingEnabled() const
{
    // TODO: implement
	return false;
}

bool KTEView::setMouseTrackingEnabled(bool /*enable*/)
{
    // TODO: implement
	return false;
}

bool KTEView::setSelection(const KTextEditor::Range& /*range*/)
{
    // TODO: implement
	return false;
}

bool KTEView::selection() const
{
    // TODO: implement
	return false;
}

const KTextEditor::Range& KTEView::selectionRange() const
{
    // TODO: implement
	static KTextEditor::Range remove_me;
	return remove_me;
}

QString KTEView::selectionText() const
{
    // TODO: implement
	return QString();
}

bool KTEView::removeSelection()
{
    // TODO: implement
	return false;
}

bool KTEView::removeSelectionText()
{
    // TODO: implement
	return false;
}

bool KTEView::setBlockSelection(bool /*on*/)
{
    // TODO: implement
	return false;
}

bool KTEView::blockSelection() const
{
    // TODO: implement
	return false;
}

#include "kteview.moc"
