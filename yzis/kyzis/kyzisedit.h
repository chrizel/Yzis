#ifndef KYZISEDIT_H
#define KYZISEDIT_H

#include <qtextedit.h>

/**
 * KYZis Painter Widget
 */
class KYZisEdit : public QTextEdit {
	Q_OBJECT

	public :
		KYZisEdit(QWidget *parent=0, const char *name=0);
		virtual ~KYZisEdit();

	private :
};

#endif
