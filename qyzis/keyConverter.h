#ifndef KEY_CONVERTER_H
#define KEY_CONVERTER_H

#include <qmap.h>
#include <qstring.h>

class KeyConverter {
public:
	KeyConverter();
	/** Convert Qt::key code to a YZis key string.
	  * Return QString::null if the key can not be converted.
	  */
	QString convertKey( int );

	/** Return true if key contained */
	bool contains( int key ) { return keys.contains( key ); }

protected:
	QMap<int,QString> keys;
	void initKeys();
};

#endif // KEY_CONVERTER_H	
