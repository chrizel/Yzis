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
	yzDebug("YZBuffer") << "YZBuffer()" << endl;
	myId = YZSession::mNbBuffers++;
	mIntro = false;
	mUpdateView=true;
	mSession = sess;
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
	mAction = new YZAction( this );
	mMarks = new YZMark( );
	displayIntro();
	YZSession::me->addBuffer( this );
	mSwap = new YZSwapFile( this );
	yzDebug("YZBuffer") << "NEW BUFFER CREATED : " << mPath << endl;
}

YZBuffer::~YZBuffer() {
	//remove swap file
	mSwap->unlink();
	delete mSwap;
	if ( m_highlight != 0L )
		m_highlight->release();
	mText.clear();
	delete mUndoBuffer;
	delete mAction;
	delete mMarks;
	//clear views
//	YZView *it;
//	for ( it = mViews.first(); it ; it = mViews.next() )
//		delete it;
	// delete the temporary file if we haven't changed the file
}

void YZBuffer::detach() {
	mSession->rmBuffer(this);
}

// ------------------------------------------------------------------------
//                            Char Operations 
// ------------------------------------------------------------------------

void YZBuffer::insertChar(unsigned int x, unsigned int y, const QString& c) {
	ASSERT_TEXT_WITHOUT_NEWLINE( QString("YZBuffer::insertChar(%1,%2,%3)").arg(x).arg(y).arg(c), c )
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
/*	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->invalidateLine( y ); */
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
}

void YZBuffer::delChar (unsigned int x, unsigned int y, unsigned int count) {
	ASSERT_LINE_EXISTS( QString("YZBuffer::delChar(%1,%2,%3)").arg(x).arg(y).arg(count), y )

	/* brute force, we'll have events specific for that later on */
	QString l=textline(y);
	if (l.isNull()) return;

	if (x >= l.length())
		return;

	ASSERT_COL_LINE_EXISTS( QString("YZBuffer::delChar(%1,%2,%3)").arg(x).arg(y).arg(count),x,y)


	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, l.mid(x,count), x, y );
	
	/* do the actual modification */
	l.remove(x, count);

	setTextline(y,l);
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
	if ( m_highlight != 0L ) {
		bool ctxChanged = false;
		QMemArray<signed char> foldingList;
		m_highlight->doHighlight(( mText.count() >= 2 ? yzline( mText.count() - 2 ) : new YZLine()), yzline( mText.count() - 1 ), &foldingList, &ctxChanged );
//		if ( ctxChanged ) yzDebug("YZBuffer") << "CONTEXT changed"<<endl; //no need to take any action at EOF ;)
	}
	setModified( true );
}


void  YZBuffer::insertLine(const QString &l, unsigned int line) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_NEXT_LINE_EXISTS(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),line)   
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, 
	                                   QString(), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, 
	                                   l, 0, line );

	QValueVector<YZLine*>::iterator it;
	uint idx=0;
	for ( it = mText.begin(); idx < line && it != mText.end(); it++, idx++ )
		;	
	mText.insert(it, new YZLine( l ));
//	mText.insert(line, new YZLine(l));
	setModified( true );
}

void YZBuffer::insertNewLine( unsigned int col, unsigned int line ) {
	if (line == lineCount()) {
		YZASSERT_MSG(line==lineCount() && col==0, QString("YZBuffer::insertNewLine on last line is only possible on col 0").arg(col).arg(line));
	} else {
		ASSERT_LINE_EXISTS(QString("YZBuffer::insertNewLine(%1,%2)").arg(col).arg(line),line);
	}
	if ( line == lineCount() ) {
		//we are adding a new line at the end of the buffer by adding a new
		//line at the beginning of the next unexisting line
		//fake being at end of last line to make it work
		line --;
		col = textline(line).length(); 
	}

	QString l=textline(line);
	if (l.isNull()) return;

	ASSERT_PREV_COL_LINE_EXISTS(QString("YZBuffer::insertNewLine(%1,%2)").arg(col).arg(line),col,line )    

	if (col > l.length() ) return;

	QString newline = l.mid( col );
	if ( newline.isNull() ) newline = QString( "" );

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, "", col, line+1 );
	if (newline.length()) {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, newline, col, line );
		mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, newline, 0, line+1 );
	}

	//replace old line
	setTextline(line,l.left( col ));

	//add new line
	QValueVector<YZLine*>::iterator it;
	uint idx=0;
	for ( it = mText.begin(); idx < line+1 && it != mText.end(); it++, idx++ )
		;
	mText.insert(it, new YZLine( newline ));
}

void YZBuffer::deleteLine( unsigned int line ) {
	ASSERT_LINE_EXISTS(QString("YZBuffer::deleteLine(%1)").arg(line),line)   

	if (line >= lineCount()) return;

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, textline(line), 0, line );
	if (lineCount() > 1) {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELLINE, 
										 "", 0, line );
		QValueVector<YZLine*>::iterator it;
		uint idx=0;
		for ( it = mText.begin(); idx < line && it != mText.end(); it++, idx++ )
			;	
		mText.erase(it);
	} else {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, 
										 "", 0, line );
		setTextline(0,"");
	}

	setModified( true );
}

void YZBuffer::replaceLine( const QString& l, unsigned int line ) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::replaceLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_LINE_EXISTS(QString("YZBuffer::replaceLine(%1,%2)").arg(l).arg(line),line)   
	
	if ( textline( line ).isNull() ) return;

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, textline(line), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, l, 0, line );
	setTextline(line,l);
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
	yzDebug("YZBuffer") << "YZBuffer clearText" << endl;
	/* XXX clearText is not registered to the undo buffer but should be
	 * as any other text operation. Although I doubt that this is a common
	 * operation.
	 */
	mText.clear();
	mText.append(new YZLine());
}

void YZBuffer::clearIntro() {
	yzDebug("YZBuffer") << "YZBuffer clearIntro" << endl;
	mIntro = false;
	clearText();
	updateAllViews();
}

void YZBuffer::displayIntro() {
	yzDebug("YZBuffer") << "YZBuffer displayIntro" << endl;
	QStringList introduction;
	introduction 
	<<  ""
	<<  ""
	<<  ""
	<< VERSION_CHAR_LONG
	<< VERSION_CHAR_DATE
	<<  ""
	<<	"Development Release - Use for testing only" 
	<<  ""
	<<	"Copyright (c) 2003, 2004 :"
	<<	"Mickael Marchand <mikmak@yzis.org>,"
	<<	"Thomas Capricelli <orzel@freehackers.org>,"
	<<	"Philippe Fremy <phil@freehackers.org>"
	<<	"Loic Pauleve <panard@inzenet.org>"
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
	if ( m_highlight != 0L ) {
		uint hlLine = line;
		bool ctxChanged = true;
		bool hlChanged = false;
		while ( ctxChanged && hlLine < lineCount()) {
			QMemArray<signed char> foldingList;
			m_highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine -1 ) : new YZLine()), yzline( hlLine ), &foldingList, &ctxChanged );
			if ( hlLine != line ) hlChanged = true;
			hlLine++;
		}
		if ( hlChanged ) updateAllViews( );
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
	yzDebug("YZBuffer") << "YZBuffer load " << file << endl;
	QString oldPath = mPath;
	if ( file.isNull() || file.isEmpty() ) return;
	setPath(file);
	//hmm changing file :), update Session !!!!
	mSession->updateBufferRecord( oldPath, mPath, this );
	if ( mIntro ) clearIntro();
	//stop redraws
	mUpdateView=false;
	mText.clear();
	mFileIsNew=false;

	//HL mode selection
	int hlMode = YzisHlManager::self()->detectHighlighting (this);
	if ( hlMode >=0 )
		setHighLight( hlMode );
	yzDebug("YZBuffer") << "HIGHLIGHTING " << hlMode << endl;

	QFile fl( mPath );
	//opens and eventually create the file
	mUndoBuffer->setInsideUndo( true );
	if ( fl.open( IO_ReadOnly ) ) {
		QTextStream stream( &fl );
		while ( !stream.atEnd() )
			appendLine( stream.readLine() );
		fl.close();
	}
	if ( ! mText.count() )
		appendLine("");
	mUndoBuffer->setInsideUndo( false );
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
	yzDebug("YZBuffer") << "Saving file to " << mPath << endl;
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		// do not save empty buffer to avoid creating a file
		// with only a '\n' while the buffer is emtpy
		if ( isEmpty() == false) {
			QValueVector<YZLine*>::iterator it;
			for ( it = mText.begin(); it != mText.end(); it++ ) {
				stream << (*it )->data() << "\n";
			}
		}
		file.close();
	}
	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->displayInfo(tr("Written %1 bytes to file %2").arg(getWholeTextLength()).arg(mPath));
	setModified( false );
	filenameChanged();
	//clear swap memory
	mSwap->reset();
	mSwap->unlink();
	int hlMode = YzisHlManager::self()->detectHighlighting (this);
	if ( hlMode >= 0 && m_highlight != YzisHlManager::self()->getHl( hlMode ) ) {
		setHighLight( hlMode );
	}
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
	yzDebug("YZBuffer") << "BUFFER: addView" << endl;
	mViews.append( v );
	mSession->setCurrentView( v );
}

YZView* YZBuffer::findView( unsigned int uid ) {
	yzDebug("YZBuffer") << "Buffer: findView " << uid << endl;
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() ){
//		yzDebug("YZBuffer") << "buffer:findViewChecking view " << uid << " for buffer " << fileName() << endl;
		if ( it->myId == uid ) {
//			yzDebug("YZBuffer") << "Buffer:findView " << uid << " found" << endl;
			return it;
		}
	}
//	yzDebug("YZBuffer") << "buffer::findView " << uid << " returning NULL" << endl;
	return NULL;
}

void YZBuffer::updateAllViews() {
	if ( !mUpdateView ) return;
	yzDebug("YZBuffer") << "YZBuffer updateAllViews" << endl;
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() )
		it->refreshScreen();
}

YZView* YZBuffer::firstView() {
	if (  mViews.first() != NULL ) 
		return mViews.first();
	else yzDebug("YZBuffer") << "No VIEW !!!" << endl;
	return NULL;//crash me :)
}

void YZBuffer::rmView(YZView *v) {
	int f = mViews.remove(v);
	YZASSERT( 1==f ); // isn't it ?
	yzDebug("YZBuffer") << "YZBuffer removeView found " << f << " views" << endl;
	if ( mViews.isEmpty() ) 
		detach();

}

// ------------------------------------------------------------------------
//                            Undo/Redo Operations
// ------------------------------------------------------------------------

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

	bool ctxChanged = true;
	unsigned int hlLine = 0;
	while ( hlLine < lineCount()) {
		QMemArray<signed char> foldingList;
		m_highlight->doHighlight( ( hlLine >= 1 ? yzline( hlLine -1 ) : new YZLine()), yzline( hlLine ), &foldingList, &ctxChanged );
		hlLine++;
	}
	updateAllViews();

}

void YZBuffer::setPath( const QString& _path ) {
	QString newPath = _path.stripWhiteSpace();
	if (newPath[0] != '/') {
		mPath = QDir::cleanDirPath(QDir::current().absPath()+"/"+newPath);
		yzDebug("YZBuffer") << "Changing path to absolute " << mPath << endl;
	} else
		mPath = newPath; 
	mFileIsNew=false; 
	mSwap->setFileName( mPath + ".ywp" );
	filenameChanged();
}

bool YZBuffer::substitute( const QString& what, const QString& with, bool wholeline, unsigned int line ) {
	QString l = textline( line );
	QRegExp rx( what );
	int pos=0;
	int offset=0;
	while ( ( pos = rx.search( l,offset ) ) != -1 ) {
		l = l.replace( pos, rx.matchedLength(), with );
		offset+=pos+with.length();
		if ( !wholeline ) break;
	}
	if ( offset ) {
		setTextline( line,l );
		return true;
	}
	return false;
}

QStringList YZBuffer::getText(YZCursor& from, YZCursor& to) {
	//the first line
	QStringList list;
	if ( from.getY() != to.getY() )
		list << textline( from.getY() ).mid( from.getX() );
	else
		list << textline( from.getY() ).mid( from.getX(), to.getX() - from.getX() + 1 );

	//other lines
	unsigned int i = from.getY() + 1;
	while ( i < to.getY() ) {
		list << textline( i ); //the whole line	
		i++;
	}
	
	//last line
	if ( from.getY() != to.getY() )
		list << textline( to.getY() ).left( to.getX() );

	return list;
}

void YZBuffer::clearSwap() {
	mSwap->unlink();
}

int YZBuffer::getLocalIntOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mPath+"\\"+option ) ) //find the local one ?
		return YZSession::mOptions.readIntEntry( mPath+"\\"+option, 0 );
	else
		return YZSession::mOptions.readIntEntry( "Global\\" + option, 0 ); // else give the global default if any
}

void YZBuffer::setLocalIntOption( const QString& key, int option ) {
	YZSession::mOptions.setGroup(mPath);
	YZSession::mOptions.setIntOption( key, option );
}

bool YZBuffer::getLocalBoolOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mPath+"\\"+option ) )
		return YZSession::mOptions.readBoolEntry( mPath+"\\"+option, false );
	else
		return YZSession::mOptions.readBoolEntry( "Global\\" + option, false );
}

void YZBuffer::setLocalBoolOption( const QString& key, bool option ) {
	YZSession::mOptions.setGroup(mPath);
	YZSession::mOptions.setBoolOption( key, option );
}

QString YZBuffer::getLocalStringOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mPath+"\\"+option ) )
		return YZSession::mOptions.readQStringEntry( mPath+"\\"+option, QString("") );
	else
		return YZSession::mOptions.readQStringEntry( "Global\\" + option, QString("") );
}

void YZBuffer::setLocalQStringOption( const QString& key, const QString& option ) {
	YZSession::mOptions.setGroup(mPath);
	YZSession::mOptions.setQStringOption( key, option );
}

QStringList YZBuffer::getLocalStringListOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mPath+"\\"+option ) )
		return YZSession::mOptions.readQStringListEntry( mPath+"\\"+option, QStringList::split(";","") );
	else
		return YZSession::mOptions.readQStringListEntry( "Global\\" + option, QStringList::split(";","") );
}

void YZBuffer::setLocalQStringListOption( const QString& key, const QStringList& option ) {
	YZSession::mOptions.setGroup(mPath);
	YZSession::mOptions.setQStringListOption( key, option );
}

QColor YZBuffer::getLocalColorOption( const QString& option ) {
	if ( YZSession::mOptions.hasOption( mPath+"\\"+option ) )
		return YZSession::mOptions.readQColorEntry( mPath+"\\"+option, QColor("white") );
	else
		return YZSession::mOptions.readQColorEntry( "Global\\" + option, QColor("white") );
}

void YZBuffer::setLocalQColorOption( const QString& key, const QColor& option ) {
	YZSession::mOptions.setGroup(mPath);
	YZSession::mOptions.setQColorOption( key, option );
}

