#ifndef YZ_EX_EXECUTOR
#define YZ_EX_EXECUTOR

#include "yz_view.h"

/**
 * Entry point to execute Ex functions
 * This is TEMPORARY until we designed Ex support properly
 */

class YZView;

class YZExExecutor {
	public:
		YZExExecutor();
		~YZExExecutor();

		QString write( YZView *view, const QString& inputs );
	
};

#endif
