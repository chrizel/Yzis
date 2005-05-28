/* This file is part of the Yzis libraries
 *  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
 *  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
 *  Copyright (C) 2003-2004 Philippe Fremy <pfremy@freehackers.org>
 *  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
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

// for tildeExpand
#include <sys/types.h>
#include <sys/stat.h>

#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qtextcodec.h>

#include "portability.h"
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
#include "ex_lua.h"

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
	mUpdateView=true;
	mSession = sess;
	mModified=false;
	m_highlight = 0L;
	m_hlupdating = false;
	// buffer at creation time should use a non existing temp filename
	// find a tmp file that does not exist
	do {
		mPath = QString("/tmp/yzisnew%1").arg(rand());
	} while ( QFileInfo( mPath ).exists() == true );
	// there is still a possible race condition here...
	mFileIsNew = true;
	mUndoBuffer = new YZUndoBuffer( this );
	mAction = new YZAction( this );
	mViewMarks = new YZViewMark( );
	mDocMarks = new YZDocMark( );
	currentEncoding = getLocalStringOption( "encoding" );
	YZSession::me->addBuffer( this );
	mSwap = new YZSwapFile( this );
	mLoading = false;
	mText.append( new YZLine() );
	setHighLight( 0, false );
	yzDebug("YZBuffer") << "NEW BUFFER CREATED : " << mPath << endl;
}

YZBuffer::~YZBuffer() {
	//remove swap file
	mSwap->unlink();
	delete mSwap;
	if ( m_highlight != 0L )
		m_highlight->release();

	QValueVector<YZLine*>::iterator it = mText.begin(), end = mText.end();
	for ( ; it != end; ++it )
		delete ( *it );
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
	{ for ( YZView *it = mViews.first(); it; it = mViews.next() ) \
	it->initChanges( x, y ); }

#define VIEWS_APPLY( x, y ) \
	{ for ( YZView *it = mViews.first(); it; it = mViews.next() ) \
	it->applyChanges( x, y ); }

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
		QMemArray<uint> foldingList;
		YZLine *l = new YZLine();
		m_highlight->doHighlight(( mText.count() >= 2 ? yzline( mText.count() - 2 ) : l), yzline( mText.count() - 1 ), &foldingList, &ctxChanged );
		delete l;
//		if ( ctxChanged ) yzDebug("YZBuffer") << "CONTEXT changed"<<endl; //no need to take any action at EOF ;)
	}
	YZSession::me->search()->highlightLine( this, mText.count() - 1 );

	setChanged( true );
}


void  YZBuffer::insertLine(const QString &l, unsigned int line) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_NEXT_LINE_EXISTS(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),line)
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, QString(), 0, line );
	if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::ADDLINE, QString(), 0, line );
	mUndoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, l, 0, line );
	if ( !mLoading ) mSwap->addToSwap( YZBufferOperation::ADDTEXT, l, 0, line );

	VIEWS_INIT( 0, line );

	QValueVector<YZLine*>::iterator it = mText.begin(), end = mText.end();
	uint idx=0;
	for ( ; idx < line && it != end; ++it, ++idx )
		;
	mText.insert(it, new YZLine( l ));

	YZSession::me->search()->shiftHighlight( this, line, 1 );
	YZSession::me->search()->highlightLine( this, line );
	updateHL( line );

	setChanged( true );

	VIEWS_APPLY( 0, line + 1 );
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

	//add new line
	QValueVector<YZLine*>::iterator it = mText.begin(), end = mText.end();
	uint idx=0;
	for ( ; idx < line+1 && it != end; ++it, ++idx )
		;
	mText.insert(it, new YZLine( newline ));

	YZSession::me->search()->shiftHighlight( this, line+1, 1 );
	YZSession::me->search()->highlightLine( this, line+1 );
	//replace old line
	setTextline(line,l.left( col ));
	updateHL( line + 1 );

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
		QValueVector<YZLine*>::iterator it = mText.begin(), end = mText.end();
		uint idx=0;
		for ( ; idx < line && it != end; ++it, ++idx )
			;
		delete (*it);
		mText.erase(it);

		YZSession::me->search()->shiftHighlight( this, line+1, -1 );
		YZSession::me->search()->highlightLine( this, line );
		updateHL( line );
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

	if ( line >= lineCount() ) return;
	if ( textline( line ).isNull() ) return;
	VIEWS_INIT( 0, line );

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
	QValueVector<YZLine*>::iterator it = mText.begin(), end = mText.end();
	for ( ; it != end; ++it )
		delete ( *it );
	mText.clear(); //remove the _pointers_ now
	mText.append(new YZLine());
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
	updateHL( line );
	YZSession::me->search()->highlightLine( this, line );
	setChanged( true );
}

// XXX Wrong
bool YZBuffer::isLineVisible(uint line) {
	bool shown=false;
	YZView *it;
	for ( it = mViews.first(); it && !shown; it = mViews.next() )
		shown = shown || it->isLineVisible(line);
	return shown;
}

bool YZBuffer::isEmpty() const {
	return ( mText.count() == 1 && textline(0).isEmpty() );
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
	if (s.isEmpty() ) return 0;
	while( s.at(i).isSpace() && i < s.length())
		i++;
	return i;
}

// ------------------------------------------------------------------------
//                            File Operations
// ------------------------------------------------------------------------

void YZBuffer::setEncoding( const QString& name ) {
	yzDebug("YZBuffer") << "set encoding " << name << endl;
	/*
	 * We have to reload the file
	 */
	load( mPath );
/*	//Does not work very well, problem when converting from utf8 to iso8859-15, and problem with the EOL
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
		QValueVector<YZLine*>::iterator it = mText.begin(), end = mText.end();
		for ( ; it != end; ++it ) {
			(*it)->setData( destCodec->toUnicode( fromCodec->fromUnicode( (*it)->data() ) ) );
		}
	}
	currentEncoding = name; */
}

void YZBuffer::loadText( QString* content ) {
	mText.clear(); //remove the _pointers_ now
	QTextStream stream( content, IO_ReadOnly );
	while ( !stream.atEnd() )
		appendLine( stream.readLine() );
}

void YZBuffer::load(const QString& file) {
	yzDebug("YZBuffer") << "YZBuffer load " << file << endl;
	if ( file.isNull() || file.isEmpty() ) return;
	setPath(file);

	//stop redraws
	mUpdateView=false;

	QValueVector<YZLine*>::iterator it = mText.begin(), end = mText.end();
	for ( ; it != end; ++it )
		delete ( *it );
	mText.clear();
	mFileIsNew=false;
	

	unsigned int scrollTo = 0;
	QRegExp reg = QRegExp( "(.+):(\\d+):?" );
	if ( reg.exactMatch( mPath ) && QFile::exists( reg.cap( 1 ) ) ) {
		mPath = reg.cap( 1 );
		scrollTo = reg.cap( 2 ).toUInt();
	}
	QFile fl( mPath );

	//HL mode selection
	detectHighLight();
		
	//opens and eventually create the file
	mUndoBuffer->setInsideUndo( true );
	mLoading=true;
	currentEncoding = getLocalStringOption( "encoding" );
	if ( fl.open( IO_ReadOnly ) ) {
		QTextCodec* codec;
		if ( currentEncoding == "locale" ) {
			codec = QTextCodec::codecForLocale();
/*			char *buff = (char*)malloc( 102400 * sizeof(char));
			int readl = fl.readBlock(buff,102400);
			QTextCodec *c = QTextCodec::codecForContent (buff,102400);
			free(buff);
			fl.reset();
			yzDebug() << "Detected encoding " << c->name()  << " by reading " << readl << " bytes." << endl;
			if ( readl > 0 && c && c->name() != codec->name() ) {
				codec = c;
				setLocalQStringOption("encoding", c->name());
				setLocalQStringOption("fileencoding", c->name());
			}*/ //not reliable enough
		} else {
			codec = QTextCodec::codecForName( currentEncoding );
		}
		QTextStream stream( &fl );
		stream.setCodec( codec );
		while ( !stream.atEnd() )
			appendLine( stream.readLine() );
		fl.close();
    } else if (QFile::exists(mPath)) {
		YZSession::me->popupMessage(_("Failed opening file %1 for reading : %2").arg(mPath).arg(fl.errorString()));
	}
	if ( ! mText.count() )
		appendLine("");
	setChanged( false );
	//check for a swap file left after a crash
	mSwap->setFileName( mPath );
	if ( QFile::exists( mSwap->filename() ) ) { //if it already exists, recover from it
		struct stat buf;
		int i = stat( mPath.local8Bit(), &buf );
		if ( i != -1 && S_ISREG( buf.st_mode ) && CHECK_GETEUID( buf.st_uid )  ) {
			if ( YZSession::me->promptYesNo(_("Recover"),_("A swap file was found for this file, it was presumably created because your computer or yzis crashed, do you want to start the recovery of this file ?")) ) {
				if ( mSwap->recover() )
					setChanged( true );
			}
		}
	}
//	mSwap->init(); // whatever happened before, create a new swapfile
	mLoading=false;
	mUndoBuffer->setInsideUndo( false );
	//reenable
	mUpdateView=true;
	updateAllViews();
	if ( scrollTo > 0 ) {
		YZView *it;
		for ( it = mViews.first(); it; it = mViews.next() )
			it->gotoStickyCol( scrollTo - 1 );
	}
	filenameChanged();

	YZSession::me->getYzisinfo()->readYzisinfo();
	YZCursor * tmp = YZSession::me->getYzisinfo()->startPosition();
	
	YZView *hit;
	for ( hit = mViews.first(); hit; hit = mViews.next() )
		if ( tmp ) {
			hit->gotodxdy(tmp->x(), tmp->y(), true );
		}
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
	if ( codecName.isEmpty() )
		codecName = getLocalStringOption( "encoding" );
	yzDebug("YZBuffer") << "save using " << codecName << " encoding" << endl;
	QTextCodec* codec;
	if ( codecName == "locale" ) {
		codec = QTextCodec::codecForLocale();
	} else {
		codec = QTextCodec::codecForName( codecName );
	}
	// XXX: we have to test if codec is null, then  alert the user (like we have to do with null yzline()

	QFile file( mPath );
	m_hlupdating = true; //override so that it does not parse all lines
	yzDebug("YZBuffer") << "Saving file to " << mPath << endl;
	if ( codec && file.open( IO_WriteOnly ) ) {
		QTextStream stream( &file );
		stream.setCodec( codec );
		// do not save empty buffer to avoid creating a file
		// with only a '\n' while the buffer is emtpy
		if ( isEmpty() == false) {
			QValueVector<YZLine*>::iterator it = mText.begin(), end = mText.end();
			for ( ; it != end; ++it ) {
				stream << (*it )->data() << "\n";
			}
		}
		file.close();
	} else {
		YZSession::me->popupMessage(_("Failed opening file %1 for writing : %2").arg(mPath).arg(file.errorString()));
		m_hlupdating = true;
		return false;
	}
	m_hlupdating = false; //override so that it does not parse all lines
	YZView *it;
	for ( it = mViews.first(); it ; it = mViews.next() )
		it->displayInfo(_("Written %1 bytes to file %2").arg(getWholeTextLength()).arg(mPath));
	setChanged( false );
	filenameChanged();
	//clear swap memory
	mSwap->reset();
	mSwap->unlink();
	
	YZSession::me->getYzisinfo()->saveStartPosition( mPath, 
                  (YZSession::me->currentView())->getCursor()->x(),
                  (YZSession::me->currentView())->getCursor()->y() );
				                  
	YZSession::me->getYzisinfo()->writeYzisinfo();
   
	int hlMode = YzisHlManager::self()->detectHighlighting (this);
	if ( hlMode >= 0 && m_highlight != YzisHlManager::self()->getHl( hlMode ) )
		setHighLight( hlMode );
	return true;
}

// ------------------------------------------------------------------------
//                            View Operations
// ------------------------------------------------------------------------

void YZBuffer::addView (YZView *v) {
	YZView *it;
	for ( it = mViews.first(); it; it=mViews.next() )
		if ( it == v ) {
			yzWarning()<< "view " << v->myId << " added for the second time, discarding"<<endl;
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
		if ( it->myId == uid )
			return it;
	}
//	yzDebug("YZBuffer") << "buffer::findView " << uid << " returning NULL" << endl;
	return NULL;
}

void YZBuffer::updateAllViews() {
	if ( !mUpdateView ) return;
	yzDebug("YZBuffer") << "YZBuffer updateAllViews" << endl;
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() ) {
		it->sendRefreshEvent();
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
	mViews.remove(v);
//	yzDebug("YZBuffer") << "YZBuffer removeView found " << f << " views" << endl;
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
	setModified( modif );
}
void YZBuffer::setModified( bool ) {
}

void YZBuffer::statusChanged() {
	//update all views
	YZView *it;
	for ( it = mViews.first(); it; it = mViews.next() )
		it->syncViewInfo();
}


// ------------------------------------------------------------------------
//                            Syntax Highlighting
// ------------------------------------------------------------------------

void YZBuffer::setHighLight( uint mode, bool warnGUI ) {
	YzisHighlighting *h = YzisHlManager::self()->getHl( mode );

	if ( h != m_highlight ) { //HL is changing
		if ( m_highlight != 0L )
			m_highlight->release(); //free memory

		//init
		h->use();

		m_highlight = h;

		makeAttribs();
		if ( warnGUI )
			highlightingChanged();

		//load indent plugin
		//XXX should we check whether it was already loaded ?
		QString hlName = h->name();
		hlName.replace("+","p");
		YZExLua::instance()->source(NULL, hlName.lower(),false);
	}
}

void YZBuffer::setHighLight( const QString& name ) {
	int hlMode = YzisHlManager::self()->nameFind( name );
	if ( hlMode > 0 )
		setHighLight( hlMode, true );
}


void YZBuffer::makeAttribs() {
	m_highlight->clearAttributeArrays();

	bool ctxChanged = true;
	unsigned int hlLine = 0;
	if ( !mLoading )
		while ( hlLine < lineCount()) {
			QMemArray<uint> foldingList;
			YZLine *l = new YZLine();
			m_highlight->doHighlight( ( hlLine >= 1 ? yzline( hlLine -1 ) : l), yzline( hlLine ), &foldingList, &ctxChanged );
			delete l;
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
	YZSession::mOptions->updateOptions(oldPath, mPath);
	filenameChanged();
}

bool YZBuffer::substitute( const QString& _what, const QString& with, bool wholeline, unsigned int line ) {
	QString l = textline( line );
	bool cs = true;
	QString what = _what;
	if ( what.endsWith("\\c") ) {
		what.truncate(what.length()-2);
		cs = false;
	}
	QRegExp rx( what );
	rx.setCaseSensitive(cs);
	bool changed = false;
	int pos=0;
	int offset=0;
	while ( ( pos = rx.search( l,offset ) ) != -1 ) {
		l = l.replace( pos, rx.matchedLength(), with );
		changed = true;
		offset = pos + with.length();
		if ( !wholeline ) break;
	}
	if ( changed ) {
		setTextline( line,l );
		return true;
	}
	return false;
}

QStringList YZBuffer::getText(const YZCursor& from, const YZCursor& to) {
	m_hlupdating=true; //override
	//the first line
	QStringList list;
	if ( from.y() != to.y() )
		list << textline( from.y() ).mid( from.x() );
	else
		list << textline( from.y() ).mid( from.x(), to.x() - from.x() + 1 );

	//other lines
	unsigned int i = from.y() + 1;
	while ( i < to.y() ) {
		list << textline( i ); //the whole line
		i++;
	}

	//last line
	if ( from.y() != to.y() )
		list << textline( to.y() ).left( to.x() + 1 );

	m_hlupdating=false; //override
	return list;
}
QStringList YZBuffer::getText( const YZInterval& i ) {
	YZCursor from, to;
	intervalToCursors( i, &from, &to );
	return getText( from, to );
}

void YZBuffer::intervalToCursors( const YZInterval& i, YZCursor* from, YZCursor* to ) {
	*from = i.fromPos();
	*to = i.toPos();
	if ( i.from().opened() )
		from->setX( from->x() + 1 );
	if ( i.to().opened() ) {
		if ( to->x() > 0 ) {
			to->setX( to->x() - 1 );
		} else if ( to->y() > 0 ) {
			to->setY( to->y() - 1 );
			to->setX( textline(to->y()).length() - 1 );
		}
	}
}


QString YZBuffer::getWordAt( const YZCursor& at ) {
	QString l = textline( at.y() );
	QRegExp reg( "\\b(\\w+)\\b" );
	int idx = reg.searchRev( l, at.x() );
	if ( idx == -1 || idx + reg.cap( 1 ).length() <= at.x() ) {
		idx = reg.search( l, at.x() );
		if ( idx >= 0 ) return reg.cap( 1 );
		else {
			reg.setPattern( "(^|[\\s\\w])([^\\s\\w]+)([\\s\\w]|$)" );
			idx = reg.searchRev( l, at.x() );
			if ( idx == -1 || idx + reg.cap( 1 ).length() + reg.cap( 2 ).length() <= at.x() ) {
				idx = reg.search( l, at.x() );
				if ( idx >= 0 ) return reg.cap( 2 );
			} else {
				return reg.cap( 2 );
			}
		}
	} else {
		return reg.cap( 1 );
	}
	return QString::null;
}

void YZBuffer::clearSwap() {
	mSwap->unlink();
}

int YZBuffer::getLocalIntegerOption( const QString& option ) {
	if ( YZSession::mOptions->hasOption( mPath+"\\"+option ) ) //find the local one ?
		return YZSession::mOptions->readIntegerOption( mPath+"\\"+option, 0 );
	else
		return YZSession::mOptions->readIntegerOption( "Global\\" + option, 0 ); // else give the global default if any
}

bool YZBuffer::getLocalBooleanOption( const QString& option ) {
	if ( YZSession::mOptions->hasOption( mPath+"\\"+option ) )
		return YZSession::mOptions->readBooleanOption( mPath+"\\"+option, false );
	else
		return YZSession::mOptions->readBooleanOption( "Global\\" + option, false );
}

QString YZBuffer::getLocalStringOption( const QString& option ) {
	if ( YZSession::mOptions->hasOption( mPath+"\\"+option ) )
		return YZSession::mOptions->readStringOption( mPath+"\\"+option );
	else
		return YZSession::mOptions->readStringOption( "Global\\" + option );
}

QStringList YZBuffer::getLocalListOption( const QString& option ) {
	if ( YZSession::mOptions->hasOption( mPath+"\\"+option ) )
		return YZSession::mOptions->readListOption( mPath+"\\"+option, QStringList() );
	else
		return YZSession::mOptions->readListOption( "Global\\" + option, QStringList() );
}

bool YZBuffer::updateHL( unsigned int line ) {
//	yzDebug() << "updateHL " << line << endl;
	if ( mLoading ) return false;
	unsigned int hlLine = line, nElines = 0;
	bool ctxChanged = true;
	bool hlChanged = false;
	YZLine* yl = NULL;
	unsigned int maxLine = lineCount();
/*	for ( unsigned int i = hlLine; i < maxLine; i++ ) {
		YZSession::me->search()->highlightLine( this, i );
	}*/
	if ( m_highlight == 0L ) return false;
	while ( ctxChanged && hlLine < maxLine ) {
		yl = yzline( hlLine );
		QMemArray<uint> foldingList;
		YZLine *l = new YZLine();
		m_highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine -1 ) : l), yl, &foldingList, &ctxChanged );
		delete l;
//		yzDebug() << "updateHL line " << hlLine << ", " << ctxChanged << "; " << yl->data() << endl;
		hlChanged = ctxChanged || hlChanged;
		if ( ! ctxChanged && yl->data().isEmpty() ) {
			ctxChanged = true; // line is empty 
			++nElines;
		} else if ( ctxChanged )
			nElines = 0;
		hlLine++;
	}
	if ( hlChanged ) {
		unsigned int nToDraw = hlLine - line - nElines - 1;
//		yzDebug() << "syntaxHL: update " << nToDraw << " lines from line " << line << endl;
		YZView *it;
		for ( it = mViews.first(); it; it = mViews.next() )
			it->sendBufferPaintEvent( line, nToDraw );
	}
	return hlChanged;
}

void YZBuffer::initHL( unsigned int line ) {
	if ( m_hlupdating ) return;
//	yzDebug() << "initHL " << line << endl;
	m_hlupdating = true;
	if ( m_highlight != 0L ) {
		uint hlLine = line;
		bool ctxChanged = true;
		QMemArray<uint> foldingList;
		YZLine *l = new YZLine();
		m_highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine -1 ) : l), yzline( hlLine ), &foldingList, &ctxChanged );
		delete l;
	}
	m_hlupdating=false;
}

void YZBuffer::detectHighLight() {
	int hlMode = YzisHlManager::self()->detectHighlighting (this);
	if ( hlMode >=0 )
		setHighLight( hlMode );
	yzDebug("YZBuffer") << "HIGHLIGHTING " << hlMode << endl;
}


QString YZBuffer::tildeExpand( const QString& path ) {
	QString ret = path;
	if ( path[0] == '~' ) {
		if ( path[1] == '/' || path.length() == 1 ) {
			ret = QDir::homeDirPath() + path.mid( 1 );
		}
#ifndef YZIS_WIN32_MSVC
		  else {
			int pos = path.find('/');
			if ( pos < 0 ) // eg: ~username (without /)
				pos = path.length() - 1;
			QString user = path.left( pos ).mid( 1 );
			struct passwd* pw = getpwnam( QFile::encodeName( user ).data() );
			if ( pw )
				ret = QFile::decodeName( pw->pw_dir ) + path.mid( pos );
			// else.. do nothing
		}
#endif
	}
	return ret;
}


