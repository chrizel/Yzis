#ifndef YZ_EX_EXECUTOR
#define YZ_EX_EXECUTOR

#include "view.h"

class YZView;

/**
 * Entry point to execute Ex functions
 * This is TEMPORARY until we design Ex support properly
 */
class YZExExecutor {
	public:
		YZExExecutor();
		~YZExExecutor();

		QString write( YZView *view, const QString& inputs );
		QString buffernext( YZView *view, const QString& inputs );
		QString edit( YZView *view, const QString& inputs );
		QString quit( YZView *view, const QString& inputs );
	
};

#endif
