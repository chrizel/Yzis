
#include "qyzbuffer.h"

QYZBuffer::QYZBuffer( YZSession * sess)
	: YZBuffer(sess)
{
	
}

QYZBuffer::~QYZBuffer()
{

}

bool QYZBuffer::popupFileSaveAs()
{
	// called when yzis is saving all the modified buffers
	// but no name has been defined ?
	return false;
}

void QYZBuffer::filenameChanged()
{
	// The name of the file name has changed, you should tell all the views
	// so that if any is displaying the filename, it should update it.
	// XXX: could be moved to libyzis
}

void QYZBuffer::highlightingChanged()
{
	// No need to put anything here, this is used only by kyzis
}


