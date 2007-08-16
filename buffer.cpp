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

#include "kate4/katehighlight.h"

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
    KateHighlighting *highlight;

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
};

YBuffer::YBuffer()
        : d(new Private)
{
    dbg() << "YBuffer()" << endl;

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

    // Default to an BufferInactive buffer
    // other actions will make it BufferActive later
    setState( BufferInactive );

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
              this, qp(fileName()), qp(sViewlist), d->isModified, d->isFileNew
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

static void viewsInit( YBuffer *buffer, QPoint pos )
{
    foreach( YView *view, buffer->views() )
    view->initChanges(pos);
}

static void viewsApply( YBuffer *buffer, int y )
{
    foreach( YView *view, buffer->views() )
    view->applyChanges(y);
}

void YBuffer::insertChar(QPoint pos, const QString& c )
{
    ASSERT_TEXT_WITHOUT_NEWLINE( QString("YBuffer::insertChar(%1,%2,%3)").arg(pos.x()).arg(pos.y()).arg(c), c )
    ASSERT_LINE_EXISTS( QString("YBuffer::insertChar(%1,%2,%3)").arg(pos.x()).arg(pos.y()).arg(c), pos.y() )

    /* brute force, we'll have events specific for that later on */
    QString l = textline(pos.y());
    if (l.isNull()) return ;

    ASSERT_PREV_POS_EXISTS( QString("YBuffer::insertChar(%1,%2,%3)").arg(pos.x()).arg(pos.y()).arg(c), pos)

    if (pos.x() > l.length()) {
        // if we let Qt proceed, it would append spaces to extend the line
        // and we do not want that
        return ;
    }


    viewsInit( this, pos );

    d->undoBuffer->addBufferOperation( YBufferOperation::OpAddText, c, pos);
    if ( !d->isLoading ) d->swapFile->addToSwap( YBufferOperation::OpAddText, c, pos );

    l.insert(pos.x(), c);
    setTextline(pos.y(), l);

    viewsApply( this, pos.y() );
}

void YBuffer::delChar (QPoint pos, int count )
{
    ASSERT_LINE_EXISTS( QString("YBuffer::delChar(%1,%2,%3)").arg(pos.x()).arg(pos.y()).arg(count), pos.y() )

    /* brute force, we'll have events specific for that later on */
    QString l = textline(pos.y());
    if (l.isNull()) return ;

    if (pos.x() >= l.length())
        return ;

    ASSERT_POS_EXISTS( QString("YBuffer::delChar(QPoint(%1,%2),%3)").arg(pos.x()).arg(pos.y()).arg(count), pos)

    viewsInit( this, pos );

    d->undoBuffer->addBufferOperation( YBufferOperation::OpDelText, l.mid(pos.x(), count), pos );
    if ( !d->isLoading ) d->swapFile->addToSwap( YBufferOperation::OpDelText, l.mid( pos.x(), count ), pos );

    /* do the actual modification */
    l.remove(pos.x(), count);

    setTextline(pos.y(), l);

    viewsApply( this, pos.y() );
}

// ------------------------------------------------------------------------
//                            Line Operations
// ------------------------------------------------------------------------

void YBuffer::appendLine(const QString &l)
{
    ASSERT_TEXT_WITHOUT_NEWLINE(QString("YBuffer::appendLine(%1)").arg(l), l);

    if ( !d->isLoading ) {
        d->undoBuffer->addBufferOperation( YBufferOperation::OpAddLine, QString(), QPoint(0, lineCount()));
        d->swapFile->addToSwap( YBufferOperation::OpAddLine, QString(), QPoint(0, lineCount()));
        d->undoBuffer->addBufferOperation( YBufferOperation::OpAddText, l, QPoint(0, lineCount()));
        d->swapFile->addToSwap( YBufferOperation::OpAddText, l, QPoint(0, lineCount()));
    }

    d->text->append(new YLine(l));
    if ( !d->isLoading && d->highlight != 0L ) {
        bool ctxChanged = false;
        QVector<int> foldingList;
        YLine *l = new YLine();
        d->highlight->doHighlight(( d->text->count() >= 2 ? yzline( d->text->count() - 2 ) : l), yzline( d->text->count() - 1 ), foldingList, ctxChanged );
        delete l;
        //  if ( ctxChanged ) dbg() << "CONTEXT changed"<<endl; //no need to take any action at EOF ;)
    }
    YSession::self()->search()->highlightLine( this, d->text->count() - 1 );

    setChanged( true );
}


void YBuffer::insertLine(const QString &l, int line)
{
    ASSERT_TEXT_WITHOUT_NEWLINE(QString("YBuffer::insertLine(%1,%2)").arg(l).arg(line), l)
    ASSERT_NEXT_LINE_EXISTS(QString("YBuffer::insertLine(%1,%2)").arg(l).arg(line), line)
    d->undoBuffer->addBufferOperation( YBufferOperation::OpAddLine, QString(), QPoint(0, line));
    if ( !d->isLoading ) d->swapFile->addToSwap( YBufferOperation::OpAddLine, QString(), QPoint(0, line));
    d->undoBuffer->addBufferOperation( YBufferOperation::OpAddText, l, QPoint(0, line));
    if ( !d->isLoading ) d->swapFile->addToSwap( YBufferOperation::OpAddText, l, QPoint(0, line));

    viewsInit( this, QPoint(0, line));

    QVector<YLine*>::iterator it = d->text->begin(), end = d->text->end();
    int idx = 0;
    for ( ; idx < line && it != end; ++it, ++idx )
        ;
    d->text->insert(it, new YLine( l ));

    YSession::self()->search()->shiftHighlight( this, line, 1 );
    YSession::self()->search()->highlightLine( this, line );
    updateHL( line );

    setChanged( true );

    viewsApply( this, line + 1 );
}

void YBuffer::insertNewLine( QPoint pos )
{
    if (pos.y() == lineCount()) {
        YASSERT_MSG(pos.y() == lineCount() && pos.x() == 0, QString("YBuffer::insertNewLine on last line is only possible on col 0"));
    } else {
        ASSERT_LINE_EXISTS(QString("YBuffer::insertNewLine(QPoint(%1,%2))").arg(pos.y()).arg(pos.x()), pos.y());
    }
    if ( pos.y() == lineCount() ) {
        //we are adding a new line at the end of the buffer by adding a new
        //line at the beginning of the next unexisting line
        //fake being at end of last line to make it work
        pos = QPoint( textline(pos.y() - 1).length(), pos.y() - 1);
    }
    viewsInit( this, pos);

    if ( pos.y() >= lineCount() ) return ;
    QString l = textline(pos.y());
    if (l.isNull()) return ;

    ASSERT_PREV_POS_EXISTS(QString("YBuffer::insertNewLine(QPoint(%1,%2))").arg(pos.x()).arg(pos.y()), pos )

    if (pos.x() > l.length() ) return ;

    QString newline = l.mid( pos.x() );
    if ( newline.isNull() ) newline = QString( "" );

    d->undoBuffer->addBufferOperation( YBufferOperation::OpAddLine, "", QPoint(pos.x(), pos.y() + 1));
    if ( !d->isLoading ) d->swapFile->addToSwap( YBufferOperation::OpAddLine, "", QPoint(pos.x(), pos.y() + 1));
    if (newline.length()) {
        d->undoBuffer->addBufferOperation( YBufferOperation::OpDelText, newline, pos);
        d->undoBuffer->addBufferOperation( YBufferOperation::OpAddText, newline, QPoint(0, pos.y() + 1));
        if ( !d->isLoading ) {
            d->swapFile->addToSwap( YBufferOperation::OpDelText, newline, pos);
            d->swapFile->addToSwap( YBufferOperation::OpAddText, newline, QPoint(0, pos.y() + 1));
        }
    }

    //add new line
    QVector<YLine*>::iterator it = d->text->begin(), end = d->text->end();
    int idx = 0;
    for ( ; idx < pos.y() + 1 && it != end; ++it, ++idx )
        ;
    d->text->insert(it, new YLine( newline ));

    YSession::self()->search()->shiftHighlight( this, pos.y() + 1, 1 );
    YSession::self()->search()->highlightLine( this, pos.y() + 1 );
    //replace old line
    setTextline(pos.y(), l.left( pos.x() ));
    updateHL( pos.y() + 1 );

    viewsApply( this, pos.y() + 1 );
}

void YBuffer::deleteLine( int line )
{
    ASSERT_LINE_EXISTS(QString("YBuffer::deleteLine(%1)").arg(line), line)

    if (line >= lineCount()) return ;

    viewsInit( this, QPoint(0, line));
    d->undoBuffer->addBufferOperation( YBufferOperation::OpDelText, textline(line), QPoint(0, line));
    if ( !d->isLoading ) d->swapFile->addToSwap( YBufferOperation::OpDelText, textline( line ), QPoint(0, line));
    if (lineCount() > 1) {
        d->undoBuffer->addBufferOperation( YBufferOperation::OpDelLine, "", QPoint(0, line));
        if ( !d->isLoading ) d->swapFile->addToSwap( YBufferOperation::OpDelLine, "", QPoint(0, line));
        QVector<YLine*>::iterator it = d->text->begin(), end = d->text->end();
        int idx = 0;
        for ( ; idx < line && it != end; ++it, ++idx )
            ;
        delete (*it);
        d->text->erase(it);

        YSession::self()->search()->shiftHighlight( this, line + 1, -1 );
        YSession::self()->search()->highlightLine( this, line );
        updateHL( line );
    } else {
        d->undoBuffer->addBufferOperation( YBufferOperation::OpDelText, "", QPoint(0, line));
        if ( !d->isLoading ) d->swapFile->addToSwap( YBufferOperation::OpDelText, "", QPoint(0, line));
        setTextline(0, "");
    }

    setChanged( true );

    viewsApply( this, line + 1 );
}

void YBuffer::replaceLine( const QString& l, int line )
{
    ASSERT_TEXT_WITHOUT_NEWLINE(QString("YBuffer::replaceLine(%1,%2)").arg(l).arg(line), l)
    ASSERT_LINE_EXISTS(QString("YBuffer::replaceLine(%1,%2)").arg(l).arg(line), line)

    if ( line >= lineCount() ) return ;
    if ( textline( line ).isNull() ) return ;
    viewsInit( this, QPoint(0, line));

    d->undoBuffer->addBufferOperation( YBufferOperation::OpDelText, textline(line), QPoint(0, line));
    d->undoBuffer->addBufferOperation( YBufferOperation::OpAddText, l, QPoint(0, line));
    if ( !d->isLoading ) {
        d->swapFile->addToSwap( YBufferOperation::OpDelText, textline( line ), QPoint(0, line));
        d->swapFile->addToSwap( YBufferOperation::OpAddText, l, QPoint(0, line));
    }
    setTextline(line, l);

    viewsApply( this, line );
}

// ------------------------------------------------------------------------
//                            Content Operations
// ------------------------------------------------------------------------

void YBuffer::clearText()
{
    dbg() << "YBuffer clearText" << endl;
    /* XXX clearText is not registered to the undo buffer but should be
     * as any other text operation. Although I doubt that this is a common
     * operation.
     */ 
    //clear is fine but better _delete_ all yzlines too ;)
    QVector<YLine*>::iterator it = d->text->begin(), end = d->text->end();
    for ( ; it != end; ++it )
        delete ( *it );
    d->text->clear(); //remove the _pointers_ now
    d->text->append(new YLine());
}

void YBuffer::setTextline( int line , const QString & l)
{
    ASSERT_TEXT_WITHOUT_NEWLINE( QString("YBuffer::setTextline(%1,%2)").arg(line).arg(l), l );
    ASSERT_LINE_EXISTS( QString("YBuffer::setTextline(%1,%2)").arg(line).arg(l), line );
    if (yzline(line)) {
        if (l.isNull()) {
            yzline(line)->setData("");
        } else {
            yzline(line)->setData(l);
        }
    }
    updateHL( line );
    YSession::self()->search()->highlightLine( this, line );
    setChanged( true );
}

// XXX Wrong
bool YBuffer::isLineVisible(int line) const
{
    bool shown = false;
    foreach( YView *view, d->views )
    shown = shown | view->isLineVisible(line);
    return shown;
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
    while ( s.at(i).isSpace() && i < ( int )s.length())
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
    d->text->clear(); //remove the _pointers_ now
    QTextStream stream( content, QIODevice::ReadOnly );
    while ( !stream.atEnd() ) {
        appendLine( stream.readLine() );
    }

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

    QVector<YLine*>::iterator it = d->text->begin(), end = d->text->end();
    for ( ; it != end; ++it )
        delete ( *it );
    d->text->clear();
    d->isFileNew = false;

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
        while ( !stream.atEnd() )
            appendLine( stream.readLine() );
        fl.close();
    } else if (QFile::exists(d->path)) {
        YSession::self()->guiPopupMessage(_("Failed opening file %1 for reading : %2").arg(d->path).arg(fl.errorString()));
    }
    if ( ! d->text->count() )
        appendLine("");
    setChanged( false );
    //check for a swap file left after a crash
    d->swapFile->setFileName( d->path );
    if ( QFile::exists( d->swapFile->filename() ) ) { //if it already exists, recover from it
        struct stat buf;
        int i = stat( d->path.toLocal8Bit(), &buf );
        if ( i != -1 && S_ISREG( buf.st_mode ) && CHECK_GETEUID( buf.st_uid ) ) {
            if ( YSession::self()->guiPromptYesNo(_("Recover"), _("A swap file was found for this file, it was presumably created because your computer or yzis crashed, do you want to start the recovery of this file ?")) ) {
                if ( d->swapFile->recover() )
                    setChanged( true );
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
    foreach( YView *view, d->views )
    view->guiDisplayInfo(_("Written %1 bytes to file %2").arg(getWholeTextLength()).arg(d->path));
    setChanged( false );
    filenameChanged();
    //clear swap memory
    d->swapFile->reset();
    d->swapFile->unlink();

    saveYzisInfo( firstView() );

#warning port me
#if 0
    int hlMode = KateHlManager::self()->detectHighlighting (this);
    if ( hlMode >= 0 && d->highlight != KateHlManager::self()->getHl( hlMode ) )
        setHighLight( hlMode );
#endif
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
    dbg().sprintf("addView( %s )", qp(v->toString() ) );
    if ( d->views.contains( v ) ) {
        err() << "view " << v->getId() << " added for the second time, discarding" << endl;
        return ; // don't append twice
    }
    d->views.append( v );
}

void YBuffer::updateAllViews()
{
    if ( !d->enableUpdateView ) return ;
    dbg() << "YBuffer updateAllViews" << endl;
    foreach( YView *view, d->views ) {
        view->sendRefreshEvent();
        view->guiSyncViewInfo();
    }
}

YView* YBuffer::firstView() const
{
    if ( d->views.isEmpty() ) {
        err().sprintf("firstView() - no view to return, returning NULL" );
        return NULL; //crash me :)
    }

    return d->views.first();
}

void YBuffer::rmView(YView *v)
{
    dbg().sprintf("rmView( %s )", qp(v->toString() ) );
    d->views.removeAll(v);
    // dbg() << "YBuffer removeView found " << f << " views" << endl;
    if ( d->views.isEmpty() ) {
        setState( BufferHidden );
    }
}

// ------------------------------------------------------------------------
//                            Undo/Redo Operations
// ------------------------------------------------------------------------

void YBuffer::setChanged( bool modif )
{
    d->isModified = modif;
    if ( !d->enableUpdateView ) return ;
    statusChanged();
    setModified( modif );
}
void YBuffer::setModified( bool )
{}

void YBuffer::statusChanged()
{
    //update all views
    foreach( YView *view, d->views )
    view->guiSyncViewInfo();
}


// ------------------------------------------------------------------------
//                            Syntax Highlighting
// ------------------------------------------------------------------------

void YBuffer::setHighLight( int mode, bool warnGUI )
{
    dbg().sprintf("setHighLight( %d, %d )", mode, warnGUI );
    KateHighlighting *h = KateHlManager::self()->getHl( mode );

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
            YLuaEngine::self()->source(hlName);
        }
    }
}

void YBuffer::setHighLight( const QString& name )
{
    dbg().sprintf("setHighLight( %s )", qp(name) );
    int hlMode = KateHlManager::self()->nameFind( name );
    if ( hlMode > 0 )
        setHighLight( hlMode, true );
}


void YBuffer::makeAttribs()
{
    d->highlight->clearAttributeArrays();

    bool ctxChanged = true;
    int hlLine = 0;
    if ( !d->isLoading )
        while ( hlLine < lineCount()) {
            QVector<int> foldingList;
            YLine *l = new YLine();
            d->highlight->doHighlight( ( hlLine >= 1 ? yzline( hlLine - 1 ) : l), yzline( hlLine ), foldingList, ctxChanged );
            delete l;
            hlLine++;
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
        l = l.replace( pos, rx.matchedLength(), with );
        changed = true;
        offset = pos + with.length();
        if ( !wholeline ) break;
    }
    if ( changed ) {
        setTextline( line, l );
        return true;
    }
    return false;
}

QStringList YBuffer::getText(const YCursor from, const YCursor to) const
{
    d->isHLUpdating = true; //override
    //the first line
    QStringList list;
    if ( from.y() != to.y() )
        list << textline( from.y() ).mid( from.x() );
    else
        list << textline( from.y() ).mid( from.x(), to.x() - from.x() + 1 );

    //other lines
    int i = from.y() + 1;
    while ( i < to.y() ) {
        list << textline( i ); //the whole line
        i++;
    }

    //last line
    if ( from.y() != to.y() )
        list << textline( to.y() ).left( to.x() + 1 );

    d->isHLUpdating = false; //override
    return list;
}
QStringList YBuffer::getText( const YInterval& i ) const
{
    YCursor from, to;
    intervalToCursors( i, &from, &to );
    return getText( from, to );
}

void YBuffer::intervalToCursors( const YInterval& i, YCursor* from, YCursor* to ) const
{
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

bool YBuffer::updateHL( int line )
{
    // dbg() << "updateHL " << line << endl;
    if ( d->isLoading ) return false;
    int hlLine = line, nElines = 0;
    bool ctxChanged = true;
    bool hlChanged = false;
    YLine* yl = NULL;
    int maxLine = lineCount();
    /* for ( int i = hlLine; i < maxLine; i++ ) {
      YSession::self()->search()->highlightLine( this, i );
     }*/
    if ( d->highlight == 0L ) return false;
    while ( ctxChanged && hlLine < maxLine ) {
        yl = yzline( hlLine );
        QVector<int> foldingList;
        YLine *l = new YLine();
        d->highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine - 1 ) : l), yl, foldingList, ctxChanged );
        delete l;
        //  dbg() << "updateHL line " << hlLine << ", " << ctxChanged << "; " << yl->data() << endl;
        hlChanged = ctxChanged || hlChanged;
        if ( ! ctxChanged && yl->data().isEmpty() ) {
            ctxChanged = true; // line is empty
            ++nElines;
        } else if ( ctxChanged )
            nElines = 0;
        hlLine++;
    }
    if ( hlChanged ) {
        int nToDraw = hlLine - line - nElines - 1;
        //  dbg() << "syntaxHL: update " << nToDraw << " lines from line " << line << endl;
        foreach( YView *view, d->views )
        view->sendBufferPaintEvent( line, nToDraw );
    }
    return hlChanged;
}

void YBuffer::initHL( int line )
{
    if ( d->isHLUpdating ) return ;
    // dbg() << "initHL " << line << endl;
    d->isHLUpdating = true;
    if ( d->highlight != 0L ) {
        int hlLine = line;
        bool ctxChanged = true;
        QVector<int> foldingList;
        YLine *l = new YLine();
        d->highlight->doHighlight(( hlLine >= 1 ? yzline( hlLine - 1 ) : l), yzline( hlLine ), foldingList, ctxChanged );
        delete l;
    }
    d->isHLUpdating = false;
}

void YBuffer::detectHighLight()
{
    dbg() << "detectHighLight()" << endl;
#warning port me
#if 0
    int hlMode = KateHlManager::self()->detectHighlighting (this);
    if ( hlMode >= 0 ) {
        setHighLight( hlMode );
    }
    dbg() << "detectHighLight() done: " << hlMode << endl;
#endif
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
    view->guiFilenameChanged();
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
KateHighlighting *YBuffer::highlight() const
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

