/**
 * $Id$
 */

#include "kyzisedit.h"
#include <kdebug.h>

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
	: QScrollView( parent, name,WStaticContents | WRepaintNoErase | WResizeNoErase ) {
	setFont(QFont("Fixed",10));
	_parent = parent;

	viewport()->setFocusProxy( this );
	viewport()->setFocusPolicy( StrongFocus );
	viewport()->setBackgroundMode( PaletteBase );
	viewport()->setPaletteBackgroundColor(QColor("white"));
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

void KYZisEdit::setTextLine(int l, const QString &str){
	//mText.insert(l,str);
	updateContents( 0, ( l - _parent->getCurrent() ) * fontMetrics().lineSpacing(),
								width(), fontMetrics().lineSpacing()  );
}

// INTERNAL API
void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	kdDebug()<< " Got key : " << e->key()<< " Got ASCII : " << e->ascii()<< " Got Unicode : " << e->text() << endl;
	if ( e->ascii() != 0 ) {
		_parent->sendChar(e->ascii());
		e->accept();
	}
}

void KYZisEdit::contentsMousePressEvent ( QMouseEvent * e ) {
	_parent->updateCursor(e->x()/fontMetrics().maxWidth() , e->y()/fontMetrics().lineSpacing() + _parent->getCurrent());
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
	int i=0;
	int current = _parent->getCurrent();
	for ( i=current; i < current + _parent->getLinesVisible() ; ++i ) {
			if (fontMetrics().lineSpacing() * ( i-current ) >= clipy &&
					fontMetrics().lineSpacing() * ( i-current ) <= clipy+cliph ) {
					QRect clip(0, ( i-current ) * fontMetrics().lineSpacing(), width(),fontMetrics().lineSpacing());
					p->eraseRect(clip);
					p->drawText(clip,Qt::AlignLeft|Qt::DontClip|Qt::SingleLine ,_parent->myBuffer()->getText()[ i ]);
			}
	}

	drawCursorAt(cursorx,cursory);
}

#include "kyzisedit.moc"
