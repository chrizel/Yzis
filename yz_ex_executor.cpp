#include "yz_ex_executor.h"

YZExExecutor::YZExExecutor() {

}

YZExExecutor::~YZExExecutor() {

}

QString YZExExecutor::write( YZView *view, const QString& inputs ) {
	view->myBuffer()->save();
	printf( "File saved\n" );
	return QString::null;
}

