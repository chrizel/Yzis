/* This file is part of the Yzis libraries
 *  Copyright (C) 2003,2004 Mickael Marchand <marchand@kde.org>,
 *  Thomas Capricelli <orzel@freehackers.org>,
 *  Philippe Fremy <pfremy@freehackers.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 **/

/**
 * $Id$
 */

#include <assert.h>
#include <cstdlib>
#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qdir.h>

#include "buffer.h"
#include "line.h"
#include "view.h"
#include "undo.h"
#include "debug.h"

#define ASSERT_TEXT_WITHOUT_NEWLINE( functionname, text ) \
	YZASSERT_MSG( text .contains('\n')==false, QString("%1 - text contains newline").arg(text) )

#define ASSERT_LINE_EXISTS( functionname, line ) \
	YZASSERT_MSG( line < lineCount(), QString("%1 - line %2 does not exist, buffer has %3 lines").arg(functionname).arg(line).arg(lineCount()) )

#define ASSERT_NEXT_LINE_EXISTS( functionname, line ) \
	YZASSERT_MSG( line <= lineCount(), QString("%1 - line %2 does not exist, buffer has %3 lines").arg(functionname).arg(line).arg(lineCount()) )

#define ASSERT_COL_LINE_EXISTS( functionname, col, line ) \
	YZASSERT_MSG( col < textline(line).length(), QString("%1 - col %2 does not exist, line %3 has %4 columns").arg( functionname ).arg( col ).arg( line ).arg( textline(line).length() ) );

#define ASSERT_PREV_COL_LINE_EXISTS( functionname, col, line ) \
	YZASSERT_MSG( col <= textline(line).length(), QString("%1 - col %2 does not exist, line %3 has %4 columns").arg( functionname ).arg( col ).arg( line ).arg( textline(line).length() ) );

YZBuffer::YZBuffer(YZSession *sess) {
	myId = YZSession::mNbBuffers++;
	mIntro = false;
	mUpdateView=true;
	mSession = sess;
	mText.setAutoDelete( true );
	mModified=false;
	m_highlight = 0L;
	// buffer at creation time should use a non existing temp filename
	// find a tmp file that does not exist
	do {
		mPath = QString("/tmp/yzisnew%1").arg(random());
	} while ( QFileInfo( mPath ).exists() == true );
	// there is still a possible race condition here...
	mFileIsNew = true;
	mUndoBuffer = new YZUndoBuffer( this );
	displayIntro();
	mSession->addBuffer( this );
	yzDebug() << "NEW BUFFER CREATED : " << mPath << endl;
}

YZBuffer::~YZBuffer() {
	if ( m_highlight != 0L )
		m_highlight->release();
	mText.clear();
	delete mUndoBuffer;
	// delete the temporary file if we haven't changed the file
}

void YZBuffer::detach(void)
{
	mSession->rmBuffer(this);
}

// ------------------------------------------------------------------------
//                            Char Operations 
// ------------------------------------------------------------------------

void YZBuffer::insertChar(unsigned int x, unsigned int y, const QString& c) {
	ASSERT_TEXT_WITHOUT_NEWLINE( QString("YZBuffer::insertChar(%1,%2,%3))").arg(x).arg(y).arg(c), c )
	ASSERT_LINE_EXISTS( QString("YZBuffer::insertChar(%1,%2,%3)").arg(x).arg(y).arg(c), y )

	/* brute force, we'll have events specific for that later on */
	QString l=textline(y);
	if (l.isNull()) return;

	ASSERT_PREV_COL_LINE_EXISTS( QString("YZBuffer::insertChar(%1,%2,%3)").arg(x).arg(y).arg(c),x,y)

	if (x > l.length()) {
		// if we let Qt proceed, it would append spaces to extend the line
		// and we do not want that
		return;
	}

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, c, x, y );

	l.insert(x, c);
	setTextline(y,l);

	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->invalidateLine( y );

}

void YZBuffer::chgChar (unsigned int x, unsigned int y, const QString& c) {
	ASSERT_TEXT_WITHOUT_NEWLINE( "YZBuffer::chgChar(%1,%2,%3).arg(x).arg(y).arg(c)", c )
	ASSERT_LINE_EXISTS( QString("YZBuffer::chgChar(%1,%2,%3)").arg(x).arg(y).arg(c), y )

	QString l=textline(y);
	if (l.isNull()) return;

	ASSERT_COL_LINE_EXISTS( QString("YZBuffer::chgChar(%1,%2,%3)").arg(x).arg(y).arg(c),x,y)

	if (x >= l.length()) {
		// if we let Qt proceed, it will append spaces to extend the line
		return;
	}

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
	                                 l.mid(x,1), x, y );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, c, x, y );
	
	/* do the actual modification */
	l.remove(x, 1);
	l.insert(x, c);

	setTextline(y,l);

	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() )
		it->invalidateLine( y );
}

void YZBuffer::delChar (unsigned int x, unsigned int y, unsigned int count) {
	ASSERT_LINE_EXISTS( QString("YZBuffer::delChar(%1,%2,%3)").arg(x).arg(y).arg(count), y )

	/* brute force, we'll have events specific for that later on */
	QString l=textline(y);
	if (l.isNull()) return;

	ASSERT_COL_LINE_EXISTS( QString("YZBuffer::delChar(%1,%2,%3)").arg(x).arg(y).arg(count),x,y)

	if (x >= l.length())
		return;

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, l.mid(x,count), x, y );
	
	/* do the actual modification */
	l.remove(x, count);

	setTextline(y,l);

	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() )
		it->invalidateLine( y );
}

// ------------------------------------------------------------------------
//                            Line Operations 
// ------------------------------------------------------------------------

void  YZBuffer::appendLine(const QString &l) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::appendLine(%1)").arg(l),l);

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, 
	                                 QString(), 0, lineCount() );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
	                                   l,  0, lineCount());

	mText.append(new YZLine(l));
	if ( m_highlight ) {
		bool ctxChanged = false;
		QMemArray<signed char> foldingList;
		m_highlight->doHighlight(( mText.count() >= 2 ? yzline( mText.count() - 2 ) : new YZLine()), yzline( mText.count() - 1 ), &foldingList, &ctxChanged );
	}
	setModified( true );
	updateAllViews();
}


void  YZBuffer::insertLine(const QString &l, unsigned int line) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_NEXT_LINE_EXISTS(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),line)   
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, 
	                                   QString(), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
	                                   l, 0, line );

	mText.insert(line, new YZLine(l));
	setModified( true );
	updateAllViews();
}

void YZBuffer::insertNewLine( unsigned int col, unsigned int line ) {
	ASSERT_LINE_EXISTS(QString("YZBuffer::insertNewLine(%1,%2)").arg(col).arg(line),line);
	if ( line == lineCount() ) {//we are adding a line, fake being at end of last line
		line --;
		col = textline(line).length(); 
	}

	QString l=textline(line);
	if (l.isNull()) return;

	ASSERT_PREV_COL_LINE_EXISTS(QString("YZBuffer::insertNewLine(%1,%2)").arg(col).arg(line),col,line )    

	if (col > l.length() ) return;

	QString newline = l.mid( col );
	if ( newline.isNull() ) newline = QString( "" );

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, 
	                                   "", col, line+1 );
	if (newline.length()) {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
										   newline, col, line );
		mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
										   newline, 0, line+1 );
	}

	//replace old line
	setTextline(line,l.left( col ));

	//add new line
	mText.insert( line + 1, new YZLine(newline));

	/* inform the views */
	updateAllViews();
}

void YZBuffer::deleteLine( unsigned int line ) {
	ASSERT_LINE_EXISTS(QString("YZBuffer::deleteLine(%1)").arg(line),line)   

	if (line >= lineCount()) return;

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
										 textline(line), 0, line );
	if (lineCount() > 1) {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELLINE, 
										 "", 0, line );
		mText.remove(line);
	} else {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
										 "", 0, line );
		setTextline(0,"");
	}

	setModified( true );
	updateAllViews(); //hmm ...
}

void YZBuffer::replaceLine( const QString& l, unsigned int line ) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::replaceLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_LINE_EXISTS(QString("YZBuffer::replaceLine(%1,%2)").arg(l).arg(line),line)   
	
	if ( textline( line ).isNull() ) return;

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
	                                   textline(line), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
	                                   l, 0, line );
	setTextline(line,l);
	/* inform the views */
	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->invalidateLine( line );
}

void YZBuffer::mergeNextLine( unsigned int line ) {
	replaceLine( textline( line ) + textline( line+1 ), line );
	deleteLine( line+1 );
	setModified( true );
}

// ------------------------------------------------------------------------
//                            Content Operations 
// ------------------------------------------------------------------------

void YZBuffer::clearText() {
	/* XXX clearText is not registered to the undo buffer but should be
	 * as any other text operation. Although I doubt that this is a common
	 * operation.
	 */
	mText.clear();
	mText.append(new YZLine());
}

QString YZBuffer::textline( uint line ) const {
	if (yzline(line)) {
		QString s = yzline(line)->data();
		return yzline(line)->data();
	}
	return QString::null;
}

void YZBuffer::clearIntro() {
	yzDebug() << "ClearIntro"<< endl;
	mIntro = false;
	clearText();
	updateAllViews();
}

void YZBuffer::displayIntro() {
	yzDebug() << "DisplayIntro"<< endl;

	QStringList introduction;
	introduction 
	<<  ""
	<<  ""
	<<  ""
	<< VERSION_CHAR_LONG
	<< VERSION_CHAR_DATE
	<<  ""
	<<	"Development Release - Use for testing only" 
	<<  "Copyright 2003, 2004 Yzis Team <yzis-dev@yzis.org>"
	<<  ""
	<<  "Please report bugs at http://bugs.yzis.org" ;

	mUndoBuffer->setInsideUndo( true );
	for ( int i=0; i< 100; i++ ) introduction << ""; //add empty lines to avoids displaying '~' :)

	for (  QStringList::Iterator it = introduction.begin(); it != introduction.end(); ++it )
		mText.append( new YZLine( *it ) );
	mIntro=true;
	mUndoBuffer->setInsideUndo( false );

	updateAllViews();
}

YZLine * YZBuffer::yzline(unsigned int line) const {
	return ( ( QPtrList<YZLine> ) mText ).at(line);
}

void YZBuffer::setTextline( uint line , const QString & l) {
	ASSERT_TEXT_WITHOUT_NEWLINE( QString("YZBuffer::setTextline(%1,%2)").arg(line).arg(l), l );
	ASSERT_LINE_EXISTS( QString("YZBuffer::setTextline(%1,%2)").arg(line).arg(l), line );
	if (yzline(line)) {
		if (l.isNull()) {
			yzline(line)->setData(QString(""));
		} else {
			yzline(line)->setData(l);
		}
	} 
	if ( m_highlight ) {
		bool ctxChanged = false;
		QMemArray<signed char> foldingList;
		m_highlight->doHighlight(( line >= 1 ? yzline( line -1 ) : new YZLine()), yzline( line ), &foldingList, &ctxChanged );
	}
	setModified( true );
}

bool YZBuffer::isEmpty() const {
	if ( mText.count( ) == 1 && textline(0).isEmpty() ) return true;
	return false;
}


QString YZBuffer::getWholeText() const {
	if ( isEmpty() ) { return QString(""); }

	QString wholeText;
	for ( uint i = 0 ; i < lineCount() ; i++ )
		wholeText += textline(i) + "\n";
	return wholeText;
}

uint YZBuffer::getWholeTextLength() const {
	if ( isEmpty() ) { return 0; }

	uint length = 0;
	for ( uint i = 0 ; i < lineCount() ; i++ ) {
		length += textline(i).length() + 1;
	}
	
	return length;
}

uint YZBuffer::firstNonBlankChar( uint line ) {
	uint i=0;
	QString s = textline(line);
	if (s == QString::null) return 0;
	while( s[i].isSpace() && i < s.length()) {
		i++;
	}
	return i;
}
	
// ------------------------------------------------------------------------
//                            File Operations 
// ------------------------------------------------------------------------

void YZBuffer::load(const QString& file) {
	//stop redraws
	if ( mIntro ) clearIntro();
	mUpdateView=false;
	mText.clear();
	QString oldPath = mPath;
	mPath = file;
	if ( !file.isNull() ) { 
	//check if the path is absolute or relative
		if (file[0] != '/') {
			mPath = QDir::cleanDirPath(QDir::current().absPath()+"/"+mPath);
			yzDebug() << "Changing path to " << mPath << endl;
		}
		//hmm changing file :), update Session !!!!
		mSession->updateBufferRecord( oldPath, mPath, this );
	} else return; // no file name ...
	mFileIsNew=false;
	//HL mode selection
	int hlMode = YzisHlManager::self()->detectHighlighting (this);
	setHighLight( hlMode );
	yzDebug() << "HIGHLIGHTING " << hlMode << endl;

	QFile fl( mPath );
	//opens and eventually create the file
	if ( fl.open( IO_ReadOnly ) ) {
		QTextStream stream( &fl );
		while ( !stream.atEnd() ) {
			QString line(stream.readLine() ); // line of text excluding '\n'
			appendLine( line );
		}
		fl.close();
	}
	if ( ! mText.count() ) {
		mUndoBuffer->setInsideUndo( true );
		appendLine("");
		mUndoBuffer->setInsideUndo( false );
	}
	setModified( false );
	//reenable
	mUpdateView=true;
	updateAllViews();
	filenameChanged();
}

bool YZBuffer::save() {
	if (mPath.isEmpty())
		return false;
	if ( mFileIsNew ) {
		//popup to ask a file name
		if ( !popupFileSaveAs() )
			return false; //dont try to save
	}
	QFile file( mPath );
	yzDebug() << "Saving file to " << mPath << endl;
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		// do not save empty buffer to avoid creating a file
		// with only a '\n' while the buffer is emtpy
		if ( isEmpty() == false) {
			for(YZLine *it = mText.first(); it; it = mText.next()) {
				stream << it->data() << "\n";
			}
		}
		file.close();
	}
	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->displayInfo(tr("Written %1 bytes to file %2").arg(getWholeTextLength()).arg(mPath));
	setModified( false );
	filenameChanged();
	return true;
}

// ------------------------------------------------------------------------
//                            View Operations 
// ------------------------------------------------------------------------

void YZBuffer::addView (YZView *v) {
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() )
		if ( it == v ) {
			yzWarning()<< "view " << ( int )v << " added for the second time, discarding"<<endl;
			return; // don't append twice
		}
	yzDebug() << "BUFFER: addView" << endl;
	mViews.append( v );
	mSession->setCurrentView( v );
}

YZView* YZBuffer::findView( unsigned int uid ) {
	yzDebug() << "Buffer: findView " << uid << endl;
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() ){
//		yzDebug() << "buffer:findViewChecking view " << uid << " for buffer " << fileName() << endl;
		if ( it->myId == uid ) {
//			yzDebug() << "Buffer:findView " << uid << " found" << endl;
			return it;
		}
	}
//	yzDebug() << "buffer::findView " << uid << " returning NULL" << endl;
	return NULL;
}

void YZBuffer::updateAllViews() {
	if ( !mUpdateView ) return;
	yzDebug() << "updateAllViews" << endl;
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() ) {
		it->redrawScreen();
		it->updateCursor();
	}
}

YZView* YZBuffer::firstView() {
	if (  mViews.first() != NULL ) 
		return mViews.first();
	else yzDebug() << "No VIEW !!!" << endl;
	return NULL;//crash me :)
}

void YZBuffer::rmView(YZView *v) {
	int f = mViews.remove(v);
	YZASSERT( 1==f ); // isn't it ?
	yzDebug() << "buffer: removeView found " << f << " views" << endl;
	if ( mViews.isEmpty() ) 
		detach();

}

// ------------------------------------------------------------------------
//                            Undo/Redo Operations
// ------------------------------------------------------------------------

QString YZBuffer::undoLast( const QString& , YZCommandArgs args ) {
	yzDebug() << "buffer: undoLast" << endl;
	mUndoBuffer->undo();	
	
	//reset the input buffer of the originating view
	args.view->purgeInputBuffer();

	//return something
	return QString::null;
}

void YZBuffer::setModified( bool modif ) {
	mModified = modif;
	if ( !mUpdateView ) return;
	statusChanged();
}

void YZBuffer::statusChanged() {
	//update all views
	for ( YZView *it = mViews.first(); it; it = mViews.next() )
		it->syncViewInfo();
}


// ------------------------------------------------------------------------
//                            Syntax Highlighting
// ------------------------------------------------------------------------

void YZBuffer::setHighLight( uint mode ) {
	YzisHighlighting *h = YzisHlManager::self()->getHl( mode );
	
	if ( h != m_highlight ) { //HL is changing
		if ( m_highlight != 0L ) 
			m_highlight->release(); //free memory

		//init
		h->use();

		m_highlight = h;

		makeAttribs();
		//emit hlChanged();
	}
}

void YZBuffer::makeAttribs() {
	m_highlight->clearAttributeArrays();

	//do something on the views
	
	//buffer->invalidateHL(); ?
	
	//tagAll(); ?
}
