
#include "qyzsession.h"

QYZSession::QYZSession()
	: YZSession()
{
	// Init QYzis globally, create the view shell
}

QYZSession::~QYZSession()
{
}

void QYZSession::changeCurrentView( YZView* )
{
		// set the current view in the view shell
}

void QYZSession::deleteView ( int Id )
{
		// delete the given view
}

void QYZSession::deleteBuffer( YZBuffer *b )
{

}
void QYZSession::quit(int errorCode)
{
	// quit the application
}
void QYZSession::popupMessage( const QString& message )
{
	// popup a message box
}

bool QYZSession::promptYesNo(const QString& title, const QString& message)
{
	// self explanatory
	return false;
}

int QYZSession::promptYesNoCancel(const QString& title, const QString& message)
{
	// self explanatory
	return 0;
}

YZBuffer * QYZSession::createBuffer(const QString& path)
{
	// create a new yzbuffer
	// create a new view for this buffer
	// setCurrentView( createView( b ) );
	// set the view as the current view
	// load the file in the buffer
	// b->load( filename );
	// currentView->refreshScreen();
}

YZView * QYZSession::createView ( YZBuffer* )
{
	// create a view
	// add it to the buffer
	// buffer->addView ( v);
	// return it
	return NULL;
}

void QYZSession::setFocusCommandLine()
{

}
void QYZSession::setFocusMainWindow()
{

}

void QYZSession::splitHorizontally ( YZView* )
{
}
