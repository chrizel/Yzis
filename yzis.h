#ifndef YZIS_H
#define YZIS_H
/**
 * yzis.h
 *
 * Main include file for the yzis project
 *
 */


#include <stdio.h>
#include <stdlib.h>

#define VERSION_MAJOR	0
#define VERSION_MINOR	0
#define VERSION_PATCH	1

#define VERSION_CHAR	"0.0.1"

struct yzpoint {
	int x;
	int y;
};

typedef struct yzpoint yz_point;

namespace YZIS {
	enum YZKeys {
		Shift = 1,
		Alt = 2,
		Ctrl = 4
	};
}

#endif /* YZIS_H */
