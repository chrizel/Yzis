#include "kyzisview.h"

KYZisView::KYZisView ( KYZisDoc *doc, QWidget *parent, const char *name ) 
: KTextEditor::View (doc, parent, name)
{
	editor = new KYZisEdit (parent,"editor");
	buffer = doc;
}

KYZisView::~KYZisView () {

}

#include "kyzisview.moc"
