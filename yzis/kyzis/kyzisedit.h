#ifndef KYZISEDIT_H
#define KYZISEDIT_H

#include "kyzisview.h"
#include <qscrollview.h>
#include <qpainter.h>
#include <qevent.h>
#include <qmap.h>

typedef QMap<int,QString> KYZLine;
class KYZisView;

/**
 * KYZis Painter Widget
 */
class KYZisEdit : public QScrollView {
	Q_OBJECT

	public :
		KYZisEdit(KYZisView *parent=0, const char *name=0);
		virtual ~KYZisEdit();

		//erase all text, and set new text
		void setText (const QString);

		//append text
		void append ( const QString );

		//set text at line ...
		void setTextLine(int l, const QString &);
		
		//move cursor to position column, line relative to viewport
		void setCursor(int c,int l);

	protected:
		//update view when the viewport gets resized
		void viewportResizeEvent(QResizeEvent*);

		//entry point for drawing events
		void drawContents(QPainter *p, int clipx, int clipy, int clipw, int cliph );

		//normal keypressEvents processing
		void keyPressEvent (QKeyEvent *);

		//mouse events
		void mousePressEvent (QMouseEvent *);

		//insert text at line
		void insetTextAt(const QString, int line);

		//insert a char at idx on line ....
		void insertCharAt(QChar,int);
		
		//replace a char at idx on line ....
		void replaceCharAt( QChar,int );
		
		//number of lines
		long lines();
		
		//draw the cursor at the given position
		void drawCursorAt(QPainter *paint,int x, int y);

	private :
		KYZisView *_parent;

		//cursor position (sync with libyzis one)
		int cursorx;
		int cursory;

		//QMap<int,QString> mText;
		KYZLine mText;
};

#endif
