
#include <libyzis/session.h>

class QYZSession: public YZSession {
public:
	QYZSession();
	virtual ~QYZSession();

	virtual void changeCurrentView( YZView* );
	virtual void deleteView ( int Id = -1 );
	virtual void deleteBuffer( YZBuffer *b );
	virtual void quit(int errorCode=0);
	virtual void popupMessage( const QString& message );
	virtual bool promptYesNo(const QString& title, const QString& message);
	virtual int promptYesNoCancel(const QString& title, const QString& message);
	virtual	YZBuffer *createBuffer(const QString& path=QString::null);
	virtual YZView* createView ( YZBuffer* );
	virtual void setFocusCommandLine();
	virtual void setFocusMainWindow();
	virtual void splitHorizontally ( YZView* );
};
