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

	viewport()->setFocusProxy( this );
	viewport()->setFocusPolicy( StrongFocus );
	viewport()->setBackgroundMode( PaletteBase );
	//viewport()->setPaletteBackgroundColor(QColor("white"));
	viewport()->setBackgroundColor(QColor("black"));
	viewport()->setPaletteForegroundColor(QColor("white"));
	mCursorShown = false; //cursor is not shown
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::viewportResizeEvent(QResizeEvent *ev) {
	yzDebug() << "viewportResizeEvent" << endl;
	QSize s = ev->size();
	int lines = s.height() / fontMetrics().lineSpacing();
	mParent->setVisibleLines( lines );
}

void KYZisEdit::setCursor(int c, int l) {
	yzDebug() << "setCursor " << c << ", " << l << endl;
	//undraw previous cursor
	if ( mCursorShown && ( c!= mCursorX || l!=mCursorY ) ) {
		yzDebug() << "Undraw previous cursor" << endl;
		drawCursorAt( mCursorX,mCursorY );
	} else if ( mCursorShown ) {
		yzDebug() << "Dont redraw cursor" << endl;
		return;
	}

	mCursorX = c;
	mCursorY = l;

	//draw new cursor
	mCursorShown = true;
	
	//check if the line contains TABs
	QString currentLine = mParent->myBuffer()->data( mCursorY );
	int s = fontMetrics().size( Qt::ExpandTabs|Qt::SingleLine, currentLine, c, 0, 0).width();
	mCursorX = s / fontMetrics().maxWidth();
	drawCursorAt( mCursorX,mCursorY );
}

void KYZisEdit::setTextLine(int l, const QString &/*str*/) {
	updateContents( 0, ( l - mParent->getCurrent() ) * fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing()  );
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
		ButtonState st = e->state();
		if (e->key() != Qt::Key_unknown) mParent->sendKey(e->key(), st);
		e->accept();
	}
}

void KYZisEdit::contentsMousePressEvent ( QMouseEvent * e ) {
	if (mParent->getCurrentMode() != YZView::YZ_VIEW_MODE_EX) {
		QString line = mParent->myBuffer()->data(e->y()/fontMetrics().lineSpacing() + mParent->getCurrent());
		int nbcols=0;
		int len=0;
		while ( len <= e->x() ) {
			len = fontMetrics().size( Qt::ExpandTabs|Qt::SingleLine, line, nbcols ,0 , 0).width();
			nbcols++;
		}
		nbcols = nbcols - 2 >= 0 ? nbcols -2 : 0; //dont ask me why i need this :) i understand -1 but not -2 ...
		mParent->gotoxy(nbcols, e->y()/fontMetrics().lineSpacing() + mParent->getCurrent());
	}
}

void KYZisEdit::drawCursorAt(int x, int y) {
	yzDebug() << "drawCursorAt :" << x << ", " << y << endl;
	bitBlt (
			viewport(),
			x*fontMetrics().maxWidth(),( y - mParent->getCurrent() ) *fontMetrics().lineSpacing(),
			viewport(),
			x*fontMetrics().maxWidth(), ( y - mParent->getCurrent() )*fontMetrics().lineSpacing(),
			fontMetrics().maxWidth(), fontMetrics().lineSpacing(),
			Qt::NotROP,	    // raster Operation
			true );		    // ignoreMask
}

void KYZisEdit::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph) {
	yzDebug() << "drawContents " << endl;
	int flag = mParent->myBuffer()->introShown() ? Qt::AlignCenter : Qt::AlignLeft;

	for ( unsigned int i=0; i < mParent->getLinesVisible() ; ++i ) {
		if ( fontMetrics().lineSpacing() * i >= ( unsigned int )clipy && fontMetrics().lineSpacing() * i <= ( unsigned int ) ( clipy+cliph ) ) {
			QRect clip(0, i * fontMetrics().lineSpacing(), width(),fontMetrics().lineSpacing());
			p->eraseRect(clip);
			if (mParent->myBuffer()->lineCount() > i + mParent->getCurrent() )
				p->drawText(clip,Qt::ExpandTabs|flag|Qt::DontClip|Qt::SingleLine ,mParent->myBuffer()->data(i + mParent->getCurrent()));
			else 
				p->drawText(clip,Qt::ExpandTabs|flag|Qt::DontClip|Qt::SingleLine ,"~");
			if ( ( i + mParent->getCurrent() ) == mCursorY ) 
				drawCursorAt( mCursorX, mCursorY );
		}
	}
}

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	yzDebug() << "Activate Window " << endl;
	KYZisFactory::s_self->setCurrentView( mParent );
}

#include "editor.moc"
