#ifndef KYZISVIEW_H
#define KYZISVIEW_H

#include <ktexteditor/view.h>
#include "kyzisdoc.h"
#include "kyzisedit.h"
#include <yz_view.h>
#include <kstatusbar.h>
#include <qevent.h>
#include <gui.h>
#include "kyziscommand.h"

class KYZisEdit;
class KYZisCommand;

class KYZisView: public KTextEditor::View
	, public YZView
	, public Gui
{
	Q_OBJECT

	public:
		KYZisView(KYZisDoc *doc, YZSession*, QWidget *parent, const char *name=0);
	 	virtual ~KYZisView();
		KTextEditor::Document *document () const { return buffer; }
		void postEvent (yz_event);
		void scrollDown( int l=1 );
		void scrollUp( int l=1 );
		YZSession *getCurrentSession();
		void setCommandLineText( const QString& text );
		QString getCommandLineText() const;
		
	protected:
		void customEvent( QCustomEvent * );

	private:
		KYZisEdit *editor;
		KYZisDoc *buffer;
		KStatusBar *status;
		KYZisCommand *command;
		YZSession *currentSession;
		QString commandline;
};

#endif
