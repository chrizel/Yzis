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

#include "editor.h"
#include "debug.h"
#include "yzis.h"

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
: QScrollView( parent, name,WStaticContents | WRepaintNoErase | WResizeNoErase ) 
{
	setFont(QFont("Fixed",10));
	_parent = parent;

	viewport()->setFocusProxy( this );
	viewport()->setFocusPolicy( StrongFocus );
	viewport()->setBackgroundMode( PaletteBase );
	//viewport()->setPaletteBackgroundColor(QColor("white"));
	viewport()->setBackgroundColor(QColor("black"));
	viewport()->setPaletteForegroundColor(QColor("white"));
	cursor_shown = false; //cursor is not shown
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::viewportResizeEvent(QResizeEvent *ev) {
	QSize s = ev->size();
	int lines=0;
	lines = s.height() / fontMetrics().lineSpacing();
	_parent->setVisibleLines( lines );
}

void KYZisEdit::setCursor(int c, int l) {
	//undraw previous cursor
	if ( cursor_shown ) drawCursorAt( cursorx,cursory );
	cursorx = c;
	cursory = l;
	//draw new cursor
	cursor_shown = true;
	drawCursorAt( cursorx,cursory );
}

void KYZisEdit::setTextLine(int l, const QString &/*str*/) {
	updateContents( 0, ( l - _parent->getCurrent() ) * fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing()  );
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
	yzDebug()<< " Got key : " << e->key()<< " Got ASCII : " << e->ascii() << " Got Unicode : " << e->text() << endl;
	if ( e->key() != 0 ) {
		int modifiers=0;
		ButtonState st = e->state();
		if ( st & Qt::ShiftButton ) modifiers |= YZIS::Shift;
		if ( st & Qt::ControlButton ) modifiers |= YZIS::Ctrl;
		if ( st & Qt::AltButton ) modifiers |= YZIS::Alt;
		if ( st & Qt::MetaButton ) modifiers |= YZIS::Meta;
		if (e->key() != Qt::Key_unknown) _parent->sendKey(e->key(), modifiers);
		e->accept();
	}
}

void KYZisEdit::contentsMousePressEvent ( QMouseEvent * e ) {
	if (_parent->getCurrentMode() != YZView::YZ_VIEW_MODE_EX)
		_parent->gotoxy(e->x()/fontMetrics().maxWidth() , e->y()/fontMetrics().lineSpacing() + _parent->getCurrent());
}

void KYZisEdit::drawCursorAt(int x, int y) {
	bitBlt (
			viewport(),
			x*fontMetrics().maxWidth(),( y - _parent->getCurrent() ) *fontMetrics().lineSpacing(),
			viewport(),
			x*fontMetrics().maxWidth(), ( y - _parent->getCurrent() )*fontMetrics().lineSpacing(),
			fontMetrics().maxWidth(), fontMetrics().lineSpacing(),
			Qt::NotROP,	    // raster Operation
			true );		    // ignoreMask
}

void KYZisEdit::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph) {
	for ( unsigned int i=0; i < _parent->getLinesVisible() ; ++i ) {
		if ( fontMetrics().lineSpacing() * i >= ( unsigned int )clipy && fontMetrics().lineSpacing() * i <= ( unsigned int ) ( clipy+cliph ) ) {
			QRect clip(0, i * fontMetrics().lineSpacing(), width(),fontMetrics().lineSpacing());
			p->eraseRect(clip);
			if ( _parent->myBuffer()->getText().count() > i + _parent->getCurrent() )
				p->drawText(clip,Qt::AlignLeft|Qt::DontClip|Qt::SingleLine ,_parent->myBuffer()->getText()[ i + _parent->getCurrent() ]);
			else 
				p->drawText(clip,Qt::AlignLeft|Qt::DontClip|Qt::SingleLine ,"~");
		}
	}

	drawCursorAt(cursorx,cursory);
}

#include "editor.moc"
