#include <qvbox.h>

#include "libyzis/view.h"


class YZBuffer;
class YZSession;
class QYZEditor;
class QLineEdit;
class KeyConverter;

class QYZView: public YZView, public QVBox {
public:
	QYZView(YZBuffer *_b, YZSession *sess);
	virtual ~QYZView();

	virtual QString getCommandLineText() const;
	virtual void refreshScreen();
	virtual void setCommandLineText( const QString& );
	virtual void modeChanged();
	virtual void displayInfo( const QString& info );
	virtual void syncViewInfo();
	virtual void scrollUp( int );
	virtual void scrollDown( int );
	virtual unsigned int stringWidth( const QString& str ) const;
	virtual unsigned int charWidth( const QChar& ch ) const;
	virtual void registerModifierKeys( const QString& keys );
	virtual void paintEvent( unsigned int curx, unsigned int cury, 
		unsigned int curw, unsigned int curh );

protected:
	void resizeEvent ( QResizeEvent * e);
	void wheelEvent( QWheelEvent * e );
	bool event(QEvent *e);
	void keyPressEvent ( QKeyEvent * e );

	KeyConverter * mKeyConverter;
	QLineEdit *    mCommandLine;
	QLineEdit *    mStatusBar;
	QYZEditor *    mEditor;	
};
