#ifndef KYZISVIEW_H
#define KYZISVIEW_H

#include <ktexteditor/view.h>
#include <kstatusbar.h>
#include <qevent.h>
#include "document.h"
#include "editor.h"
#include "viewwidget.h"
#include "command.h"

class KYZisEdit;
class KYZisCommand;

class KYZisView: public KTextEditor::View
	, public YZView
//	, public Gui
{
	Q_OBJECT

	friend class KYZisFactory;

	public:
		KYZisView(KYZisDoc *doc, QWidget *parent, const char *name=0);
	 	virtual ~KYZisView();
		KTextEditor::Document *document () const { return buffer; }
	
	protected:

	private:
		KYZisEdit *editor;
		KYZisDoc *buffer;
		KStatusBar *status;
		KYZisCommand *command;
};

#endif
