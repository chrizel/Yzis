#ifndef KYZISDOC_H
#define KYZISDOC_H

#include <ktexteditor/document.h>
#include <yz_buffer.h>
#include <yz_session.h>

class KYZisDoc : public KTextEditor::Document, public YZBuffer {
	Q_OBJECT
		
	public:
		KYZisDoc (QWidget *parentWidget = 0, const char *widgetName = 0,QObject *parent = 0, const char *name = 0);
		virtual ~KYZisDoc ();

		KTextEditor::View *createView ( QWidget *parent, const char *name = 0 );
		QPtrList<KTextEditor::View> views() const { return _views; }
		void removeView( KTextEditor::View * v );

	protected:
		bool openFile();
		bool saveFile();

	private:
		QPtrList<KTextEditor::View> _views;
};

#endif
