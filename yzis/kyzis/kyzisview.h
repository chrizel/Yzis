#ifndef KYZISVIEW_H
#define KYZISVIEW_H

#include <ktexteditor/view.h>
#include "kyzisdoc.h"
#include "kyzisedit.h"
#include <yz_view.h>
#include <kstatusbar.h>
#include <qevent.h>
#include <event_mgr.h>


class KYZisEdit;

class KYZisView: public KTextEditor::View
	, public YZView
	, public EventMgr
{
	Q_OBJECT

	public:
		KYZisView(KYZisDoc *doc, QWidget *parent, const char *name=0);
	 	virtual ~KYZisView();
		KTextEditor::Document *document () const { return buffer; }
		void postEvent (yz_event);
		
	protected:
		void customEvent( QCustomEvent * );

	private:
		KYZisEdit *editor;
		KYZisDoc *buffer;
		KStatusBar *status;
};

#endif
