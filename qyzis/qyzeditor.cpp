
#include <qpainter.h>
#include <qpushbutton.h>

#include "qyzeditor.h"
#include "qyzview.h"
#include "qyzbuffer.h"

QYZEditor::QYZEditor( QYZView * parent )
: QWidget( parent )
{
	setFocusPolicy( QWidget::StrongFocus );
	mCharWidth = fontMetrics().maxWidth();
	mCharHeight = fontMetrics().height();
	mParent = parent;
}

void QYZEditor::paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh )
{
	qDebug("QYZEditor - Yzis paintEvent %d,%d - %d,%d", curx, cury, curw, curh );
	int col, line, w, h;
	// repaint the area (curx, cury, curx+curw, cury+curh)
	line = (cury / mCharHeight);
	col = (curx / mCharWidth);
	h = (curh / mCharHeight)+1;
	w = (curw / mCharWidth)+1;
	drawContents( col, line, w, h );
}

void QYZEditor::drawContents( unsigned int , unsigned int line, 
			unsigned int , unsigned int h )
{
	qDebug("QYZView - drawContents line %d -> %d", line, line+h );
	int i, my_y;
	QString s;
	bool wrap = mParent->getLocalBooleanOption( "wrap" );
	bool rightleft = mParent->getLocalBooleanOption( "rightleft" );
	bool number = mParent->getLocalBooleanOption( "number" );

	int my_marginLeft=0, pixMarginLeft=5;
	unsigned int lineCount = mParent->myBuffer()->lineCount();
	if ( number ) { // update marginLeft
		my_marginLeft = 2 + QString::number( lineCount ).length();
	}

	QPainter p(this);
	p.setPen( Qt::black );
//	p.fillRect( 0, 0, 100, 100, QBrush( Qt::black ) );

	// XXX handle color and attributes
	
	for( i=line; i<=line+h; i++) {
		s = mParent->myBuffer()->textline(i).latin1();
		my_y = i*mCharHeight;
		if (number) {
			p.drawText(pixMarginLeft, my_y , QString::number( i ) );
		}
		p.drawText( pixMarginLeft + my_marginLeft*mCharWidth, my_y, s );
		qDebug("drawing line %d at %d: %s", i, my_y, s.latin1() );
	}

	// XXX draw cursor
}

bool QYZEditor::event(QEvent *e)
{
//	qDebug("QYZEditor - event");
	return QWidget::event( e );
}

void QYZEditor::paintEvent ( QPaintEvent * pe)
{
	qDebug("QYZEditor - Qt paintEvent %dx%d - %dx%d", pe->rect().x(), pe->rect().y(),
	pe->rect().width(), pe->rect().height() );
	paintEvent( pe->rect().x(), pe->rect().y(), pe->rect().width(), pe->rect().height() );
}
