#include "libyzis/view.h"

class YZBuffer;
class YZSession;

class QYZView: public YZView {
public:
	QYZView(YZBuffer *_b, YZSession *sess, int lines);
	virtual ~QYZView();

	virtual QString getCommandLineText() const;
	virtual void refreshScreen();
	virtual void setCommandLineText( const QString& );
	virtual void paintEvent( unsigned int curx, unsigned int cury, unsigned int curw, unsigned int curh );
	virtual void modeChanged();
	virtual void displayInfo( const QString& info );
	virtual void syncViewInfo();
	virtual void scrollUp( int );
	virtual void scrollDown( int );
	virtual unsigned int stringWidth( const QString& str ) const;
	virtual unsigned int charWidth( const QChar& ch ) const;
	virtual void registerModifierKeys( const QString& keys );

};
