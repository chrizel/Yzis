#include <qobject.h>

/**
 * this class represents a line in the buffer
 * it holds the actual data and metadata
 */
class YZLine
{
	public:
		YZLine(const QString &l);
		YZLine();
		~YZLine();

		QString data() { return mData; }
		void setData(const QString &data) { mData = data; }

	private:
		QString mData;
};

