/**
 * $Id$
 */
#include "kyzisview.h"
#include "kyzis_factory.h"
#include <qlayout.h>
#include <qevent.h>
#include <qapplication.h>
#include <kdebug.h>

KYZisView::KYZisView ( KYZisDoc *doc, YZSession *_session, QWidget *parent, const char *name )
	: KTextEditor::View (doc, parent, name),
		YZView(doc, 10) {
	currentSession = _session;
	editor = new KYZisEdit (this,"editor");
	command = new KYZisCommand ( this, "command" );
	status = new KStatusBar (this, "status");
	status->insertItem("Yzis Ready",0);
	status->insertItem("",99,1,true);
	status->setItemAlignment(99,Qt::AlignRight);
	status->setFixedHeight(status->height());
	
	QVBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(editor);
	l->addWidget(command);
	l->addWidget(status);

	registerManager(this);

	buffer = doc;
	editor->show();
	status->show();
	editor->setFocus();
}

KYZisView::~KYZisView () {
	delete editor;
	delete status;
}

void KYZisView::postEvent(yz_event /*ev*/) {
	QCustomEvent *myev = new QCustomEvent (QEvent::User);
	QApplication::postEvent( this, myev ); //this hopefully gives Qt the priority before processing our own events
}

//receives previously generated events from Qt event loop. hopefully it will do
//what I want :)
void KYZisView::customEvent (QCustomEvent *) {
	while ( true ) {
		yz_event event = fetchNextEvent();
		switch ( event.id ) {
			case YZ_EV_INVALIDATE_LINE: {
				QString str  = buffer->findLine( event.invalidateline.y );
				if ( str.isNull() ) return;
				editor->setTextLine(event.invalidateline.y, str);
				break;
			}
			case YZ_EV_SET_CURSOR:
				kdDebug() << "event SET_CURSOR" << endl;
				editor->setCursor (event.setcursor.x, event.setcursor.y);
				status->changeItem( QString("%1,%2-%3 (%4)").arg(event.setcursor.x ).arg( event.setcursor.y ).arg( event.setcursor.y2 ).arg( event.setcursor.percentage),99 );
				break;
			case YZ_EV_SET_STATUS:
				kdDebug() << "event SET_STATUS" << event.setstatus.text <<  endl;
				status->changeItem( event.setstatus.text,0);
				break;
			case YZ_EV_REDRAW:
				editor->updateContents();
				break;
			case YZ_EV_NOOP:
				return;
		}
	}
}

void KYZisView::scrollDown( int lines ) {
	kdDebug() << "ScrollDown " << lines <<endl;
	editor->scrollBy(0, lines * editor->fontMetrics().lineSpacing());
	editor->update();
}

void KYZisView::scrollUp ( int lines ) {
	kdDebug() << "ScrollUp " << lines <<endl;
	editor->scrollBy(0, -1 * lines * editor->fontMetrics().lineSpacing());
	editor->update();
}

YZSession *KYZisView::getCurrentSession() {
	return currentSession;
}

void KYZisView::setCommandLineText( const QString& text ) {
	command->lineEdit()->setText( text );
}

QString KYZisView::getCommandLineText() const {
	return command->lineEdit()->text();
}

#include "kyzisview.moc"
