
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

}
void QYZSession::deleteView ( int Id )
{

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

