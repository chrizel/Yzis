/**
 * $id$
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
}

KYZisEdit::~KYZisEdit() {
}

void KYZisEdit::viewportResizeEvent(QResizeEvent *ev) {
	QSize s = ev->size();
	int lines=0;
	lines = s.height() / fontMetrics().lineSpacing();
	_parent->setVisibleLines( lines );
	kdDebug()<<"Update visibles lines to : "<< lines << endl;
}

// PUBLIC API
void KYZisEdit::setCursor(int c, int l) {
	//undraw previous cursor
	kdDebug() << "CursorX before : " << cursorx << " CursorY before : " << cursory << endl;
	drawCursorAt( cursorx,cursory );
	cursorx = c;
	cursory = l;
	kdDebug() << "CursorX after : " << cursorx << " CursorY after : " << cursory << endl;
	//draw new cursor
	drawCursorAt( cursorx,cursory );
}

void KYZisEdit::setTextLine(int l, const QString &str){
	mText.insert(l,str);
	updateContents( 0, ( l-1 ) * fontMetrics().lineSpacing(),
								width(), fontMetrics().lineSpacing() * 2 //yes i know, it's a 2, i don't think it will really affect 
																													// our performances
								);
}

// INTERNAL API
void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	kdDebug()<< " Got key : " << e->ascii() << endl;
	_parent->sendChar(e->ascii());
	e->accept();
}

void KYZisEdit::contentsMousePressEvent ( QMouseEvent * e ) {
	//FIXME needs improvment (add some offset)
	//relative coordinates ?
	_parent->updateCursor(e->x()/fontMetrics().maxWidth(), e->y()/fontMetrics().lineSpacing());
}

void KYZisEdit::drawCursorAt(int x, int y) {
	bitBlt (
			viewport(),
			x*fontMetrics().maxWidth(),( y /*-1*/ )*fontMetrics().lineSpacing(),
			viewport(),
			x*fontMetrics().maxWidth(),( y /*-1*/ )*fontMetrics().lineSpacing(),
			fontMetrics().maxWidth(), fontMetrics().lineSpacing(),
			Qt::NotROP,	    // raster Operation
			true );		    // ignoreMask
}

void KYZisEdit::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph) {
	//XXX draw text inside the clip
	kdDebug() << "*** DrawContents : clipx " << clipx << " clipy " << clipy << " clipw " << clipw << " cliph " << cliph << endl;
	KYZLine::iterator it;
	for (it = mText.begin(); it!=mText.end(); ++it) {
			if (fontMetrics().lineSpacing() * ( it.key() + 1 ) >= clipy &&
					fontMetrics().lineSpacing() * ( it.key() + 1 ) <= clipy+cliph ) {
				kdDebug() << "DRAW at " << it.key() << endl;
				p->eraseRect(0,( it.key()+1 )*fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing());
				p->drawText(0,( it.key()+1 )*fontMetrics().lineSpacing(),it.data());
			}
	}
	
	//XXX draw the cursor if needed
	drawCursorAt(cursorx,cursory);
}

#include "kyzisedit.moc"
