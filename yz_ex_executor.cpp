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

