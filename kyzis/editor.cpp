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

static const QChar tabChar( '\t' );
static const QChar spaceChar( ' ' );

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
: QScrollView( parent, name,WStaticContents | WNoAutoErase ) 
{
	QFont f ("fixed");
	f.setFixedPitch(true);
	f.setStyleHint(QFont::TypeWriter);
	myFont = f;
	setFont(f);
	standard = new QFontMetrics( myFont );
	f.setBold( true );
	standardBold = new QFontMetrics( f );
	f.setItalic( true );
	standardBoldItalic = new QFontMetrics( f );
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
	delete standard;
	delete standardBold;
	delete standardBoldItalic;
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
	
	//check if the line contains TABs
	QString line = mParent->myBuffer()->textline( mCursorY + mParent->getCurrentTop() );
	uint startcol = 0;
	while ( !mParent->isColumnVisible(startcol++, l) && startcol <= line.length())
		;
	startcol--;
	//new position of cursor
	mCursorX = c - startcol;
	mCursorY = l - mParent->getCurrentTop();
	QString currentLine = mParent->myBuffer()->textline( mCursorY + mParent->getCurrentTop() ).mid( startcol, mParent->getColumnsVisible() );

	mCursorX = 0;
	for ( uint tmp = 0; tmp < c - startcol; tmp++ )
		mCursorX +=  currentLine[ tmp ] == tabChar ? 8 : 1 ;
	
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
			//FIXME
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

void KYZisEdit::drawContents(QPainter *p, int , int clipy, int , int cliph) {
	int flag = ( mParent->myBuffer()->introShown() ? Qt::AlignCenter : Qt::AlignLeft )| Qt::AlignVCenter | Qt::ExpandTabs | Qt::SingleLine;

	for ( uint line = mParent->getCurrentTop(); line < mParent->getCurrentTop() + mParent->getLinesVisible() ; line++ ) {
		uint i = line - mParent->getCurrentTop(); //relative line number
		if ( fontMetrics().lineSpacing() * i >= (  unsigned int )clipy && fontMetrics().lineSpacing() * i < (  unsigned int ) (  clipy+cliph ) ) {
			QRect clip(0, i * fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing());
			p->eraseRect(clip);
			QString toDraw = mParent->myBuffer()->textline( line );
			if (mParent->myBuffer()->lineCount() > line && !toDraw.isNull() && !toDraw.isEmpty()) {
				uint startcol = 0;
				while ( !mParent->isColumnVisible(startcol++, line) && startcol <= toDraw.length())
					;
				startcol--;
				toDraw = toDraw.mid( startcol );

				int currentX = 0;
				const uchar* a = NULL;
				YZLine *yl = mParent->myBuffer()->yzline( line );
				if ( yl->length() != 0 ) a = yl->attributes();
				YzisAttribute *at = 0L;
				YzisHighlighting *highlight = mParent->myBuffer()->highlight();
				if ( highlight )
					at = highlight->attributes( 0 /*only one schema*/ )->data( );
				uint atLen = at ? highlight->attributes( 0 /*only one schema*/ )->size() : 0;
				bool noAttribs = !a;
				a = a + startcol;
				for ( uint tmp = 0; tmp < toDraw.length(); tmp++ ) {
					YzisAttribute hl;
					YzisAttribute *curAt = ( !noAttribs && (*a) >= atLen ) ?  &at[ 0 ] : &at[*a];
					if ( curAt ) {
						hl+=*curAt;
						p->setPen(hl.textColor());
						//p->setFont(hl.font(myFont));
					}
					if ( toDraw[ tmp ] == tabChar ) {
						QString spaces = "        ";
						QRect clip3(currentX, ( i ) * fontMetrics().lineSpacing(), fontMetrics().maxWidth(), fontMetrics().lineSpacing());
						p->drawText(clip3,flag,spaces);
						currentX += fontMetrics().maxWidth() * 8;
					} else {
						QRect clip2(currentX, ( i ) * fontMetrics().lineSpacing(), fontMetrics().maxWidth(), fontMetrics().lineSpacing());
						p->drawText(clip2,flag,toDraw.mid( tmp,1 ));
						currentX+=fontMetrics().maxWidth();
					}
					a++;
				}
			} else if (mParent->myBuffer()->lineCount() <= line) {
				p->drawText(clip,flag ,"~");
			}
			if ( mCursorShown && mCursorY + mParent->getCurrentTop() == line) {
				setCursor( mParent->getCursor()->getX(), mCursorY + mParent->getCurrentTop() );
			}
		}
	}
}

void KYZisEdit::focusInEvent ( QFocusEvent * ) {
	KYZisFactory::s_self->setCurrentView( mParent );
}

#include "editor.moc"
