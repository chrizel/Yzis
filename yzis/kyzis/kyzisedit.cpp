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
	//strange we should not need a -1 //FIXME
	//undraw previous cursor
	updateContents(cursorx*fontMetrics().maxWidth(),cursory*fontMetrics().lineSpacing(),width(),fontMetrics().lineSpacing()*2);
	cursorx = c;
	cursory = l - 1;
	//draw new cursor
	updateContents(cursorx*fontMetrics().maxWidth(),cursory*fontMetrics().lineSpacing(),width(),fontMetrics().lineSpacing()*2);
}

void KYZisEdit::setTextLine(int l, const QString &str){
	mText.insert(l,str);
	updateContents(0,l * fontMetrics().lineSpacing(),width(),fontMetrics().lineSpacing());
}

// INTERNAL API
void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	kdDebug()<< " Got key : " << e->ascii() << endl;
	_parent->send_char(e->ascii());
	e->accept();
}

void KYZisEdit::contentsMousePressEvent ( QMouseEvent * e ) {
	//FIXME needs improvment (add some offset)
	//relative coordinates ?
	_parent->update_cursor(e->x()/fontMetrics().maxWidth(), e->y()/fontMetrics().lineSpacing());
}

void KYZisEdit::drawCursorAt(int x, int y) {
	bitBlt (
			viewport(),
			x*fontMetrics().maxWidth(),y*fontMetrics().lineSpacing(),
			viewport(),
			x*fontMetrics().maxWidth(),y*fontMetrics().lineSpacing(),
			fontMetrics().maxWidth(), fontMetrics().lineSpacing(),
			Qt::NotROP,	    // raster Operation
			true );		    // ignoreMask
}

void KYZisEdit::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph) {
	//XXX draw text inside the clip
	kdDebug() << "*** DrawContents : clipx " << clipx << " clipy " << clipy << " clipw " << clipw << " cliph " << cliph << endl;
	KYZLine::iterator it;
	for (it = mText.begin(); it!=mText.end(); ++it) {
			if (fontMetrics().lineSpacing() * it.key() >= clipy && fontMetrics().lineSpacing() * it.key() <= clipy+cliph ) {
				p->eraseRect(0,it.key() * fontMetrics().lineSpacing(), width(), fontMetrics().lineSpacing());
				p->drawText(0,it.key() * fontMetrics().lineSpacing(),it.data());
			}
	}
	
	//XXX draw the cursor if needed
	drawCursorAt(cursorx,cursory);
}

#include "kyzisedit.moc"
