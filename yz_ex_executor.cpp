#include "yz_ex_executor.h"
#include "yz_debug.h"

YZExExecutor::YZExExecutor() {
}

YZExExecutor::~YZExExecutor() {
}

QString YZExExecutor::write( YZView *view, const QString& inputs ) {
	view->myBuffer()->save();
	yzDebug() << "File saved" << endl;
	return QString::null;
}

QString YZExExecutor::buffernext( YZView *view, const QString& inputs ) {
	yzDebug() << "Switching buffers ..." << endl;
	view->mySession()->setCurrentBuffer(view->mySession()->nextBuffer());
	return QString::null;
}
