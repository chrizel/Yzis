
#include <libyzis/buffer.h>

class YZSession;

class QYZBuffer: public YZBuffer {
public:
	QYZBuffer(YZSession *sess);
	virtual ~QYZBuffer();

	virtual bool popupFileSaveAs();
	virtual void filenameChanged();
	virtual void highlightingChanged();
};


