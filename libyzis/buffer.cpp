/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>,
*  Copyright (C) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
*  Copyright (C) 2003-2004 Philippe Fremy <pfremy@freehackers.org>
*  Copyright (C) 2005 Scott Newton <scottn@ihug.co.nz>
*  Copyright (C) 2008 Loic Pauleve <panard@inzenet.org>
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

/* Yzis */
#include "buffer.h"
#include "line.h"
#include "view.h"
#include "undo.h"
#include "debug.h"
#include "action.h"
#include "internal_options.h"
#include "swapfile.h"
#include "session.h"
#include "luaengine.h"
#include "resourcemgr.h"
#include "search.h"
#include "mark.h"
#include "yzisinfo.h"

#include "kate/syntaxhighlight.h"

// for tildeExpand
#include <sys/types.h>
#include <sys/stat.h>

/* Qt */
#include <QTextCodec>

#include <libintl.h>

#define dbg()    yzDebug("YBuffer")
#define err()    yzError("YBuffer")

#define ASSERT_TEXT_WITHOUT_NEWLINE( functionname, text ) \
    YASSERT_MSG( text.contains('\n')==false, QString("%1 - text contains newline").arg(text) )

#define ASSERT_LINE_EXISTS( functionname, line ) \
    YASSERT_MSG( line < lineCount(), QString("%1 - line %2 does not exist, buffer has %3 lines").arg(functionname).arg(line).arg(lineCount()) )

#define ASSERT_NEXT_LINE_EXISTS( functionname, line ) \
    YASSERT_MSG( line <= lineCount(), QString("%1 - line %2 does not exist, buffer has %3 lines").arg(functionname).arg(line).arg(lineCount()) )

#define ASSERT_POS_EXISTS( functionname, pos ) \
    YASSERT_MSG( pos.x() < textline(pos.y()).length(), QString("%1 - col %2 does not exist, line %3 has %4 columns").arg( functionname ).arg( pos.x() ).arg( pos.y()).arg( textline(pos.y()).length() ) );

#define ASSERT_PREV_POS_EXISTS( functionname, pos ) \
    YASSERT_MSG( pos.x() <= textline(pos.y()).length(), QString("%1 - col %2 does not exist, line %3 has %4 columns").arg( functionname ).arg( pos.x() ).arg( pos.y() ).arg( textline(pos.y()).length() ) );


static QString Null = QString();

struct YBuffer::Private
{
    Private()
    {}

    // The current filename (absolute path name)
    QString path;

    // list of all views that are displaying this buffer
    QList<YView*> views;

    // data structure containing the actual text of the file
    YBufferData *text;

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
    YViewMarker* viewMarks;
    YDocMark* docMarks;
    YSwapFile *swapFile;

    // string containing encoding of the file
    QString currentEncoding;

    // buffer state
    BufferState state;

        // Pending replay (on load only)
        bool mPendingReplay;
};

YBuffer::YBuffer()
        : d(new Private)
{
    dbg() << "YBuffer()" << endl;

    // flags
    d->enableUpdateView = true;
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
        d->mPendingReplay = false;

    // Default to an BufferInactive buffer
    // other actions will make it BufferActive later
    //setState( BufferInactive );

    dbg() << "YBuffer() : " << d->path << endl;
}

YBuffer::~YBuffer()
{
    setState( BufferInactive );

    // These two aren't deleted when the buffer is made BufferInactive
    delete d->docMarks;
    delete d->viewMarks;
}

QString YBuffer::toString() const
{
    QString s;

    QString sViewlist;
    foreach( YView * v, d->views ) {
        QString tmp;
        tmp.sprintf("%p", v);
        sViewlist += tmp + ',';
    }
    sViewlist.chop(1);

    s.sprintf("Buffer(this=%p filename='%s' views=%s modif=%d new=%d",
              this, qp(fileNameShort()), qp(sViewlist), d->isModified, d->isFileNew
             );
    return s;
}

// ------------------------------------------------------------------------
//                            Char Operations
// ------------------------------------------------------------------------

/**
 * WARNING! Here are elementary buffer operations only! 
 * do _not_ use them directly, use action() ( actions.cpp ) instead.
 */

void YBuffer::insertChar(YCursor pos, const QString& c )
{
	insertRegion(pos, YRawData()<<c);
}

void YBuffer::delChar (YCursor pos, int count )
{
	deleteRegion(YInterval(pos, YCursor(pos.column()+count-1, pos.line())));
}

// ------------------------------------------------------------------------
//                            Line Operations
// ------------------------------------------------------------------------

void YBuffer::insertLine(const QString &l, int line)
{
	YASSERT(line <= lineCount());
	if ( line == 0 ) {
		insertRegion(YCursor(0,0), YRawData()<<l);
	} else {
		insertRegion(YCursor(getLineLength(line-1),line-1), YRawData()<<""<<l);
	}
}
void YBuffer::insertNewLine( YCursor pos )
{
	insertRegion(pos, YRawData()<<""<<"");
}
void YBuffer::deleteLine( int line )
{
	deleteRegion(YInterval(YCursor(0,line), YBound(YCursor(0,line+1), true)));
}
void YBuffer::replaceLine( const QString& l, int line )
{
	replaceRegion(YInterval(YCursor(0,line), YCursor(getLineLength(line)-1,line)), YRawData()<<l);
}

// ------------------------------------------------------------------------
//                            Content Operations
// ------------------------------------------------------------------------

YCursor YBuffer::insertRegion( const YCursor& begin, const YRawData& data )
{
	YLine* l;
	QString ldata;
	QString rdata;
	int ln = begin.line();
	YCursor after;

	l = yzline(ln);
	QString curdata = l->data();
	ldata = curdata.left(begin.column());
	rdata = curdata.mid(begin.column());

	/* first line */
	int i = 0;
	ldata += data[i];
	after.setLine(begin.line());
	++i;
	if ( i == data.size() ) {
		after.setColumn(ldata.length());
		ldata += rdata;
	}
	l->setData(ldata);
	if ( i < data.size() ) {
		/* middle lines */
		for( ; i < data.size() - 1; ++i ) {
			d->text->insert(++ln, new YLine(data[i]));
		}

		/* last line */
		ldata = data[i];
		after.setColumn(ldata.length());
		ldata += rdata;
		d->text->insert(++ln, new YLine(ldata));
		after.setLine(ln);
	}

	/* syntax highlighting update */
	int el = begin.line();
	int nl; // next line not affected by HL update
	while( el <= ln ) {
		nl = updateHL(el);
		el = qMax(nl, el+1);
	}
	YBound end(YCursor(0,el), true);
	YInterval bi(begin, end);
	dbg() << "insertRegion: insert \\cup hl : " << bi << endl;

	/* TODO: other highlighting */
	/* TODO: undo */

	/* inform views */
	foreach( YView* v, views() ) {
		v->updateBufferInterval(bi);
	}

    setChanged( true );

	return after;
}

void YBuffer::deleteRegion( const YInterval& bi )
{
	QString ldata;
	QString rdata;

	YCursor begin = bi.fromPos();
	if ( bi.from().opened() )
		begin.setColumn(begin.column() + 1);
	YCursor end = bi.toPos();
	if ( bi.to().closed() )
		end.setColumn(end.column() + 1);

	/* merge first and last line */
	YLine* l;
	l = yzline(begin.line());
	ldata = l->data().left(begin.column());
	rdata = textline(end.line()).mid(end.column());
	l->setData(ldata + rdata);

	/* delete ylines */
	int ln = begin.line() + 1;
	int n = end.line() - begin.line();
	QVector<YLine*>::iterator it = d->text->begin() + ln;
	while ( n-- && it != d->text->end() ) {
		delete (*it);
		it = d->text->erase(it);
	}

	/* ensure at least one empty line exists */
	if ( lineCount() == 0 ) {
		d->text->append(new YLine());
	}

	/* syntax highlighting update */
	ln = updateHL(begin.line());
	if ( ln > begin.line() ) {
		--ln;
	}
	YCursor update_end(getLineLength(ln), ln);

	/* TODO: other highlighting */
	/* TODO: undo */

	/* inform views */
	foreach( YView* v, views() ) {
		v->updateBufferInterval(bi);
	}

    setChanged( true );
}

YCursor YBuffer::replaceRegion( const YInterval& bi, const YRawData& data )
{
	deleteRegion(bi);
	YCursor begin = bi.fromPos();
	if ( bi.from().opened() )
		begin.setColumn(begin.column() + 1);
	return insertRegion(begin, data);
}

YRawData YBuffer::dataRegion( const YInterval& bi ) const
{
	YRawData d;

	YCursor begin = bi.fromPos();
	if ( bi.from().opened() )
		begin.setColumn(begin.column() + 1);
	YCursor end = bi.toPos();
	if ( bi.to().closed() ) {
		end.setColumn(end.column() + 1);
	}

	const YLine* l = yzline(begin.line());
	QString data = l->data().mid(begin.column());
	if ( end.line() == begin.line() ) {
		data = data.left(end.column() - begin.column());
	}
	d << data;
	int i = begin.line() + 1;
	for ( ; i <= end.line() && i < lineCount(); ++i ) {
		if ( i == end.line() ) {
			d << textline(i).left(end.column());
		} else {
			d << textline(i);
		}
	}
	if ( i <= end.line() ) {
		d << "";
	}

	return d;
}

void YBuffer::clearText()
{
	deleteRegion(YInterval(YCursor(0,0), YBound(YCursor(0,lineCount()), true)));
}

void YBuffer::setTextline( int line , const QString & l)
{
    ASSERT_TEXT_WITHOUT_NEWLINE( QString("YBuffer::setTextline(%1,%2)").arg(line).arg(l), l );
    ASSERT_LINE_EXISTS( QString("YBuffer::setTextline(%1,%2)").arg(line).arg(l), line );
	replaceRegion(YInterval(YCursor(0,line), YBound(YCursor(0,line+1), true)), YRawData() << l << "");
}

bool YBuffer::isEmpty() const
{
    return ( d->text->count() == 1 && textline(0).isEmpty() );
}


QString YBuffer::getWholeText() const
{
    if ( isEmpty() ) {
        return QString("");
    }

    QString wholeText;
    for ( int i = 0 ; i < lineCount() ; i++ )
        wholeText += textline(i) + '\n';
    return wholeText;
}

int YBuffer::getWholeTextLength() const
{
    if ( isEmpty() ) {
        return 0;
    }

    int length = 0;
    for ( int i = 0 ; i < lineCount() ; i++ ) {
        length += textline(i).length() + 1;
    }

    return length;
}

int YBuffer::firstNonBlankChar( int line ) const
{
    int i = 0;
    QString s = textline(line);
    if (s.isEmpty() ) return 0;
    while ( i < (int)s.length() && s.at(i).isSpace() )
        i++;
    return i;
}

// ------------------------------------------------------------------------
//                            File Operations
// ------------------------------------------------------------------------

void YBuffer::setEncoding( const QString& name )
{
    dbg() << "set encoding " << name << endl;
    /*
     * We have to reload the file
     */
    load( d->path );
    /* //Does not work very well, problem when converting from utf8 to iso8859-15, and problem with the EOL
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
      QValueVector<YLine*>::iterator it = d->text->begin(), end = d->text->end();
      for ( ; it != end; ++it ) {
       (*it)->setData( destCodec->toUnicode( fromCodec->fromUnicode( (*it)->data() ) ) );
      }
     }
     d->currentEncoding = name; */
}

QString YBuffer::parseFilename( const QString& filename, YCursor* gotoPos )
{
    if ( filename.isEmpty() ) {
        return filename;
    }
    QString r_filename = filename;
    if ( !QFile::exists( filename ) ) {
        /* match /file/name:line:col */
        QRegExp reg = QRegExp( "(.+):(\\d+):(\\d+):?" );
        if ( reg.exactMatch( filename ) && QFile::exists( reg.cap(1) ) ) {
            r_filename = reg.cap(1);
            if ( gotoPos != NULL ) {
                gotoPos->setY( qMax(0, reg.cap(2).toInt() - 1) );
                gotoPos->setX( qMax(0, reg.cap(3).toInt() - 1) );
            }
        } else {
            /* match /file/name:line */
            reg.setPattern( "(.+):(\\d+):?" );
            if ( reg.exactMatch( filename ) && QFile::exists( reg.cap(1) ) ) {
                r_filename = reg.cap(1);
                if ( gotoPos != NULL ) {
                    gotoPos->setY( qMax(0, reg.cap(2).toInt() - 1) );
                }
            }
        }
    }
    return r_filename;
}

void YBuffer::loadText( QString* content )
{
	clearText();
    QTextStream stream( content, QIODevice::ReadOnly );
	YRawData data;
    while ( !stream.atEnd() ) {
		data << stream.readLine();
    }
	insertRegion(YCursor(0,0), data);
    d->isFileNew = true;
}

void YBuffer::load(const QString& file)
{
    dbg() << "YBuffer::load( " << file << " ) " << endl;
    if ( file.isNull() || file.isEmpty() ) return ;

    QFileInfo fileInfo( file );
    if (fileInfo.isDir()) {
        // TODO: we cannot handle directories now
        YSession::self()->guiPopupMessage( "Sorry, we cannot open directories at the moment :(");
        return ;
    }

    //stop redraws
    d->enableUpdateView = false;

	clearText();

    setPath( file );

    QFile fl( d->path );

    //HL mode selection
    detectHighLight();

    //opens and eventually create the file
    d->undoBuffer->setInsideUndo( true );
    d->isLoading = true;
    d->currentEncoding = getLocalStringOption( "encoding" );
    if ( QFile::exists(d->path) && fl.open( QIODevice::ReadOnly ) ) {
        QTextCodec* codec;
        if ( d->currentEncoding == "locale" ) {
            codec = QTextCodec::codecForLocale();
            /*   char *buff = (char*)malloc( 102400 * sizeof(char));
               int readl = fl.readBlock(buff,102400);
               QTextCodec *c = QTextCodec::codecForContent (buff,102400);
               free(buff);
               fl.reset();
               dbg() << "Detected encoding " << c->name()  << " by reading " << readl << " bytes." << endl;
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
		YRawData data;
        while ( !stream.atEnd() ) {
			data << stream.readLine();
		}
		insertRegion(YCursor(0,0), data);
        fl.close();
    } else if (QFile::exists(d->path)) {
        YSession::self()->guiPopupMessage(_("Failed opening file %1 for reading : %2").arg(d->path).arg(fl.errorString()));
    }
    setChanged( false );
    //check for a swap file left after a crash
    d->swapFile->setFileName( d->path );
    if ( QFile::exists( d->swapFile->filename() ) ) { //if it already exists, recover from it
        struct stat buf;
        int i = stat( d->path.toLocal8Bit(), &buf );
        if ( i != -1 && S_ISREG( buf.st_mode ) && CHECK_GETEUID( buf.st_uid ) ) {
                        YView *view = YSession::self()->findViewByBuffer( this );
                        if ( !view )
                                d->mPendingReplay = true;
                        else 
                        {
                                checkRecover();
            }
        }
    }
    // d->swapFile->init(); // whatever happened before, create a new swapfile
    d->isLoading = false;
    d->undoBuffer->setInsideUndo( false );
    //reenable
    d->enableUpdateView = true;
    updateAllViews();
    filenameChanged();
}

bool YBuffer::save()
{
    if (d->path.isEmpty())
        return false;
    if ( d->isFileNew ) {
        //popup to ask a file name
        // FIXME: can this be moved somewhere higher?
        // having the low level buffer open popups
        // seems wrong to me
        YView *view = YSession::self()->findViewByBuffer( this );
        if ( !view || !view->guiPopupFileSaveAs() )
            return false; // don't try to save
    }

    QString codecName = getLocalStringOption( "fileencoding" );
    if ( codecName.isEmpty() )
        codecName = getLocalStringOption( "encoding" );
    dbg() << "save using " << codecName << " encoding" << endl;
    QTextCodec* codec;
    if ( codecName == "locale" ) {
        codec = QTextCodec::codecForLocale();
    } else {
        codec = QTextCodec::codecForName( codecName.toLatin1() );
    }
    // XXX: we have to test if codec is null, then  alert the user (like we have to do with null yzline()

    QFile file( d->path );
    d->isHLUpdating = true; //override so that it does not parse all lines
    dbg() << "Saving file to " << d->path << endl;
    if ( codec && file.open( QIODevice::WriteOnly ) ) {
        QTextStream stream( &file );
        stream.setCodec( codec );
        // do not save empty buffer to avoid creating a file
        // with only a '\n' while the buffer is emtpy
        if ( isEmpty() == false) {
            QVector<YLine*>::iterator it = d->text->begin(), end = d->text->end();
            for ( ; it != end; ++it ) {
                stream << (*it )->data() << "\n";
            }
        }
        file.close();
    } else {
        YSession::self()->guiPopupMessage(_("Failed opening file %1 for writing : %2").arg(d->path).arg(file.errorString()));
        d->isHLUpdating = true;
        return false;
    }
    d->isHLUpdating = false; //override so that it does not parse all lines
    foreach( YView *view, d->views ) {
                view->displayInfo(_("Written %1 bytes to file %2").arg(getWholeTextLength()).arg(d->path));
    }
    setChanged( false );
    filenameChanged();
    //clear swap memory
    d->swapFile->reset();
    d->swapFile->unlink();

    saveYzisInfo( firstView() );

    int hlMode = YzisHlManager::self()->detectHighlighting (this);
    if ( hlMode >= 0 && d->highlight != YzisHlManager::self()->getHl( hlMode ) )
        setHighLight( hlMode );
    return true;
}

void YBuffer::saveYzisInfo( YView* view )
{
    YASSERT( view->myBuffer() == this );
    /* save buffer cursor */
    YSession::self()->getYzisinfo()->updateStartPosition( this, view->getBufferCursor() );
    YSession::self()->getYzisinfo()->write();
}

YCursor YBuffer::getStartPosition( const QString& filename, bool parseFilename )
{
    YCursor infilename_pos;
    YCursor yzisinfo_pos;
    QString r_filename = filename;
    if ( parseFilename ) {
        r_filename = YBuffer::parseFilename( filename, &infilename_pos );
    }
    if ( infilename_pos.y() >= 0 ) {
        return infilename_pos;
    } else
        return YSession::self()->getYzisinfo()->startPosition( r_filename );
}

// ------------------------------------------------------------------------
//                            View Operations
// ------------------------------------------------------------------------

void YBuffer::addView (YView *v)
{
    dbg().SPrintf("addView( %s )", qp(v->toString() ) );
    if ( d->views.contains( v ) ) {
        err() << "view " << v->getId() << " added for the second time, discarding" << endl;
        return ; // don't append twice
    }
    d->views.append( v );
}

void YBuffer::updateAllViews()
{
	/* TODO: useful? */
    if ( !d->enableUpdateView ) return ;
    dbg() << "YBuffer updateAllViews" << endl;
    foreach( YView *view, d->views ) {
        view->sendRefreshEvent();
        view->updateFileInfo();
        view->updateFileName();
        view->updateMode();
        view->updateCursor();
        view->displayInfo("");
    }
}

YView* YBuffer::firstView() const
{
    if ( d->views.isEmpty() ) {
        err().SPrintf("firstView() - no view to return, returning NULL" );
        return NULL; //crash me :)
    }

    return d->views.first();
}

void YBuffer::rmView(YView *v)
{
    dbg().SPrintf("rmView( %s )", qp(v->toString() ) );
    d->views.removeAll(v);
    // dbg() << "YBuffer removeView found " << f << " views" << endl;
    if ( d->views.isEmpty() ) {
        setState( BufferHidden );
    }
}

// ------------------------------------------------------------------------
//                            Undo/Redo Operations
// ------------------------------------------------------------------------

void YBuffer::setChanged(bool modif)
{
    if (d->isModified == modif) {
        return;
    } else {
        d->isModified = modif;
    }

    if (!d->enableUpdateView) return;

    //update all views
    foreach(YView *view, d->views) {
        view->updateFileInfo();
    }
}


// ------------------------------------------------------------------------
//                            Syntax Highlighting
// ------------------------------------------------------------------------

void YBuffer::setHighLight( int mode, bool warnGUI )
{
    dbg().SPrintf("setHighLight( %d, %d )", mode, warnGUI );
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
        hlName.replace("+", "p");
        hlName = hlName.toLower();
        QString resource = resourceMgr()->findResource( IndentResource, hlName );
        if (! resource.isEmpty()) {
            dbg() << "setHighLight(): found indent file" << resource << endl;
            YLuaEngine::self()->source(resource);
        }
    }
}

void YBuffer::setHighLight( const QString& name )
{
    dbg().SPrintf("setHighLight( %s )", qp(name) );
    int hlMode = YzisHlManager::self()->nameFind( name );
    if ( hlMode > 0 )
        setHighLight( hlMode, true );
}


void YBuffer::makeAttribs()
{
    d->highlight->clearAttributeArrays();

    bool ctxChanged = true;
    int hlLine = 0;
    if ( !d->isLoading ) {
        while ( hlLine < lineCount()) {
            QVector<uint> foldingList;
            YLine *l = new YLine();
            d->highlight->doHighlight( ( hlLine >= 1 ? yzline( hlLine - 1 ) : l), yzline( hlLine ), &foldingList, &ctxChanged );
            delete l;
            hlLine++;
        }
	} else {
		dbg() << "makeAttribs aborted because YBuffer marked as loading" << endl;
	}
    updateAllViews();

}

void YBuffer::setPath( const QString& _path )
{
    QString oldPath = d->path;
    d->path = QFileInfo( _path.trimmed() ).absoluteFilePath();

    if ( !oldPath.isEmpty() ) {
        YSession::self()->getOptions()->updateOptions(oldPath, d->path);
    }

    // update swap file too
    d->swapFile->setFileName( _path );
        
        //no more a new file since it got a name
    d->isFileNew = false;

    filenameChanged();
}

bool YBuffer::substitute( const QString& _what, const QString& with, bool wholeline, int line )
{
    QString l = textline( line );
    bool cs = true;
    QString what = _what;
    if ( what.endsWith("\\c") ) {
        what.truncate(what.length() - 2);
        cs = false;
    }
    QRegExp rx( what );
    rx.setCaseSensitivity(cs ? Qt::CaseSensitive : Qt::CaseInsensitive );
    bool changed = false;
    int pos = 0;
    int offset = 0;
    while ( ( pos = rx.indexIn( l, offset ) ) != -1 ) {
                QPoint unique_pos(pos, line);
                d->undoBuffer->addBufferOperation(YBufferOperation::OpDelText,
                                rx.capturedTexts()[0], unique_pos);
                d->undoBuffer->addBufferOperation(YBufferOperation::OpAddText,
                                 with, unique_pos);
                //in order to apply captures , extract the match first, apply the regexp+captures, then replace full strings in the real
                //text line ;) see #167
                QString rep = l.mid( pos, rx.matchedLength());
                int savedlen = rx.matchedLength();
                QString result = rep.replace ( rx, with ); //this should apply captures correctly
                //replace it in the real string
                l = l.replace ( pos, result.length(), result);
        //l = l.replace( pos, rx.matchedLength(), with );
        changed = true;
        offset = pos + savedlen;
        if ( !wholeline ) break;
    }
    if ( changed ) {
        setTextline( line, l );
        return true;
    }
    return false;
}

QChar YBuffer::getCharAt( const YCursor at ) const
{
    QString line = textline( at.y() );
    return line[at.x()];
}

QString YBuffer::getWordAt( const YCursor at ) const
{
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
    return QString();
}

QStringList YBuffer::getText(const YCursor from, const YCursor to) const
{
	return dataRegion(YInterval(from, to));
}
QStringList YBuffer::getText( const YInterval& i ) const
{
	return dataRegion(i);
}

int YBuffer::getLocalIntegerOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( d->path + "\\" + option ) ) //find the local one ?
        return YSession::self()->getOptions()->readIntegerOption( d->path + "\\" + option, 0 );
    else
        return YSession::self()->getOptions()->readIntegerOption( "Global\\" + option, 0 ); // else give the global default if any
}

bool YBuffer::getLocalBooleanOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( d->path + "\\" + option ) )
        return YSession::self()->getOptions()->readBooleanOption( d->path + "\\" + option, false );
    else
        return YSession::self()->getOptions()->readBooleanOption( "Global\\" + option, false );
}

QString YBuffer::getLocalStringOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( d->path + "\\" + option ) )
        return YSession::self()->getOptions()->readStringOption( d->path + "\\" + option );
    else
        return YSession::self()->getOptions()->readStringOption( "Global\\" + option );
}

QStringList YBuffer::getLocalListOption( const QString& option ) const
{
    if ( YSession::self()->getOptions()->hasOption( d->path + "\\" + option ) )
        return YSession::self()->getOptions()->readListOption( d->path + "\\" + option, QStringList() );
    else
        return YSession::self()->getOptions()->readListOption( "Global\\" + option, QStringList() );
}

int YBuffer::updateHL( int line )
{
    // dbg() << "updateHL " << line << endl;

    int hlLine = line;
	int nElines = 0;
    if ( d->highlight != 0L ) {
		bool ctxChanged = true;
		bool hlChanged = false;
		int maxLine = lineCount();

		YLine* yl = NULL;
		YLine* last_yl = hlLine > 0 ? yzline(hlLine-1) : new YLine();

		for( ; ctxChanged && hlLine < maxLine; ++hlLine ) {
			yl = yzline(hlLine);
			QVector<uint> foldingList;
			d->highlight->doHighlight(last_yl, yl, &foldingList, &ctxChanged );
			if ( hlLine == 0 )
				delete last_yl;
			last_yl = yl;
			//  dbg() << "updateHL line " << hlLine << ", " << ctxChanged << "; " << yl->data() << endl;
			hlChanged = ctxChanged || hlChanged;
			if ( !ctxChanged && yl->data().isEmpty() ) {
				ctxChanged = true; // line is empty, continue
				++nElines;
			} else if ( ctxChanged ) {
				nElines = 0;
			}
		}

		if ( hlChanged ) { // XXX: remove it when redesign will be done
			int nToDraw = hlLine - line - nElines - 1;
			//  dbg() << "syntaxHL: update " << nToDraw << " lines from line " << line << endl;
			foreach( YView *view, d->views )
				view->updateBufferInterval(YInterval(YCursor(0,line), YBound(YCursor(0,line+nToDraw),true)));
		}
	}
	return hlLine - nElines;
}

void YBuffer::initHL( int line )
{
    if ( d->isHLUpdating ) return ;
    // dbg() << "initHL " << line << endl;
    d->isHLUpdating = true;
    if ( d->highlight != 0L ) {
        int hlLine = line;
        bool ctxChanged = true;
        QVector<uint> foldingList;
        YLine *l = new YLine();
        d->highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine - 1 ) : l), yzline( hlLine ), &foldingList, &ctxChanged );
        delete l;
    }
    d->isHLUpdating = false;
}

void YBuffer::detectHighLight()
{
    dbg() << "detectHighLight()" << endl;
    int hlMode = YzisHlManager::self()->detectHighlighting (this);
    if ( hlMode >= 0 ) {
        setHighLight( hlMode );
    }
    dbg() << "detectHighLight() done: " << hlMode << endl;
}


QString YBuffer::tildeExpand( const QString& path )
{
    QString ret = path;
    if ( path[0] == '~' ) {
        if ( path[1] == '/' || path.length() == 1 ) {
            ret = QDir::homePath() + path.mid( 1 );
        }
#ifndef YZIS_WIN32_GCC
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

void YBuffer::filenameChanged()
{
    dbg() << HERE() << endl;
    foreach( YView *view, d->views )
        view->updateFileName();
}

const QString YBuffer::fileNameShort() const
{
    QFileInfo fi ( d->path );
    return fi.fileName();
}

void YBuffer::highlightingChanged()
{
    foreach( YView *view, d->views )
    view->guiHighlightingChanged();
}

void YBuffer::preserve()
{
    dbg() << HERE() << endl;
    d->swapFile->flush();
}

YLine * YBuffer::yzline(int line, bool noHL /*= true*/)
{
    // call the const version of ::yzline().  Make the const
    // explicit, so we don't end up with infinite recursion
    const YBuffer *const_this = this;
    YLine *yl = const_cast<YLine*>( const_this->yzline( line ) );
    if ( !noHL && yl && !yl->initialized() ) {
        initHL( line );
    }

    return yl;
}

const YLine * YBuffer::yzline(int line) const
{
    const YLine* yl = NULL;
    if ( line >= lineCount() ) {
        dbg() << "ERROR: you are asking for line " << line << " (max is " << lineCount() << ")" << endl;
        YZIS_SAFE_MODE {
            yl = new YLine();
        }
        // we will perhaps crash after that, but we don't want to disguise bugs!
        // fix the one which call yzline ( or textline ) with a wrong line number instead.
    } else {
        yl = d->text->at( line );
    }
    return yl;
}

int YBuffer::lineCount() const
{
    return d->text->count();
}

int YBuffer::getLineLength(int line) const
{
    int length = 0;
    if ( line < lineCount() ) {
        length = yzline( line )->length();
    }
    return length;
}

const QString YBuffer::textline(int line) const
{
    if ( line < lineCount() ) {
        return yzline( line )->data();
    } else {
        return Null;
    }
}

void YBuffer::setState( BufferState state )
{
    // if we're making the buffer active or hidden, we have to ensure
    // that all the support stuff has been created
    if ( state == BufferActive || state == BufferHidden ) {
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
            d->viewMarks = new YViewMarker( );
        }

        if ( !d->docMarks ) {
            d->docMarks = new YDocMark( );
        }

        if ( !d->swapFile ) {
            d->swapFile = new YSwapFile( this );
        }

        if ( !d->text ) {
            d->text = new YBufferData;
            d->text->append( new YLine() );
        }
    }
    // if we're making the buffer inactive, we have to
    // do some cleanup
    else if ( state == BufferInactive ) {
        if ( d->swapFile ) {
            d->swapFile->unlink();
            delete d->swapFile;
            d->swapFile = NULL;
        }

        if ( d->text ) {
            for ( YBufferData::iterator itr = d->text->begin(); itr != d->text->end(); ++itr ) {
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
    if ( state == BufferActive ) {
        makeActive();
    } else if ( state == BufferHidden ) {
        makeHidden();
    } else {
        makeInactive();
    }

    d->state = state;
}

YBuffer::BufferState YBuffer::getState() const
{
    return d->state;
}

YZUndoBuffer * YBuffer::undoBuffer() const
{
    return d->undoBuffer;
}
YZAction* YBuffer::action() const
{
    return d->action;
}
YViewMarker* YBuffer::viewMarks() const
{
    return d->viewMarks;
}
YDocMark* YBuffer::docMarks() const
{
    return d->docMarks;
}
YzisHighlighting *YBuffer::highlight() const
{
    return d->highlight;
}
const QString& YBuffer::encoding() const
{
    return d->currentEncoding;
}
bool YBuffer::fileIsModified() const
{
    return d->isModified;
}
bool YBuffer::fileIsNew() const
{
    return d->isFileNew;
}
const QString& YBuffer::fileName() const
{
    return d->path;
}
QList<YView*> YBuffer::views() const
{
    return d->views;
}

void YBuffer::openNewFile()
{
    QString filename;
    // buffer at creation time should use a non existing temp filename
    // find a tmp file that does not exist
    do {
        filename = QString("/tmp/yzisnew%1").arg(rand());
    } while ( QFileInfo( filename ).exists() );

    setState( BufferActive );
    setPath( filename );
    d->isFileNew = true;
}

YCursor YBuffer::begin() const
{
    return YCursor( 0, 0 );
}

YCursor YBuffer::end() const
{
    return YCursor( getLineLength( lineCount() - 1 ), lineCount() );
}

bool YBuffer::checkRecover() {
        //check if we have a pending replay
        if ( d->mPendingReplay && YSession::self()->guiPromptYesNo(_("Recover"), _("A swap file was found for this file, it was presumably created because your computer or yzis crashed, do you want to start the recovery of this file ?")) ) {
                if ( d->swapFile->recover() )
                        setChanged( true );
                d->mPendingReplay = false;
                return true;
        }
        return false;
}

