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
#include <qtextcodec.h>

#include "buffer.h"
#include "line.h"
#include "view.h"
#include "undo.h"
#include "debug.h"
#include "action.h"
#include "internal_options.h"
#include "mark.h"
#include "swapfile.h"
#include "session.h"

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
	m_hlupdating = false;
	// buffer at creation time should use a non existing temp filename
	// find a tmp file that does not exist
	do {
		mPath = QString("/tmp/yzisnew%1").arg(random());
	} while ( QFileInfo( mPath ).exists() == true );
	// there is still a possible race condition here...
	mFileIsNew = true;
	mUndoBuffer = new YZUndoBuffer( this );
	mAction = new YZAction( this );
	mViewMarks = new YZViewMark( );
	mDocMarks = new YZDocMark( );
	currentEncoding = getLocalStringOption( "encoding" );
	displayIntro();
	YZSession::me->addBuffer( this );
	mSwap = new YZSwapFile( this );
	mLoading = false;
	yzDebug("YZBuffer") << "NEW BUFFER CREATED : " << mPath << endl;
}

YZBuffer::~YZBuffer() {
	//remove swap file
	mSwap->unlink();
	delete mSwap;
	if ( m_highlight != 0L )
		m_highlight->release();

	QValueVector<YZLine*>::iterator it;
	for ( it = mText.begin(); it != mText.end(); it++ ) {
		delete ( *it );
	}
	mText.clear();
	delete mUndoBuffer;
	delete mAction;
	delete mDocMarks;
	delete mViewMarks;
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

/**
 * WARNING! Here are elementary buffer operations only! 
 * do _not_ use them directly, use action() ( actions.cpp ) instead.
 */

#define VIEWS_INIT( x, y ) \
	for ( YZView *it = mViews.first(); it; it = mViews.next() ) \
		it->initChanges( x, y )

#define VIEWS_APPLY( x, y ) \
	for ( YZView *it = mViews.first(); it; it = mViews.next() ) \
		it->applyChanges( x, y )

void YZBuffer::insertChar(unsigned int x, unsigned int y, const QString& c ) {
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


	VIEWS_INIT( x, y );

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, c, x, y );
	if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::ADDTEXT, c, x, y );

	l.insert(x, c);
	setTextline(y,l);

	VIEWS_APPLY( x + c.length(), y );
}

void YZBuffer::delChar (unsigned int x, unsigned int y, unsigned int count ) {
	ASSERT_LINE_EXISTS( QString("YZBuffer::delChar(%1,%2,%3)").arg(x).arg(y).arg(count), y )

	/* brute force, we'll have events specific for that later on */
	QString l=textline(y);
	if (l.isNull()) return;

	if (x >= l.length())
		return;

	ASSERT_COL_LINE_EXISTS( QString("YZBuffer::delChar(%1,%2,%3)").arg(x).arg(y).arg(count),x,y)

	VIEWS_INIT( x, y );

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, l.mid(x,count), x, y );
	if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::DELTEXT, l.mid( x,count ), x, y );

	/* do the actual modification */
	l.remove(x, count);

	setTextline(y,l);

	VIEWS_APPLY( x, y );
}

// ------------------------------------------------------------------------
//                            Line Operations
// ------------------------------------------------------------------------

void  YZBuffer::appendLine(const QString &l) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::appendLine(%1)").arg(l),l);
	
	if ( !mLoading ) {
		mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, QString(), 0, lineCount() );
		mSwap->addToSwap( YZBufferOperation::ADDLINE, QString(), 0, lineCount() );
		mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, l,  0, lineCount());
		mSwap->addToSwap( YZBufferOperation::ADDTEXT, l, 0, lineCount() );
	}

	mText.append(new YZLine(l));
	if ( !mLoading && m_highlight != 0L ) {
		bool ctxChanged = false;
		QMemArray<signed char> foldingList;
		m_highlight->doHighlight(( mText.count() >= 2 ? yzline( mText.count() - 2 ) : new YZLine()), yzline( mText.count() - 1 ), &foldingList, &ctxChanged );
//		if ( ctxChanged ) yzDebug("YZBuffer") << "CONTEXT changed"<<endl; //no need to take any action at EOF ;)
	}

	setChanged( true );
}


void  YZBuffer::insertLine(const QString &l, unsigned int line) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_NEXT_LINE_EXISTS(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),line)
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, QString(), 0, line );
	if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::ADDLINE, QString(), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, l, 0, line );
	if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::ADDTEXT, l, 0, line );

	VIEWS_INIT( 0, line - 1 );

	QValueVector<YZLine*>::iterator it;
	uint idx=0;
	for ( it = mText.begin(); idx < line && it != mText.end(); it++, idx++ )
		;
	mText.insert(it, new YZLine( l ));
	if ( !mLoading && m_highlight != 0L ) {
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

	setChanged( true );

	VIEWS_APPLY( 0, line );
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
	VIEWS_INIT( col, line );

	if ( line >= lineCount() ) return;
	QString l=textline(line);
	if (l.isNull()) return;

	ASSERT_PREV_COL_LINE_EXISTS(QString("YZBuffer::insertNewLine(%1,%2)").arg(col).arg(line),col,line )

	if (col > l.length() ) return;

	QString newline = l.mid( col );
	if ( newline.isNull() ) newline = QString( "" );

	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, "", col, line+1 );
	if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::ADDLINE, "", col, line+1 );
	if (newline.length()) {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, newline, col, line );
		mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, newline, 0, line+1 );
		if ( !mLoading ) {
			mSwap->addToSwap( YZBufferOperation::DELTEXT, newline, col, line );
			mSwap->addToSwap( YZBufferOperation::ADDTEXT, newline, 0, line+1 );
		}
	}

	//replace old line
	setTextline(line,l.left( col ));

	//add new line
	QValueVector<YZLine*>::iterator it;
	uint idx=0;
	for ( it = mText.begin(); idx < line+1 && it != mText.end(); it++, idx++ )
		;
	mText.insert(it, new YZLine( newline ));

	if ( !mLoading && m_highlight != 0L ) {
		uint hlLine = line+1;
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

	VIEWS_APPLY( 0, line+1 );
}

void YZBuffer::deleteLine( unsigned int line ) {
	ASSERT_LINE_EXISTS(QString("YZBuffer::deleteLine(%1)").arg(line),line)

	if (line >= lineCount()) return;

	VIEWS_INIT( 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, textline(line), 0, line );
	if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::DELTEXT, textline( line ), 0, line );
	if (lineCount() > 1) {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELLINE, "", 0, line );
		if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::DELLINE, "", 0, line );
		QValueVector<YZLine*>::iterator it;
		uint idx=0;
		for ( it = mText.begin(); idx < line && it != mText.end(); it++, idx++ )
			;
		delete (*it);
		mText.erase(it);
	} else {
		mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, "", 0, line );
		if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::DELTEXT, "", 0, line );
		setTextline(0,"");
	}

	setChanged( true );

	VIEWS_APPLY( 0, line + 1 );
}

void YZBuffer::replaceLine( const QString& l, unsigned int line ) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::replaceLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_LINE_EXISTS(QString("YZBuffer::replaceLine(%1,%2)").arg(l).arg(line),line)

	VIEWS_INIT( 0, line );
	if ( line >= lineCount() ) return;
	if ( textline( line ).isNull() ) return;

	mUndoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, textline(line), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, l, 0, line );
	if ( !mLoading ) {
		mSwap->addToSwap( YZBufferOperation::DELTEXT, textline( line ), 0, line );
		mSwap->addToSwap( YZBufferOperation::ADDTEXT, l, 0, line );
	}
	setTextline(line,l);

	VIEWS_APPLY( l.length(), line );
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
	//clear is fine but better _delete_ all yzlines too ;)
	QValueVector<YZLine*>::iterator it;
	for ( it = mText.begin(); it != mText.end(); it++ ) {
		delete ( *it );
	}
	mText.clear(); //remove the _pointers_ now
	mText.append(new YZLine());
}

void YZBuffer::clearIntro() {
	if (mIntro) {
		yzDebug("YZBuffer") << "YZBuffer clearIntro" << endl;
		mIntro = false;
		clearText();
		updateAllViews();
	}
}

void YZBuffer::displayIntro() {
	yzDebug("YZBuffer") << "YZBuffer displayIntro" << endl;
	QStringList introduction;
	introduction
	<<  ""
	<<  ""
	<<  ""
	<< VERSION_CHAR_LONG;
	if (VERSION_CHAR_ST == VERSION_CHAR_STATE2)
		introduction << VERSION_CHAR_DATE;
	introduction << VERSION_CHAR_ST
	<<  ""
	<<  "http://www.yzis.org"
	<<  "Contact/Patches/Requests: yzis-dev@yzis.org"
	<<  ""
	<<  "Yzis is distributed under the terms of the GPL v2"
	<<  ""
	<<  "Please report bugs at http://bugs.yzis.org";

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
			yzline(line)->setData("");
		} else {
			yzline(line)->setData(l);
		}
	}
	if ( !mLoading && m_highlight != 0L ) {
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
	setChanged( true );
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

void YZBuffer::setEncoding( const QString& name ) {
	yzDebug("YZBuffer") << "set encoding " << name << endl;
	QTextCodec* destCodec;
	QTextCodec* fromCodec;
	if ( name == "locale" ) {
		destCodec = QTextCodec::codecForLocale();
	} else {
		destCodec = QTextCodec::codecForName( name );
	}
	if ( currentEncoding == "locale" ) {
		fromCodec = QTextCodec::codecForLocale();
	} else {
		fromCodec = QTextCodec::codecForName( currentEncoding );
	}
	if ( ! isEmpty() ) {
		QValueVector<YZLine*>::iterator it;
		for ( it = mText.begin(); it != mText.end(); it++ ) {
			(*it)->setData( destCodec->toUnicode( fromCodec->fromUnicode( (*it)->data() ) ) );
		}
	}
	currentEncoding = name;
}

void YZBuffer::load(const QString& file) {
	yzDebug("YZBuffer") << "YZBuffer load " << file << endl;
	if ( file.isNull() || file.isEmpty() ) return;
	setPath(file);
	if ( mIntro ) clearIntro();
	//stop redraws
	mUpdateView=false;

	QValueVector<YZLine*>::iterator it;
	for ( it = mText.begin(); it != mText.end(); it++ ) {
		delete ( *it );
	}
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
	mLoading=true;
	currentEncoding = getLocalStringOption( "encoding" );
	if ( fl.open( IO_ReadOnly ) ) {
		QTextCodec* codec;
		if ( currentEncoding == "locale" ) {
			codec = QTextCodec::codecForLocale();
		} else {
			codec = QTextCodec::codecForName( currentEncoding );
		}
		QTextStream stream( &fl );
		stream.setCodec( codec );
		while ( !stream.atEnd() )
			appendLine( stream.readLine() );
		fl.close();
	}
	if ( ! mText.count() )
		appendLine("");
	setChanged( false );
	//check for a swap file left after a crash
	mSwap->setFileName( mPath + ".ywp" );
	if ( QFile::exists( mPath + ".ywp" ) ) {//if it already exists, recover from it
		if ( YZSession::me->promptYesNo(tr("Recover"),tr("A swap file was found for this file, it was presumably created because your computer or yzis crashed, do you want to start the recovery of this file ?")) ) {
			if ( mSwap->recover() )
				setChanged( true );
		}
	}
	mSwap->init(); // whatever happened before, create a new swapfile
	mLoading=false;
	mUndoBuffer->setInsideUndo( false );
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

	QString codecName = getLocalStringOption( "fileencoding" );
	yzDebug("YZBuffer") << "save using " << codecName << " encoding" << endl;
	QTextCodec* codec;
	if ( codecName == "locale" ) {
		codec = QTextCodec::codecForLocale();
	} else {
		codec = QTextCodec::codecForName( codecName );
	}

	QFile file( mPath );
	m_hlupdating = true; //override so that it does not parse all lines
	yzDebug("YZBuffer") << "Saving file to " << mPath << endl;
	if ( file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		stream.setCodec( codec );
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
	m_hlupdating = false; //override so that it does not parse all lines
	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->displayInfo(tr("Written %1 bytes to file %2").arg(getWholeTextLength()).arg(mPath));
	setChanged( false );
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
	for ( it = mViews.first(); it; it = mViews.next() ) {
		it->sendPaintEvent( it->getDrawCurrentLeft(), it->getDrawCurrentTop(), it->getColumnsVisible(), it->getLinesVisible() );
		it->syncViewInfo();
	}
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

void YZBuffer::setChanged( bool modif ) {
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
		highlightingChanged();
	}
}

void YZBuffer::makeAttribs() {
	m_highlight->clearAttributeArrays();

	bool ctxChanged = true;
	unsigned int hlLine = 0;
	if ( !mLoading )
		while ( hlLine < lineCount()) {
			QMemArray<signed char> foldingList;
			m_highlight->doHighlight( ( hlLine >= 1 ? yzline( hlLine -1 ) : new YZLine()), yzline( hlLine ), &foldingList, &ctxChanged );
			hlLine++;
		}
	updateAllViews();

}

void YZBuffer::setPath( const QString& _path ) {
	QString newPath = _path.stripWhiteSpace();
	QString oldPath = mPath;
	if (newPath[0] != '/') {
		mPath = QDir::cleanDirPath(QDir::current().absPath()+"/"+newPath);
		yzDebug("YZBuffer") << "Changing path to absolute " << mPath << endl;
	} else
		mPath = newPath;
	mFileIsNew=false;
	//hmm changing file :), update Session !!!!
	mSession->updateBufferRecord( oldPath, mPath, this );
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
	m_hlupdating=true; //override
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

	m_hlupdating=false; //override
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

void YZBuffer::updateHL( unsigned int line ) {
	if ( m_hlupdating ) return;
//	yzDebug() << "updateHL " << line << endl;
	m_hlupdating = true;
	if ( m_highlight != 0L ) {
		uint hlLine = line;
		bool ctxChanged = true;
		QMemArray<signed char> foldingList;
		m_highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine -1 ) : new YZLine()), yzline( hlLine ), &foldingList, &ctxChanged );
	}
	m_hlupdating=false;
}
