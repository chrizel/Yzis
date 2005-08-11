//
// C++ Interface: history
//
// Description: 
//
//
// Author: Craig Howard <craig@choward.ca>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef HISTORY_H
#define HISTORY_H

class QString;
class QTextStream;

/**
	@author Craig Howard <craig@choward.ca>
*/
class YZHistory{
public:
    YZHistory();
    virtual ~YZHistory();
	
	/**
	 * add a new entry to the end of the history list
	 */
	void addEntry( const QString &entry );
	
	/**
	 * returns the last entry added to the list
	 * this should be the most commonly used access method,
	 * not getEntryByIdx()
	 */
	QString &getEntry();
	const QString &getEntry() const;
	
	/**
	 * move back in time one command
	 */
	void goBackInTime();
	
	/**
	 * move forward in time one command
	 */
	void goForwardInTime();
	
	/**
	 * returns true iff we cannot go any further back in time
	 */
	bool atBeginning() const;
	
	/**
	 * returns true iff we cannot go any further forward in time
	 */
	bool atEnd() const;

	/**
	 * return the number of entries in the history
	 */
	unsigned int getNumEntries() const;
	
	/**
	 * empty test
	 */
	bool isEmpty() const;
	
	/**
	 * get entries at index
	 * ugh, I don't like this interface, but I don't
	 * want to template it and make iterators
	 */
	QString &getEntryByIdx( unsigned int idx );
	const QString &getEntryByIdx( unsigned int idx ) const;
	
	/**
	 * write the history out to text stream
	 * useful for saving to .yzisinfo
	 */
	QTextStream& writeToStream( QTextStream &stream ) const;
	
private:
	struct Private;
	Private *d;	
};

#endif
