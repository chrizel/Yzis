
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qwidgetstack.h>
#include <qvbox.h>
#include <qpushbutton.h>

#include "qyzsession.h"
#include "qyzview.h"
#include "qyzbuffer.h"

QYZSession::QYZSession()
	: YZSession(), QVBox()
{
	new QPushButton("Before Stack", this );
	mViewStack = new QWidgetStack( this );
	mViewStack->show();
	new QPushButton("After Stack", this );
	mCommandLine = new QLineEdit( this );
    setFont ( YZSession::mOptions.readQStringEntry("Font", font().toString()) );
	resize( 80 * 10, 50 * 10 );
}

QYZSession::~QYZSession()
{
}

YZBuffer * QYZSession::createBuffer(const QString& path)
{
	QYZBuffer * buf = new QYZBuffer( YZSession::me );
	setCurrentView( createView( buf ) );
	buf->load( path );
	currentView()->refreshScreen();
	return buf;
}

YZView * QYZSession::createView ( YZBuffer * buf )
{
	int id;
	QYZView * view = new QYZView( buf, YZSession::me );
	id = mViewStack->addWidget( view );
	qDebug("QYZSession - createView id %d", id );
	qDebug("QYZSession - id of view =  %d", mViewStack->id( view ) );
	buf->addView( view );
	return view;
}

void QYZSession::setFocusCommandLine()
{
	mCommandLine->setFocus();
}

void QYZSession::setFocusMainWindow()
{
	QYZView * qyzview = static_cast<QYZView *>( currentView() );
	mViewStack->setFocus();
}

void QYZSession::changeCurrentView( YZView * view )
{
	qDebug("QYZSession - changeCurrentView %p", view );
	// set the current view in the view shell
	QYZView * qyzview = static_cast<QYZView *>(view);
	qDebug("QYZSession - changeCurrentView %p", qyzview );
	qDebug("QYZSession - raising view %d", mViewStack->id( qyzview ) );
	qDebug("QYZSession - view visible %d", qyzview->isVisible() );
	mViewStack->raiseWidget( qyzview );
	qyzview->setFocus();
}

void QYZSession::deleteView ( int Id )
{
	// delete the given view
	// but how to map id to view ?
}

void QYZSession::deleteBuffer( YZBuffer *b )
{

}

void QYZSession::quit(int errorCode)
{
	qApp->closeAllWindows();
	qApp->exit( errorCode );
}

void QYZSession::popupMessage( const QString& message )
{
	QMessageBox::information( 0L, "QYZis information", message, QMessageBox::Ok);
}

bool QYZSession::promptYesNo(const QString& title, const QString& message)
{
	int ret = QMessageBox::question( 0L, title, message, QMessageBox::Yes, QMessageBox::No);
	return ret == QMessageBox::Yes;
}

int QYZSession::promptYesNoCancel(const QString& title, const QString& message)
{
	int ret = QMessageBox::question( 0L, title, message, QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
	if (ret == QMessageBox::Yes) return 0;
	else if (ret == QMessageBox::No) return 1;
	return 2;
}

void QYZSession::splitHorizontally ( YZView* )
{
}




