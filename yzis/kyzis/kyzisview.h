#ifndef KYZISVIEW_H
#define KYZISVIEW_H

#include <ktexteditor/view.h>
#include "kyzisdoc.h"
#include "kyzisedit.h"

class KYZisView: public KTextEditor::View {
	Q_OBJECT

	public:
		KYZisView(KYZisDoc *doc, QWidget *parent=0, const char *name=0);
	 	virtual ~KYZisView();
		KTextEditor::Document *document () const { return buffer; }

	private:
		KYZisEdit *editor;
		KYZisDoc *buffer;
		
};

#endif
