#ifndef YZ_MOTIONPOOL
#define YZ_MOTIONPOOL

#include "motion.h"
#include <qmap.h>

/**
 * This is the main place for handling motion objects 
 * This is used for operators commands like "d".
 * The goal is to handle ocmmands like : "2d3w" to delete twice 3 words 
 * Basics: 
 * We use RegExps to define the motion objects ( but this can be extended ),
 * each object has an identifier key like "w" for words, "p" for paragraphs
 * etc...
 */
class YZMotionPool {
	public:
		YZMotionPool();
		~YZMotionPool();

		//adds a new motion to the pool of known motions :)
		void addMotion(const YZMotion& regexp, const QString& key);

		void initPool();

	private:
		QMap<QString,YZMotion> pool;
};

#endif
