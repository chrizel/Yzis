#include "ex_executor.h"
#include "debug.h"

YZExExecutor::YZExExecutor() {
}

YZExExecutor::~YZExExecutor() {
}

QString YZExExecutor::write( YZView *view, const QString& inputs ) {
	QRegExp rx ( "^(\\d*)(\\S+) (.*)$");
	rx.search( inputs );
	if ( rx.cap(3) != QString::null ) view->myBuffer()->setPath(rx.cap(3));
	view->myBuffer()->save();
	yzDebug() << "File saved as " << view->myBuffer()->fileName() << endl;
	return QString::null;
}

QString YZExExecutor::buffernext( YZView *view, const QString& ) {
	yzDebug() << "Switching buffers (actually sw views) ..." << endl;
	view->mySession()->setCurrentView(view->mySession()->nextView());
	return QString::null;
}

QString YZExExecutor::quit ( YZView *view, const QString& ) {
	view->mySession()->mGUI->quit();
	return QString::null;
}

QString YZExExecutor::edit ( YZView *view, const QString& inputs ) {
	int idx = inputs.find(" ");
	if ( idx == -1 ) return QString::null; //XXX display error : "No filename given"
	QString path = inputs.mid( idx + 1 ); //extract the path 
	yzDebug() << "New buffer / view : " << path << endl;
	YZBuffer *b = view->mySession()->mGUI->createBuffer( path );
	//TODO
//	YZView* v = view->mySession()->mGUI->createView(b);
//	view->mySession()->mGUI->setCurrentView(v);
	return QString::null;
}
