/* This file is part of the Yzis libraries
 *  Copyright (C) 2007 Lothar Braun <lothar@lobraun.de>
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef _KTE_VIEW_H_
#define _KTE_VIEW_H_

#include <ktexteditor/view.h>

class KTEDocument;
class KYZisView;
class KYZisCommand;

class KTEView : public KTextEditor::View
{
Q_OBJECT
public:
	KTEView(KTEDocument* doc, QWidget* parent);
	virtual ~KTEView();
	virtual KTextEditor::Document* document() const; 
	virtual QString viewMode() const;
	virtual enum KTextEditor::View::EditMode viewEditMode() const;
	virtual void setContextMenu(QMenu *menu);
	virtual QMenu* contextMenu() const;
	virtual QMenu* defaultContextMenu(QMenu *menu=0L) const;
	virtual bool setCursorPosition(KTextEditor::Cursor position);
	virtual KTextEditor::Cursor cursorPosition() const;
	virtual KTextEditor::Cursor cursorPositionVirtual() const;
	virtual QPoint cursorToCoordinate(const KTextEditor::Cursor &cursor) const;
	virtual QPoint cursorPositionCoordinates() const;
	virtual bool mouseTrackingEnabled() const;
	virtual bool setMouseTrackingEnabled(bool enable);
	virtual bool setSelection(const KTextEditor::Range &range);
	virtual bool selection() const;
	virtual const KTextEditor::Range& selectionRange() const;
	virtual QString selectionText() const;
	virtual bool removeSelection();
	virtual bool removeSelectionText();
	virtual bool setBlockSelection(bool on);
	virtual bool blockSelection() const;
signals:
	void   focusIn (KTextEditor::View *view);
	void   focusOut (KTextEditor::View *view);
	void   viewModeChanged (KTextEditor::View *view);
	void   viewEditModeChanged (KTextEditor::View *view, enum KTextEditor::View::EditMode mode);
	void   informationMessage (KTextEditor::View *view, const QString &message);
	void   textInserted (KTextEditor::View *view, const KTextEditor::Cursor &position, const QString &text);
	void   contextMenuAboutToShow (KTextEditor::View *view, QMenu *menu);
	void   cursorPositionChanged (KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
	void   verticalScrollPositionChanged (KTextEditor::View *view, const KTextEditor::Cursor &newPos);
	void   horizontalScrollPositionChanged (KTextEditor::View *view);
	void   mousePositionChanged (KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
	void   selectionChanged (KTextEditor::View *view);

private:
	KTEDocument* m_doc;
	KYZisView* m_view;
	KYZisCommand* m_command;
};

#endif
