#include "ex_executor.h"
#include "debug.h"

YZExExecutor::YZExExecutor() {
}

YZExExecutor::~YZExExecutor() {
}

QString YZExExecutor::write( YZView *view, const QString& inputs ) {
	//XXX needs support for file name
	view->myBuffer()->save();
	yzDebug() << "File saved" << endl;
	return QString::null;
}

QString YZExExecutor::buffernext( YZView *view, const QString& inputs ) {
	yzDebug() << "Switching buffers ..." << endl;
	view->mySession()->setCurrentBuffer(view->mySession()->nextBuffer());
	return QString::null;
}

QString YZExExecutor::quit ( YZView *view, const QString& inputs ) {
	view->mySession()->gui_manager->quit();
}

QString YZExExecutor::edit ( YZView *view, const QString& inputs ) {
	yzDebug() << "New buffer ..." << endl;
	int idx = inputs.find(" ");
	if ( idx == -1 ) return QString::null; //XXX display error : "No filename given"
	QString path = inputs.mid( idx ); //extract the path 
	view->mySession()->createBuffer( path );
	return QString::null;
}
