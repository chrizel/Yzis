
#include "qyzview.h"

QYZView::QYZView(YZBuffer *_b, YZSession *sess, int lines)
	: YZView( _b, sess, lines )
{

}

QYZView::~QYZView()
{
}

void QYZView::setCommandLineText( const QString& )
{
	// set the text of the command line (the place where the user types
	// when he presses :
}

QString QYZView::getCommandLineText() const
{
	// get the text previously set by setCommandLineText
	return "";
}

void QYZView::paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh )
{
	// repaint the area (curx, cury, curx+curw, cury+curh)
}

void QYZView::modeChanged()
{
	// mode has been changed: insert, replace, ex, ...
}

void QYZView::displayInfo( const QString& info )
{
	// information to be displayed in the status bar
}

void QYZView::syncViewInfo()
{
	// ???
}

void QYZView::scrollUp( int )
{
	// scroll up the content of the view by n lines
	// use it to optimise redrawing
}

void QYZView::scrollDown( int )
{
	// scroll down the content of the view by n lines
	// use it to optimise redrawing
}

uint QYZView::stringWidth( const QString& str ) const
{
	// width of the string using the current font ??
	// why do we need it in libyzis ? We are supposed to only manipulate 
	// lines and columns
	return 0;
}

uint QYZView::charWidth( const QChar& ch ) const
{
	// width of the string using the current font ??
	// why do we need it in libyzis ? We are supposed to only manipulate 
	// lines and columns
	return 0;
}

void QYZView::registerModifierKeys( const QString& keys )
{
	// ???
}

void QYZView::refreshScreen()
{
	// repaint the current view
}


