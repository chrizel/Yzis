
#include "qyzview.h"

QYZView::QYZView(YZBuffer *_b, YZSession *sess, int lines)
	: YZView( _b, sess, lines )
{

}

QYZView::~QYZView()
{
}

QString QYZView::getCommandLineText() const
{
	return "";
}

void QYZView::setCommandLineText( const QString& )
{

}

void QYZView::paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh )
{

}

void QYZView::modeChanged()
{

}

void QYZView::displayInfo( const QString& info )
{

}

void QYZView::syncViewInfo()
{

}

void QYZView::scrollUp( int )
{

}

void QYZView::scrollDown( int )
{

}

uint QYZView::stringWidth( const QString& str ) const
{
	return 0;
}

uint QYZView::charWidth( const QChar& ch ) const
{
	return 0;
}

void QYZView::registerModifierKeys( const QString& keys )
{

}

void QYZView::refreshScreen()
{

}


