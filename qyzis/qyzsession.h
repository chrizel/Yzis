
#include <qvbox.h>

#include <libyzis/session.h>

class QWidgetStack;

class QYZSession: public YZSession, public QVBox {
public:
	QYZSession();
	virtual ~QYZSession();

	// -------------	View & Buffer ----------------
	virtual void changeCurrentView( YZView* );
	virtual void deleteView ( int Id = -1 );
	virtual void deleteBuffer( YZBuffer *b );
	virtual	YZBuffer *createBuffer(const QString& path=QString::null);
	virtual YZView* createView ( YZBuffer* );

	// -------------  Graphical stuff -----------------
	virtual void setFocusCommandLine();
	virtual void setFocusMainWindow();
	virtual void splitHorizontally ( YZView* );

	// --------------  Messages ------------------
	virtual void popupMessage( const QString& message );
	virtual bool promptYesNo(const QString& title, const QString& message);
	virtual int promptYesNoCancel(const QString& title, const QString& message);

	virtual void quit(int errorCode=0);

protected:
	QWidgetStack * mViewStack;
};
