#include "kyzisedit.h"
#include <kdebug.h>

KYZisEdit::KYZisEdit(KYZisView *parent, const char *name)
	: QScrollView( parent, name,WStaticContents | WRepaintNoErase | WResizeNoErase ) {
	_parent = parent;

	viewport()->setFocusProxy( this );
	viewport()->setFocusPolicy( StrongFocus );
}

KYZisEdit::~KYZisEdit() {
}


// PUBLIC API
void KYZisEdit::setCursor(int c, int l) {
	cursorx = c;
	cursory = l;
}

void KYZisEdit::setTextLine(int l, const QString &str){
	drawText(0,l * fontMetrics().lineSpacing(),str);
}

// INTERNAL API
void KYZisEdit::keyPressEvent ( QKeyEvent * e ) {
	kdDebug()<< " Got key : " << e->ascii() << endl;
	_parent->send_char(e->ascii());
	e->accept();
}

void KYZisEdit::drawCursorAt(QPainter *paint, int x, int y) {
	QBrush brush(blue);
	paint->setBrush(brush);
	paint->setPen(NoPen);
	paint->drawRect(x,y,100,100);
}

void KYZisEdit::drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph) {
	p->drawText(0,1 * fontMetrics().lineSpacing(),"Test");
}

#include "kyzisedit.moc"
