
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
	// XXXphil: could be moved to libyzis, by providing the method
	// filenameChanged directly on the view
}

void QYZBuffer::highlightingChanged()
{
	// XXXphil: this method does not need to be pure virtual
}


