/* This file is part of the Yzis libraries
 *  Copyright (C) 2004 Mickael Marchand <marchand@kde.org>
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

#include "editor.h"
#include "debug.h"
#include "yzis.h"
#include <kglobalsettings.h>
#include "factory.h"

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
: QScrollView( parent, name,WStaticContents | WNoAutoErase ) 
{
	QFont f ("fixed");
	f.setFixedPitch(true);
	f.setStyleHint(QFont::TypeWriter);
	setFont(f);
	mParent = parent;

	setFocusPolicy( StrongFocus );
	viewport()->setBackgroundMode( PaletteBase );
	//viewport()->setPaletteBackgroundColor(QColor("white"));
	viewport()->setBackgroundColor(QColor("black"));
	viewport()->setPaletteForegroundColor(QColor("white"));
	mCursorShown = false; //cursor is not shown
	mCursorY = mCursorX = 0;
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::viewportResizeEvent(QResizeEvent *ev) {
	QSize s = ev->size();
	int lines = s.height() / fontMetrics().lineSpacing();
	int columns = s.width() / fontMetrics().maxWidth();
	mParent->setVisibleArea( columns, lines );
}

void KYZisEdit::setCursor(int c, int l) {
	//erase the previous cursor by redrawing the line 
	mCursorShown = false; //lock
	repaintContents( mCursorX * fontMetrics().maxWidth(), mCursorY * fontMetrics().lineSpacing(), fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	mCursorShown = true; //unlock
	
	//new position of cursor
	mCursorX = c - mParent->getCurrentLeft();
	mCursorY = l - mParent->getCurrentTop();

	//check if the line contains TABs
	QString currentLine = mParent->myBuffer()->textline( mCursorY + mParent->getCurrentTop() ).mid( mParent->getCurrentLeft(), mParent->getColumnsVisible() );
	int s = fontMetrics().size( Qt::ExpandTabs|Qt::SingleLine, currentLine, mCursorX, /*tabstops*/0, /*tabarray*/0).width();
	mCursorX = s / fontMetrics().maxWidth();
	drawCursorAt( mCursorX, mCursorY );
}

void KYZisEdit::setTextLine(int l, const QString &/*str*/) {
	updateContents( 0, ( l - mParent->getCurrentTop() ) * fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing()  );
}

bool KYZisEdit::event(QEvent *e) {
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Key_Tab ) {
			keyPressEvent(ke);
			return TRUE;
		}
	}
	return QWidget::event(e);
}

void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	if ( e->key() != 0 ) {
		ButtonState st = e->state();
		if (e->key() != Qt::Key_unknown) mParent->sendKey(e->key(), st);
		e->accept();
	}
}

void KYZisEdit::contentsMousePressEvent ( QMouseEvent * e ) {
	if (mParent->getCurrentMode() != YZView::YZ_VIEW_MODE_EX) {
		QString line = mParent->myBuffer()->textline(e->y()/fontMetrics().lineSpacing() + mParent->getCurrentTop()).mid( mParent->getCurrentLeft() );
		int nbcols=0;
		int len=0;
		while ( len <= e->x() ) {
			len = fontMetrics().size( Qt::ExpandTabs|Qt::SingleLine, line, nbcols ,0 , 0).width();
			nbcols++;
		}
		nbcols = nbcols - 2 >= 0 ? nbcols -2 : 0; //dont ask me why i need this :) i understand -1 but not -2 ...
		mParent->gotoxy(nbcols + mParent->getCurrentLeft(), e->y()/fontMetrics().lineSpacing() + mParent->getCurrentTop());
	}
}

void KYZisEdit::drawCursorAt(int x, int y) {
	bitBlt (
			viewport(),
			x*fontMetrics().maxWidth(),y * fontMetrics().lineSpacing(),
			viewport(),
			x*fontMetrics().maxWidth(), y * fontMetrics().lineSpacing(),
			fontMetrics().maxWidth(), fontMetrics().lineSpacing(),
			Qt::NotROP,	    // raster Operation
			true );		    // ignoreMask
}

int KYZisEdit::getLineforY( int y ) {
	int line = y / fontMetrics().lineSpacing();
	return line + mParent->getCurrentTop() - 1;
}

void KYZisEdit::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph) {
	int flag = ( mParent->myBuffer()->introShown() ? Qt::AlignCenter : Qt::AlignLeft ) | Qt::ExpandTabs /*| Qt::DontClip*/ | Qt::SingleLine;

	for ( int i = cliph / fontMetrics().lineSpacing(); i > 0 ; i-- ) {
		int line = getLineforY( clipy + i * fontMetrics().lineSpacing() );
		QRect clip(0, ( i-1 ) * fontMetrics().lineSpacing() + clipy, width(), fontMetrics().lineSpacing());
		p->eraseRect(clip);
		QString toDraw = mParent->myBuffer()->textline( line ).mid(mParent->getCurrentLeft(), mParent->getColumnsVisible() );
		if (mParent->myBuffer()->lineCount() > line )
			p->drawText(clip,flag,toDraw);
		else
			p->drawText(clip,flag ,"~");
		if ( mCursorShown && mCursorY + mParent->getCurrentTop() == line)
			setCursor( mParent->getCursor()->getX(), mCursorY + mParent->getCurrentTop() );
	}
}

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	KYZisFactory::s_self->setCurrentView( mParent );
}

#include "editor.moc"
