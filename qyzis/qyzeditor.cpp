
#include <qpainter.h>
#include <qpushbutton.h>

#include "qyzeditor.h"
#include "qyzview.h"
#include "qyzbuffer.h"

QYZEditor::QYZEditor( QYZView * parent )
: QWidget( parent )
{
	QPushButton("This is the editor", this );
	mCharWidth = fontMetrics().maxWidth();
	mCharHeight = fontMetrics().height();
	mParent = parent;
}

void QYZEditor::paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh )
{
	qDebug("QYZView - paintEvent %d,%d - %d,%d", curx, cury, curw, curh );
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
	int i;
	bool wrap = mParent->getLocalBoolOption( "wrap" );
	bool rightleft = mParent->getLocalBoolOption( "rightleft" );
	bool number = mParent->getLocalBoolOption( "number" );

	int my_marginLeft=0, pixMarginLeft=5;
	unsigned int lineCount = mParent->myBuffer()->lineCount();
	if ( number ) { // update marginLeft
		my_marginLeft = 2 + QString::number( lineCount ).length();
	}

	QPainter p(this);
	p.setPen( Qt::black );
	p.fillRect( 0, 0, 100, 100, QBrush( Qt::black ) );

	// XXX handle color and attributes
	
	for( i=line; i<=line+h; i++) {
		if (number) {
			p.drawText(pixMarginLeft, line*mCharHeight, QString::number( i ) );
		}
		p.drawText( pixMarginLeft + my_marginLeft*mCharWidth, 
			line*mCharWidth, mParent->myBuffer()->textline( i ) );
		qDebug("drawing line %d %s", i, mParent->myBuffer()->textline(i).latin1() );
	}

	// XXX draw cursor
}

