
#include <qlineedit.h>
#include <qpushbutton.h>

#include "qyzsession.h"
#include "keyConverter.h"
#include "qyzview.h"
#include "qyzeditor.h"


QYZView::QYZView(YZBuffer *_b, YZSession *sess)
	: YZView( _b, sess, 50 ), QVBox( static_cast<QYZSession*>(sess) )
{
	mKeyConverter = new KeyConverter();
	QPushButton( "Before Editor", this );
	mEditor = new QYZEditor( this );
	QPushButton( "After Editor", this );
	mStatusBar = new QLineEdit( this );
	mCommandLine = new QLineEdit( this );
	setVisibleArea( 80, 50 );
}

QYZView::~QYZView()
{
	delete mKeyConverter;
	mKeyConverter = 0L;
}

void QYZView::setCommandLineText( const QString& s )
{
	// set the text of the command line (the place where the user types
	// when he presses :
	mCommandLine->setText(s);
}

QString QYZView::getCommandLineText() const
{
	// get the text previously set by setCommandLineText
	return mCommandLine->text();
}

void QYZView::modeChanged()
{
	qDebug("QYZView - modeChanged");
	// mode has been changed: insert, replace, ex, ...
}

void QYZView::displayInfo( const QString& info )
{
	mStatusBar->setText( info );
}

void QYZView::syncViewInfo()
{
	qDebug("QYZView - syncViewInfo");
	// ???
}

void QYZView::scrollUp( int )
{
	// scroll up the content of the view by n lines
	// use it to optimise redrawing
}

void QYZView::scrollDown( int )
{
	// scroll down the content of the view by n lines
	// use it to optimise redrawing
}

uint QYZView::stringWidth( const QString& str ) const
{
	// width of the string using the current font ??
	// why do we need it in libyzis ? We are supposed to only manipulate 
	// lines and columns, no ?
	return str.length();
}

uint QYZView::charWidth( const QChar& ch ) const
{
	// width of the string using the current font ??
	// why do we need it in libyzis ? We are supposed to only manipulate 
	// lines and columns, no ?
	return 1;
}

void QYZView::registerModifierKeys( const QString& keys )
{
	// ???
}

void QYZView::refreshScreen()
{
	qDebug("QYZView - refreshScreen");
	// repaint the current view
	repaint();
}


void QYZView::resizeEvent ( QResizeEvent * e)
{
	qDebug("QYZView - resizeEvent: %dx%d", e->size().width(), e->size().height() );
	/*
	isFontFixed = fontInfo().fixedPitch();

	//mParent->setFixedFont( isFontFixed );
	//spaceWidth = mParent->spaceWidth;
	//mCursor->resize( fontMetrics().maxWidth(), fontMetrics().lineSpacing() );
	if ( isFontFixed )
		mCursor->setCursorType( KYZisCursor::KYZ_CURSOR_SQUARE );
	else
		mCursor->setCursorType( KYZisCursor::KYZ_CURSOR_LINE );
	*/

	int lines = height() / fontMetrics().lineSpacing();
	// if font is fixed, calculate the number of columns fontMetrics().maxWidth(), else give the width of the widget
	int columns = width() / fontMetrics().maxWidth();
	erase( );
	setVisibleArea( columns, lines );
	QVBox::resizeEvent( e );
}

void QYZView::wheelEvent( QWheelEvent * e ) {
	int n = - e->delta();
	yzDebug(QYZIS) << "wheelEvent " << n;
//	scrollView( getCurrentTop() + n );
}

bool QYZView::event(QEvent *e) {
	qDebug("QYZView - event" );
	/*
	if ( e->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = (QKeyEvent *)e;
		if ( ke->key() == Key_Tab ) {
			keyPressEvent(ke);
			return TRUE;
		}
	}
	*/
	return QVBox::event(e);
}

void QYZView::keyPressEvent ( QKeyEvent * e ) {
	qDebug("QYZView - keyPressEvent %s", e->text().latin1() );
	ButtonState st = e->state();
	QString modifiers;
	if ( st & Qt::ShiftButton )
		modifiers = "<SHIFT>";
	if ( st & Qt::AltButton )
		modifiers += "<ALT>";
	if ( st & Qt::ControlButton )
		modifiers += "<CTRL>";

	if ( mKeyConverter->contains( e->key() ) ) //to handle some special keys
		sendKey(mKeyConverter->convertKey( e->key() ), modifiers);
	else
		sendKey( e->text(), modifiers );
	e->accept();
}


void QYZView::paintEvent( unsigned int curx, unsigned int cury, 
	unsigned int curw, unsigned int curh )
{
	qDebug("QYZView - paintEvent %dx%d, %dx%d", curx, cury, curw, curh );
	repaint();
}





