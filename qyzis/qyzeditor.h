#ifndef QYZEDITOR_H
#define QYZEDITOR_H

#include <qwidget.h>

class QYZView;

class QYZEditor : public QWidget 
{
public:
	QYZEditor( QYZView * parent );
	void paintEvent( unsigned int curx, unsigned int cury, 
		unsigned int curw, unsigned int curh );

protected:
	void paintEvent ( QPaintEvent * e); 
	bool event(QEvent *e);
	void drawContents( unsigned int x, unsigned int y, 
			unsigned int w, unsigned int h );

	QYZView * mParent;
	int mCharWidth, mCharHeight;
};

#endif // QYZEDITOR_H
