#ifndef KYZISDOC_H
#define KYZISDOC_H

#include <ktexteditor/document.h>
#include <buffer.h>
#include <session.h>

class KYZisDoc : public KTextEditor::Document, public YZBuffer {
	Q_OBJECT
		
	public:
		KYZisDoc (bool bSingleViewMode, bool bBrowserView, bool bReadOnly, QWidget *parentWidget = 0, const char *widgetName=0, QObject *parent=0, const char *name=0);
		virtual ~KYZisDoc ();

		KTextEditor::View *createView ( QWidget *parent, const char *name = 0 );
		QPtrList<KTextEditor::View> views() const { return _views; }
		void removeView( KTextEditor::View * v );
		QWidget *parentWidget() { return m_parent; }

	protected:
		bool openFile();
		bool saveFile();

	private:
		QPtrList<KTextEditor::View> _views;
		QWidget *m_parent;
};

#endif
