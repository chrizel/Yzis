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
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
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
#include "search.h"
#include "yzisinfo.h"

#define ASSERT_TEXT_WITHOUT_NEWLINE( functionname, text ) \
	YZASSERT_MSG( text.contains('\n')==false, QString("%1 - text contains newline").arg(text) )

#define ASSERT_LINE_EXISTS( functionname, line ) \
	YZASSERT_MSG( line < ( uint )lineCount(), QString("%1 - line %2 does not exist, buffer has %3 lines").arg(functionname).arg(line).arg(lineCount()) )

#define ASSERT_NEXT_LINE_EXISTS( functionname, line ) \
	YZASSERT_MSG( line <= ( uint )lineCount(), QString("%1 - line %2 does not exist, buffer has %3 lines").arg(functionname).arg(line).arg(lineCount()) )

#define ASSERT_COL_LINE_EXISTS( functionname, col, line ) \
	YZASSERT_MSG( col < ( uint )textline(line).length(), QString("%1 - col %2 does not exist, line %3 has %4 columns").arg( functionname ).arg( col ).arg( line ).arg( textline(line).length() ) );

#define ASSERT_PREV_COL_LINE_EXISTS( functionname, col, line ) \
	YZASSERT_MSG( col <= ( uint )textline(line).length(), QString("%1 - col %2 does not exist, line %3 has %4 columns").arg( functionname ).arg( col ).arg( line ).arg( textline(line).length() ) );

static QString Null = QString::null;
	
struct YZBuffer::Private
{
	static unsigned int nextId;
	
	Private() : id(nextId++) {}
	
	// The current filename (absolute path name)
	QString path;
	
	// list of all views that are displaying this buffer
	YZList<YZView*> views;

	// data structure containing the actual text of the file
	YZBufferData *text;
	
	// pointers to sub-objects
	YZUndoBuffer *undoBuffer;
	YzisHighlighting *highlight;
	
	//if a file is new, this one is true ;) (used at saving time)
	bool isFileNew;
	//used to prevent redrawing of views during some operations
	bool enableUpdateView;
	//is the file modified
	bool isModified;
	bool isLoading;
	
	// flag to disable drawing of updates
	mutable bool isHLUpdating;

	// pointers to sub-objects
	YZAction* action;
	YZViewMark* viewMarks;
	YZDocMark* docMarks;
	YZSwapFile *swapFile;
	
	// string containing encoding of the file
	QString currentEncoding;
	
	// unique identifier of the buffer
	const unsigned int id;
	
	// buffer state
	State state;
};

unsigned int YZBuffer::Private::nextId = 1;
	
YZBuffer::YZBuffer() 
	: d(new Private)
{
	yzDebug("YZBuffer") << "YZBuffer()" << endl;
	
	// flags
	d->enableUpdateView = false;
	d->isModified = false;
	d->isHLUpdating = false;
	d->isFileNew = true;
	d->isLoading = false;
	
	// sub-objects
	d->highlight = NULL;
	d->undoBuffer = NULL;
	d->action = NULL;
	d->viewMarks = NULL;
	d->docMarks = NULL;
	d->swapFile = NULL;
	d->text = NULL;
	
	// Default to an INACTIVE buffer
	// other actions will make it ACTIVE later
	setState( INACTIVE );
	
	yzDebug("YZBuffer") << "NEW BUFFER CREATED : " << d->path << endl;
}

YZBuffer::~YZBuffer() {
	setState( INACTIVE );

	// These two aren't deleted when the buffer is made INACTIVE
	delete d->docMarks;
	delete d->viewMarks;
}

// ------------------------------------------------------------------------
//                            Char Operations
// ------------------------------------------------------------------------

/**
 * WARNING! Here are elementary buffer operations only! 
 * do _not_ use them directly, use action() ( actions.cpp ) instead.
 */
	
static void viewsInit( YZBuffer *buffer, unsigned int x, unsigned int y )
{
	YZList<YZView*> views = buffer->views();
	for ( YZList<YZView*>::Iterator itr = views.begin(); itr != views.end(); ++itr ) {
		(*itr)->initChanges(x, y);
	}
}

static void viewsApply( YZBuffer *buffer, unsigned int x, unsigned int y )
{
	YZList<YZView*> views = buffer->views();
	for ( YZList<YZView*>::Iterator itr = views.begin(); itr != views.end(); ++itr ) {
		(*itr)->applyChanges(x, y);
	}
}

void YZBuffer::insertChar(unsigned int x, unsigned int y, const QString& c ) {
	ASSERT_TEXT_WITHOUT_NEWLINE( QString("YZBuffer::insertChar(%1,%2,%3)").arg(x).arg(y).arg(c), c )
	ASSERT_LINE_EXISTS( QString("YZBuffer::insertChar(%1,%2,%3)").arg(x).arg(y).arg(c), y )

	/* brute force, we'll have events specific for that later on */
	QString l=textline(y);
	if (l.isNull()) return;

	ASSERT_PREV_COL_LINE_EXISTS( QString("YZBuffer::insertChar(%1,%2,%3)").arg(x).arg(y).arg(c),x,y)

	if (x > ( uint )l.length()) {
		// if we let Qt proceed, it would append spaces to extend the line
		// and we do not want that
		return;
	}


	viewsInit( this, x, y );

	d->undoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, c, x, y );
	if ( !d->isLoading ) d->swapFile->addToSwap( YZBufferOperation::ADDTEXT, c, x, y );

	l.insert(x, c);
	setTextline(y,l);

	viewsApply( this, x + c.length(), y );
}

void YZBuffer::delChar (unsigned int x, unsigned int y, unsigned int count ) {
	ASSERT_LINE_EXISTS( QString("YZBuffer::delChar(%1,%2,%3)").arg(x).arg(y).arg(count), y )

	/* brute force, we'll have events specific for that later on */
	QString l=textline(y);
	if (l.isNull()) return;

	if (x >= ( uint )l.length())
		return;

	ASSERT_COL_LINE_EXISTS( QString("YZBuffer::delChar(%1,%2,%3)").arg(x).arg(y).arg(count),x,y)

	viewsInit( this, x, y );

	d->undoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, l.mid(x,count), x, y );
	if ( !d->isLoading ) d->swapFile->addToSwap( YZBufferOperation::DELTEXT, l.mid( x,count ), x, y );

	/* do the actual modification */
	l.remove(x, count);

	setTextline(y,l);

	viewsApply( this, x, y );
}

// ------------------------------------------------------------------------
//                            Line Operations
// ------------------------------------------------------------------------

void  YZBuffer::appendLine(const QString &l) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::appendLine(%1)").arg(l),l);
	
	if ( !d->isLoading ) {
		d->undoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, QString(), 0, lineCount() );
		d->swapFile->addToSwap( YZBufferOperation::ADDLINE, QString(), 0, lineCount() );
		d->undoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, l,  0, lineCount());
		d->swapFile->addToSwap( YZBufferOperation::ADDTEXT, l, 0, lineCount() );
	}

	d->text->append(new YZLine(l));
	if ( !d->isLoading && d->highlight != 0L ) {
		bool ctxChanged = false;
		QVector<uint> foldingList;
		YZLine *l = new YZLine();
		d->highlight->doHighlight(( d->text->count() >= 2 ? yzline( d->text->count() - 2 ) : l), yzline( d->text->count() - 1 ), &foldingList, &ctxChanged );
		delete l;
//		if ( ctxChanged ) yzDebug("YZBuffer") << "CONTEXT changed"<<endl; //no need to take any action at EOF ;)
	}
	YZSession::me->search()->highlightLine( this, d->text->count() - 1 );

	setChanged( true );
}


void  YZBuffer::insertLine(const QString &l, unsigned int line) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_NEXT_LINE_EXISTS(QString("YZBuffer::insertLine(%1,%2)").arg(l).arg(line),line)
	d->undoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, QString(), 0, line );
	if ( !d->isLoading ) d->swapFile->addToSwap( YZBufferOperation::ADDLINE, QString(), 0, line );
	d->undoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, l, 0, line );
	if ( !d->isLoading ) d->swapFile->addToSwap( YZBufferOperation::ADDTEXT, l, 0, line );

	viewsInit( this, 0, line );

	QVector<YZLine*>::iterator it = d->text->begin(), end = d->text->end();
	uint idx=0;
	for ( ; idx < line && it != end; ++it, ++idx )
		;
	d->text->insert(it, new YZLine( l ));

	YZSession::me->search()->shiftHighlight( this, line, 1 );
	YZSession::me->search()->highlightLine( this, line );
	updateHL( line );

	setChanged( true );

	viewsApply( this, 0, line + 1 );
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
	viewsInit( this, col, line );

	if ( line >= lineCount() ) return;
	QString l=textline(line);
	if (l.isNull()) return;

	ASSERT_PREV_COL_LINE_EXISTS(QString("YZBuffer::insertNewLine(%1,%2)").arg(col).arg(line),col,line )

	if (col > ( uint )l.length() ) return;

	QString newline = l.mid( col );
	if ( newline.isNull() ) newline = QString( "" );

	d->undoBuffer->addBufferOperation( YZBufferOperation::ADDLINE, "", col, line+1 );
	if ( !d->isLoading ) d->swapFile->addToSwap( YZBufferOperation::ADDLINE, "", col, line+1 );
	if (newline.length()) {
		d->undoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, newline, col, line );
		d->undoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, newline, 0, line+1 );
		if ( !d->isLoading ) {
			d->swapFile->addToSwap( YZBufferOperation::DELTEXT, newline, col, line );
			d->swapFile->addToSwap( YZBufferOperation::ADDTEXT, newline, 0, line+1 );
		}
	}

	//add new line
	QVector<YZLine*>::iterator it = d->text->begin(), end = d->text->end();
	uint idx=0;
	for ( ; idx < line+1 && it != end; ++it, ++idx )
		;
	d->text->insert(it, new YZLine( newline ));

	YZSession::me->search()->shiftHighlight( this, line+1, 1 );
	YZSession::me->search()->highlightLine( this, line+1 );
	//replace old line
	setTextline(line,l.left( col ));
	updateHL( line + 1 );

	viewsApply( this, 0, line+1 );
}

void YZBuffer::deleteLine( unsigned int line ) {
	ASSERT_LINE_EXISTS(QString("YZBuffer::deleteLine(%1)").arg(line),line)

	if (line >= lineCount()) return;

	viewsInit( this, 0, line );
	d->undoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, textline(line), 0, line );
	if ( !d->isLoading ) d->swapFile->addToSwap( YZBufferOperation::DELTEXT, textline( line ), 0, line );
	if (lineCount() > 1) {
		d->undoBuffer->addBufferOperation( YZBufferOperation::DELLINE, "", 0, line );
		if ( !d->isLoading ) d->swapFile->addToSwap( YZBufferOperation::DELLINE, "", 0, line );
		QVector<YZLine*>::iterator it = d->text->begin(), end = d->text->end();
		uint idx=0;
		for ( ; idx < line && it != end; ++it, ++idx )
			;
		delete (*it);
		d->text->erase(it);

		YZSession::me->search()->shiftHighlight( this, line+1, -1 );
		YZSession::me->search()->highlightLine( this, line );
		updateHL( line );
	} else {
		d->undoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, "", 0, line );
		if ( !d->isLoading ) d->swapFile->addToSwap( YZBufferOperation::DELTEXT, "", 0, line );
		setTextline(0,"");
	}

	setChanged( true );

	viewsApply( this, 0, line + 1 );
}

void YZBuffer::replaceLine( const QString& l, unsigned int line ) {
	ASSERT_TEXT_WITHOUT_NEWLINE(QString("YZBuffer::replaceLine(%1,%2)").arg(l).arg(line),l)
	ASSERT_LINE_EXISTS(QString("YZBuffer::replaceLine(%1,%2)").arg(l).arg(line),line)

	if ( line >= lineCount() ) return;
	if ( textline( line ).isNull() ) return;
	viewsInit( this, 0, line );

	d->undoBuffer->addBufferOperation( YZBufferOperation::DELTEXT, textline(line), 0, line );
	d->undoBuffer->addBufferOperation( YZBufferOperation::ADDTEXT, l, 0, line );
	if ( !d->isLoading ) {
		d->swapFile->addToSwap( YZBufferOperation::DELTEXT, textline( line ), 0, line );
		d->swapFile->addToSwap( YZBufferOperation::ADDTEXT, l, 0, line );
	}
	setTextline(line,l);

	viewsApply( this, l.length(), line );
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
	QVector<YZLine*>::iterator it = d->text->begin(), end = d->text->end();
	for ( ; it != end; ++it )
		delete ( *it );
	d->text->clear(); //remove the _pointers_ now
	d->text->append(new YZLine());
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
bool YZBuffer::isLineVisible(uint line) const {
	bool shown=false;
	for ( YZList<YZView*>::ConstIterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		shown = shown || (*itr)->isLineVisible(line);
	}
	return shown;
}

bool YZBuffer::isEmpty() const {
	return ( d->text->count() == 1 && textline(0).isEmpty() );
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

uint YZBuffer::firstNonBlankChar( uint line ) const {
	uint i=0;
	QString s = textline(line);
	if (s.isEmpty() ) return 0;
	while( s.at(i).isSpace() && i < ( uint )s.length())
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
	load( d->path );
/*	//Does not work very well, problem when converting from utf8 to iso8859-15, and problem with the EOL
 	QTextCodec* destCodec;
	QTextCodec* fromCodec;
	if ( name == "locale" ) {
		destCodec = QTextCodec::codecForLocale();
	} else {
		destCodec = QTextCodec::codecForName( name );
	}
	if ( d->currentEncoding == "locale" ) {
		fromCodec = QTextCodec::codecForLocale();
	} else {
		fromCodec = QTextCodec::codecForName( d->currentEncoding );
	}
	if ( ! isEmpty() ) {
		QValueVector<YZLine*>::iterator it = d->text->begin(), end = d->text->end();
		for ( ; it != end; ++it ) {
			(*it)->setData( destCodec->toUnicode( fromCodec->fromUnicode( (*it)->data() ) ) );
		}
	}
	d->currentEncoding = name; */
}

void YZBuffer::loadText( QString* content ) {
	d->text->clear(); //remove the _pointers_ now
	QTextStream stream( content, QIODevice::ReadOnly );
	while ( !stream.atEnd() ) {
		appendLine( stream.readLine() );
	}

	d->isFileNew = true;
}

void YZBuffer::load(const QString& file) {
	yzDebug("YZBuffer") << "YZBuffer load " << file << endl;
	if ( file.isNull() || file.isEmpty() ) return;
	setPath(file);
	d->isFileNew = false;

	//stop redraws
	d->enableUpdateView=false;

	QVector<YZLine*>::iterator it = d->text->begin(), end = d->text->end();
	for ( ; it != end; ++it )
		delete ( *it );
	d->text->clear();
	d->isFileNew=false;
	

	unsigned int scrollTo = 0;
	QRegExp reg = QRegExp( "(.+):(\\d+):?" );
	if ( !QFile::exists ( d->path ) && reg.exactMatch( d->path ) && QFile::exists( reg.cap( 1 ) ) ) {
		d->path = reg.cap( 1 );
		scrollTo = reg.cap( 2 ).toUInt();
	}
	QFile fl( d->path );

	//HL mode selection
	detectHighLight();
		
	//opens and eventually create the file
	d->undoBuffer->setInsideUndo( true );
	d->isLoading=true;
	d->currentEncoding = getLocalStringOption( "encoding" );
	if ( QFile::exists(d->path) && fl.open( QIODevice::ReadOnly ) ) {
		QTextCodec* codec;
		if ( d->currentEncoding == "locale" ) {
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
			codec = QTextCodec::codecForName( d->currentEncoding.toLatin1() );
		}
		QTextStream stream( &fl );
		stream.setCodec( codec );
		while ( !stream.atEnd() )
			appendLine( stream.readLine() );
		fl.close();
    } else if (QFile::exists(d->path)) {
		YZSession::me->popupMessage(_("Failed opening file %1 for reading : %2").arg(d->path).arg(fl.errorString()));
	}
	if ( ! d->text->count() )
		appendLine("");
	setChanged( false );
	//check for a swap file left after a crash
	d->swapFile->setFileName( d->path );
	if ( QFile::exists( d->swapFile->filename() ) ) { //if it already exists, recover from it
		struct stat buf;
		int i = stat( d->path.local8Bit(), &buf );
		if ( i != -1 && S_ISREG( buf.st_mode ) && CHECK_GETEUID( buf.st_uid )  ) {
			if ( YZSession::me->promptYesNo(_("Recover"),_("A swap file was found for this file, it was presumably created because your computer or yzis crashed, do you want to start the recovery of this file ?")) ) {
				if ( d->swapFile->recover() )
					setChanged( true );
			}
		}
	}
//	d->swapFile->init(); // whatever happened before, create a new swapfile
	d->isLoading=false;
	d->undoBuffer->setInsideUndo( false );
	//reenable
	d->enableUpdateView=true;
	updateAllViews();
	if ( scrollTo > 0 ) {
		for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
			(*itr)->gotoStickyCol( scrollTo - 1 );
			(*itr)->centerViewVertically( scrollTo - 1 );
		}
	}
	filenameChanged();

	YZSession::me->getYzisinfo()->readYzisinfo();
	YZCursor * tmp = YZSession::me->getYzisinfo()->startPosition( this );
	
	for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		if ( tmp ) {
			(*itr)->centerViewVertically( tmp->y() );
			(*itr)->gotodxdy(tmp->x(), tmp->y(), true );
		}
	}
}

bool YZBuffer::save() {
	if (d->path.isEmpty())
		return false;
	if ( d->isFileNew ) {
		//popup to ask a file name
		// FIXME: can this be moved somewhere higher?
		// having the low level buffer open popups
		// seems wrong to me
		YZView *view = YZSession::me->findViewByBuffer( this );
		if ( !view || !view->popupFileSaveAs() )
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
		codec = QTextCodec::codecForName( codecName.toLatin1() );
	}
	// XXX: we have to test if codec is null, then  alert the user (like we have to do with null yzline()

	QFile file( d->path );
	d->isHLUpdating = true; //override so that it does not parse all lines
	yzDebug("YZBuffer") << "Saving file to " << d->path << endl;
	if ( codec && file.open( QIODevice::WriteOnly ) ) {
		QTextStream stream( &file );
		stream.setCodec( codec );
		// do not save empty buffer to avoid creating a file
		// with only a '\n' while the buffer is emtpy
		if ( isEmpty() == false) {
			QVector<YZLine*>::iterator it = d->text->begin(), end = d->text->end();
			for ( ; it != end; ++it ) {
				stream << (*it )->data() << "\n";
			}
		}
		file.close();
	} else {
		YZSession::me->popupMessage(_("Failed opening file %1 for writing : %2").arg(d->path).arg(file.errorString()));
		d->isHLUpdating = true;
		return false;
	}
	d->isHLUpdating = false; //override so that it does not parse all lines
	for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		(*itr)->displayInfo(_("Written %1 bytes to file %2").arg(getWholeTextLength()).arg(d->path));
	}
	setChanged( false );
	filenameChanged();
	//clear swap memory
	d->swapFile->reset();
	d->swapFile->unlink();

	YZSession::me->getYzisinfo()->updateStartPosition( this, 
                  (YZSession::me->currentView())->getCursor().x(),
                  (YZSession::me->currentView())->getCursor().y() );

	YZSession::me->getYzisinfo()->writeYzisinfo();
   
	int hlMode = YzisHlManager::self()->detectHighlighting (this);
	if ( hlMode >= 0 && d->highlight != YzisHlManager::self()->getHl( hlMode ) )
		setHighLight( hlMode );
	return true;
}

// ------------------------------------------------------------------------
//                            View Operations
// ------------------------------------------------------------------------

void YZBuffer::addView (YZView *v) {
	for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		if ( *itr == v ) {
			yzWarning()<< "view " << v->getId() << " added for the second time, discarding"<<endl;
			return; // don't append twice
		}
	}
	yzDebug("YZBuffer") << "BUFFER: addView" << endl;
	d->views.append( v );
}

YZView* YZBuffer::findView( const YZViewId &id ) const {
	yzDebug("YZBuffer") << "Buffer: findView " << id << endl;
	for ( YZList<YZView*>::ConstIterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		if ( (*itr)->getId() == id )
			return *itr;
	}
//	yzDebug("YZBuffer") << "buffer::findView " << uid << " returning NULL" << endl;
	return NULL;
}

void YZBuffer::updateAllViews() {
	if ( !d->enableUpdateView ) return;
	yzDebug("YZBuffer") << "YZBuffer updateAllViews" << endl;
	for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		(*itr)->sendRefreshEvent();
		(*itr)->syncViewInfo();
	}
}

YZView* YZBuffer::firstView() const {
	if (  d->views.first() != NULL )
		return d->views.first();
	else yzDebug("YZBuffer") << "No VIEW !!!" << endl;
	return NULL;//crash me :)
}

void YZBuffer::rmView(YZView *v) {
	d->views.remove(v);
//	yzDebug("YZBuffer") << "YZBuffer removeView found " << f << " views" << endl;
	if ( d->views.isEmpty() ) {
		setState( HIDDEN );
	}
}

// ------------------------------------------------------------------------
//                            Undo/Redo Operations
// ------------------------------------------------------------------------

void YZBuffer::setChanged( bool modif ) {
	d->isModified = modif;
	if ( !d->enableUpdateView ) return;
	statusChanged();
	setModified( modif );
}
void YZBuffer::setModified( bool ) {
}

void YZBuffer::statusChanged() {
	//update all views
	for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		(*itr)->syncViewInfo();
	}
}


// ------------------------------------------------------------------------
//                            Syntax Highlighting
// ------------------------------------------------------------------------

void YZBuffer::setHighLight( uint mode, bool warnGUI ) {
	YzisHighlighting *h = YzisHlManager::self()->getHl( mode );

	if ( h != d->highlight ) { //HL is changing
		if ( d->highlight != 0L )
			d->highlight->release(); //free memory

		//init
		h->use();

		d->highlight = h;

		makeAttribs();
		if ( warnGUI )
			highlightingChanged();

		//load indent plugin
		//XXX should we check whether it was already loaded ?
		QString hlName = h->name();
		hlName.replace("+","p");
		YZExLua::instance()->source(NULL, hlName.toLower(),false);
	}
}

void YZBuffer::setHighLight( const QString& name ) {
	int hlMode = YzisHlManager::self()->nameFind( name );
	if ( hlMode > 0 )
		setHighLight( hlMode, true );
}


void YZBuffer::makeAttribs() {
	d->highlight->clearAttributeArrays();

	bool ctxChanged = true;
	unsigned int hlLine = 0;
	if ( !d->isLoading )
		while ( hlLine < lineCount()) {
			QVector<uint> foldingList;
			YZLine *l = new YZLine();
			d->highlight->doHighlight( ( hlLine >= 1 ? yzline( hlLine -1 ) : l), yzline( hlLine ), &foldingList, &ctxChanged );
			delete l;
			hlLine++;
		}
	updateAllViews();

}

void YZBuffer::setPath( const QString& _path ) {
	QString oldPath = d->path;
	d->path = QFileInfo( _path.stripWhiteSpace() ).absFilePath();
	
	if ( oldPath != QString::null ) {
		YZSession::me->getOptions()->updateOptions(oldPath, d->path);
	}
	
	// update swap file too
	d->swapFile->setFileName( _path );
	
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
	rx.setCaseSensitivity(cs ? Qt::CaseSensitive : Qt::CaseInsensitive );
	bool changed = false;
	int pos=0;
	int offset=0;
	while ( ( pos = rx.indexIn( l,offset ) ) != -1 ) {
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

QStringList YZBuffer::getText(const YZCursor& from, const YZCursor& to) const {
	d->isHLUpdating=true; //override
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

	d->isHLUpdating=false; //override
	return list;
}
QStringList YZBuffer::getText( const YZInterval& i ) const {
	YZCursor from, to;
	intervalToCursors( i, &from, &to );
	return getText( from, to );
}

void YZBuffer::intervalToCursors( const YZInterval& i, YZCursor* from, YZCursor* to ) const {
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


QString YZBuffer::getWordAt( const YZCursor& at ) const {
	QString l = textline( at.y() );
	QRegExp reg( "\\b(\\w+)\\b" );
	int idx = reg.lastIndexIn( l, at.x() );
	if ( idx == -1 || idx + reg.cap( 1 ).length() <= ( int )at.x() ) {
		idx = reg.indexIn( l, at.x() );
		if ( idx >= 0 ) return reg.cap( 1 );
		else {
			reg.setPattern( "(^|[\\s\\w])([^\\s\\w]+)([\\s\\w]|$)" );
			idx = reg.lastIndexIn( l, at.x() );
			if ( idx == -1 || idx + reg.cap( 1 ).length() + reg.cap( 2 ).length() <= ( int )at.x() ) {
				idx = reg.indexIn( l, at.x() );
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

int YZBuffer::getLocalIntegerOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( d->path+"\\"+option ) ) //find the local one ?
		return YZSession::me->getOptions()->readIntegerOption( d->path+"\\"+option, 0 );
	else
		return YZSession::me->getOptions()->readIntegerOption( "Global\\" + option, 0 ); // else give the global default if any
}

bool YZBuffer::getLocalBooleanOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( d->path+"\\"+option ) )
		return YZSession::me->getOptions()->readBooleanOption( d->path+"\\"+option, false );
	else
		return YZSession::me->getOptions()->readBooleanOption( "Global\\" + option, false );
}

QString YZBuffer::getLocalStringOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( d->path+"\\"+option ) )
		return YZSession::me->getOptions()->readStringOption( d->path+"\\"+option );
	else
		return YZSession::me->getOptions()->readStringOption( "Global\\" + option );
}

QStringList YZBuffer::getLocalListOption( const QString& option ) const {
	if ( YZSession::me->getOptions()->hasOption( d->path+"\\"+option ) )
		return YZSession::me->getOptions()->readListOption( d->path+"\\"+option, QStringList() );
	else
		return YZSession::me->getOptions()->readListOption( "Global\\" + option, QStringList() );
}

bool YZBuffer::updateHL( unsigned int line ) {
//	yzDebug() << "updateHL " << line << endl;
	if ( d->isLoading ) return false;
	unsigned int hlLine = line, nElines = 0;
	bool ctxChanged = true;
	bool hlChanged = false;
	YZLine* yl = NULL;
	unsigned int maxLine = lineCount();
/*	for ( unsigned int i = hlLine; i < maxLine; i++ ) {
		YZSession::me->search()->highlightLine( this, i );
	}*/
	if ( d->highlight == 0L ) return false;
	while ( ctxChanged && hlLine < maxLine ) {
		yl = yzline( hlLine );
		QVector<uint> foldingList;
		YZLine *l = new YZLine();
		d->highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine -1 ) : l), yl, &foldingList, &ctxChanged );
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
		for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
			(*itr)->sendBufferPaintEvent( line, nToDraw );
		}
	}
	return hlChanged;
}

void YZBuffer::initHL( unsigned int line ) {
	if ( d->isHLUpdating ) return;
//	yzDebug() << "initHL " << line << endl;
	d->isHLUpdating = true;
	if ( d->highlight != 0L ) {
		uint hlLine = line;
		bool ctxChanged = true;
		QVector<uint> foldingList;
		YZLine *l = new YZLine();
		d->highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine -1 ) : l), yzline( hlLine ), &foldingList, &ctxChanged );
		delete l;
	}
	d->isHLUpdating=false;
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
			ret = QDir::homePath() + path.mid( 1 );
		}
#ifndef YZIS_WIN32_MSVC
		  else {
			int pos = path.indexOf('/');
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

void YZBuffer::filenameChanged()
{
	for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		(*itr)->filenameChanged();
	}
}

const QString YZBuffer::fileNameShort() const {
	QFileInfo fi ( d->path );
	return fi.fileName();
}

void YZBuffer::highlightingChanged()
{
	for ( YZList<YZView*>::Iterator itr = d->views.begin(); itr != d->views.end(); ++itr ) {
		(*itr)->highlightingChanged();
	}
}

void YZBuffer::preserve()
{
	d->swapFile->flush();
}

YZLine * YZBuffer::yzline(unsigned int line, bool noHL /*= true*/) 
{
	// call the const version of ::yzline().  Make the const
	// explicit, so we don't end up with infinite recursion
	const YZBuffer *const_this = this;
	YZLine *yl = const_cast<YZLine*>( const_this->yzline( line ) );
	if ( !noHL && yl && !yl->initialized() ) {
		initHL( line );
	}
	
	return yl;
}

const YZLine * YZBuffer::yzline(unsigned int line) const
{
	if ( line >= lineCount() ) {
		yzDebug() << "ERROR: you are asking for line " << line << " (max is " << lineCount() << ")" << endl;
		// we will perhaps crash after that, but we don't want to disguise bugs!
		// fix the one which call yzline ( or textline ) with a wrong line number instead.
		return NULL;
	}
	const YZLine *yl = d->text->at( line );
	return yl;
}

unsigned int YZBuffer::lineCount() const { 
	return d->text->count(); 
}

unsigned int YZBuffer::getLineLength(unsigned int line) const {
	unsigned int length = 0;
	
	if ( line < lineCount() ) {
		length = yzline( line )->length();
	}
	
	return length;
}

const QString YZBuffer::textline(unsigned int line) const {
	if ( line < lineCount() ) {
		return yzline( line )->data();
	} else {
		return Null;
	}
}

void YZBuffer::setState( State state ) {
	// if we're making the buffer active or hidden, we have to ensure
	// that all the support stuff has been created
	if ( state == ACTIVE || state == HIDDEN ) {
		if ( !d->highlight ) {
			d->highlight = NULL;
		}
		
		if ( !d->undoBuffer ) {
			d->undoBuffer = new YZUndoBuffer( this );
		}
		
		if ( !d->action ) {
			d->action = new YZAction( this );
		}
		
		if ( !d->viewMarks ) {
			d->viewMarks = new YZViewMark( );
		}
		
		if ( !d->docMarks ) {
			d->docMarks = new YZDocMark( );
		}
		
		if ( !d->swapFile ) {
			d->swapFile = new YZSwapFile( this );
		}
		
		if ( !d->text ) {
			d->text = new YZBufferData;
			d->text->append( new YZLine() );
		}
	}
	// if we're making the buffer inactive, we have to
	// do some cleanup
	else if ( state == INACTIVE ) {
		if ( d->swapFile ) {
			d->swapFile->unlink();
			delete d->swapFile;
			d->swapFile = NULL;
		}
		
		if ( d->text ) {
			for ( YZBufferData::iterator itr = d->text->begin(); itr != d->text->end(); ++itr ) {
				delete *itr;
			}
			delete d->text;
			d->text = NULL;
		}
		
		delete d->undoBuffer;
		d->undoBuffer = NULL;
		
		delete d->action;
		d->action = NULL;
		
		if ( d->highlight ) {
			d->highlight->release();
		}
	}
	
	// call virtual functions to allow subclasses to do special things
	if  ( state == ACTIVE ) {
		makeActive();
	} else if ( state == HIDDEN ) {
		makeHidden();
	} else {
		makeInactive();
	}
	
	d->state = state;
}

YZBuffer::State YZBuffer::getState() const
{
	return d->state;
}

YZUndoBuffer * YZBuffer::undoBuffer() const { return d->undoBuffer; }
YZAction* YZBuffer::action() const { return d->action; }
YZViewMark* YZBuffer::viewMarks() const { return d->viewMarks; }
YZDocMark* YZBuffer::docMarks() const { return d->docMarks; }
YzisHighlighting *YZBuffer::highlight() const { return d->highlight; }
const QString& YZBuffer::encoding() const { return d->currentEncoding; }
bool YZBuffer::fileIsModified() const { return d->isModified; }
bool YZBuffer::fileIsNew() const { return d->isFileNew; }
const QString& YZBuffer::fileName() const {return d->path;}
unsigned int YZBuffer::getId() const { return d->id; }
YZList<YZView*> YZBuffer::views() const { return d->views; }

void YZBuffer::openNewFile()
{
	QString filename;
	// buffer at creation time should use a non existing temp filename
	// find a tmp file that does not exist
	do {
		filename = QString("/tmp/yzisnew%1").arg(rand());
	} while ( QFileInfo( filename ).exists() );
	
	setState( ACTIVE );
	setPath( filename );
	d->isFileNew = true;
}

YZCursor YZBuffer::begin()
{
    return YZCursor( 0, 0 );
}

YZCursor YZBuffer::end()
{
    return YZCursor( getLineLength( lineCount() - 1 ), lineCount() );
}

