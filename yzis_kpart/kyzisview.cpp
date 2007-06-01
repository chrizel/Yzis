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

#include "kyzisview.h"
#include "kyzisdocument.h"

KYZisView::KYZisView(KYZisDocument* doc, QWidget *parent)
	: KTextEditor::View(parent), m_doc(doc)
{
}

KYZisView::~KYZisView()
{

}

KTextEditor::Document* KYZisView::document() const
{
	return m_doc;
}

QString KYZisView::viewMode() const 
{
	// TODO: implement
	return QString();
}

enum KTextEditor::View::EditMode KYZisView::viewEditMode() const
{
    // TODO: implement
	return KTextEditor::View::EditInsert;
}

void KYZisView::setContextMenu(QMenu* /*menu*/)
{
    // TODO: implement
}

QMenu* KYZisView::contextMenu() const
{
    // TODO: implement
	return NULL;
}

QMenu* KYZisView::defaultContextMenu(QMenu* /*menu*/) const
{
    // TODO: implement
	return NULL;
}

bool KYZisView::setCursorPosition(KTextEditor::Cursor /*position*/)
{
    // TODO: implement
	return false;
}

KTextEditor::Cursor KYZisView::cursorPosition() const
{
    // TODO: implement
	return KTextEditor::Cursor();
}

KTextEditor::Cursor KYZisView::cursorPositionVirtual() const
{
    // TODO: implement
	return KTextEditor::Cursor();
}

QPoint KYZisView::cursorToCoordinate(const KTextEditor::Cursor& /*cursor*/) const
{
    // TODO: implement
	return QPoint(0,0);
}

QPoint KYZisView::cursorPositionCoordinates() const
{
    // TODO: implement
	return QPoint(0,0);
}

bool KYZisView::mouseTrackingEnabled() const
{
    // TODO: implement
	return false;
}

bool KYZisView::setMouseTrackingEnabled(bool /*enable*/)
{
    // TODO: implement
	return false;
}

bool KYZisView::setSelection(const KTextEditor::Range& /*range*/)
{
    // TODO: implement
	return false;
}

bool KYZisView::selection() const
{
    // TODO: implement
	return false;
}

const KTextEditor::Range& KYZisView::selectionRange() const
{
    // TODO: implement
	static KTextEditor::Range remove_me;
	return remove_me;
}

QString KYZisView::selectionText() const
{
    // TODO: implement
	return QString();
}

bool KYZisView::removeSelection()
{
    // TODO: implement
	return false;
}

bool KYZisView::removeSelectionText()
{
    // TODO: implement
	return false;
}

bool KYZisView::setBlockSelection(bool /*on*/)
{
    // TODO: implement
	return false;
}

bool KYZisView::blockSelection() const
{
    // TODO: implement
	return false;
}

#include "kyzisview.moc"
