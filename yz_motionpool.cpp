#include "yz_motionpool.h"

YZMotionPool::YZMotionPool(){
}

YZMotionPool::~YZMotionPool() {
}

void YZMotionPool::initPool() {
}

void YZMotionPool::addMotion(const YZMotion& regexp, const QString& key){
	pool.insert( key,regexp );
}
