#ifndef KYZISEDIT_H
#define KYZISEDIT_H

#include <qtextedit.h>
#include "kyzisview.h"

class KYZisView;
/**
 * KYZis Painter Widget
 */
class KYZisEdit : public QTextEdit {
	Q_OBJECT

	public :
		KYZisEdit(KYZisView *parent=0, const char *name=0);
		virtual ~KYZisEdit();

	protected:
		void keyPressEvent (QKeyEvent *);

	private :
		KYZisView *_parent;
};

#endif
