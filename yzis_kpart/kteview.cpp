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
#include "kyziscommandwidget.h"

#include <libyzis/buffer.h>

#include <QGridLayout>

#include <kstatusbar.h>


KTEView::KTEView(KTEDocument* doc, QWidget* parent)
	: KTextEditor::View(parent), m_doc(doc)
{
	m_view = static_cast<KYZisView*>(KYZisSession::self()->createView(doc->buffer()));
	KStatusBar* status = new KStatusBar(this);
	m_command = new KYZisCommand(m_view, this);

	QGridLayout* g = new QGridLayout(this);
	g->addWidget(m_view, 0, 0);
	g->addWidget(m_command, 1, 0);
	g->addWidget(status, 2, 0);


	m_view->show();
	status->show();
	m_command->show();
}

KTEView::~KTEView()
{

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

void KTEView::setContextMenu(QMenu* /*menu*/)
{
    // TODO: implement
}

QMenu* KTEView::contextMenu() const
{
    // TODO: implement
	return NULL;
}

QMenu* KTEView::defaultContextMenu(QMenu* /*menu*/) const
{
    // TODO: implement
	return NULL;
}

bool KTEView::setCursorPosition(KTextEditor::Cursor /*position*/)
{
    // TODO: implement
	return false;
}

KTextEditor::Cursor KTEView::cursorPosition() const
{
    // TODO: implement
	return KTextEditor::Cursor();
}

KTextEditor::Cursor KTEView::cursorPositionVirtual() const
{
    // TODO: implement
	return KTextEditor::Cursor();
}

QPoint KTEView::cursorToCoordinate(const KTextEditor::Cursor& /*cursor*/) const
{
    // TODO: implement
	return QPoint(0,0);
}

QPoint KTEView::cursorPositionCoordinates() const
{
    // TODO: implement
	return QPoint(0,0);
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
