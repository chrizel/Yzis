
#include "qyzsession.h"

QYZSession::QYZSession()
	: YZSession()
{

}

QYZSession::~QYZSession()
{
}

void QYZSession::changeCurrentView( YZView* )
{
		// set the current view
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

}
void QYZSession::popupMessage( const QString& message )
{

}
bool QYZSession::promptYesNo(const QString& title, const QString& message)
{
	return false;
}

int QYZSession::promptYesNoCancel(const QString& title, const QString& message)
{
	return 0;
}

YZBuffer * QYZSession::createBuffer(const QString& path)
{
	return NULL;
}

YZView * QYZSession::createView ( YZBuffer* )
{
	return NULL;
}

void QYZSession::setFocusCommandLine()
{

}
void QYZSession::setFocusMainWindow()
{

}

