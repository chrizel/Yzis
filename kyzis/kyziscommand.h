#ifndef KYZISCOMMAND_H
#define KYZISCOMMAND_H

#include "kyzisview.h"
#include <kcombobox.h>

class KYZisView;

/**
 * KYzis command line
 */
class KYZisCommand : public KComboBox {
	Q_OBJECT

	public :
		KYZisCommand(KYZisView *parent=0, const char *name=0);
		virtual ~KYZisCommand();

	protected:
		void keyPressEvent (QKeyEvent *);

	private :
		KYZisView *_parent;
};

#endif
