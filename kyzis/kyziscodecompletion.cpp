/* This file is part of the KDE libraries
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 by Victor Röder <Victor_Roeder@GMX.de>
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>
   
   port from katecodecompletion code:
   Copyright (C) 2004 by Alexander Dymo <adymo@mksat.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/******** Partly based on the ArgHintWidget of Qt3 by Trolltech AS *********/
/* Trolltech doesn't mind, if we license that piece of code as LGPL, because there isn't much
 * left from the desigener code */

#include "kyziscodecompletion.h"
#include "kyziscodecompletion.moc"

#include "viewwidget.h"
#include "document.h"

#include <kdebug.h>

#include <qwhatsthis.h>
#include <qvbox.h>
#include <qlistbox.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qapplication.h>
#include <qsizegrip.h>
#include <qfontmetrics.h>
#include <qlayout.h>
#include <qregexp.h>

/**
 * This class is used as the codecompletion listbox. It can be resized according to its contents,
 *  therfor the needed size is provided by sizeHint();
 *@short Listbox showing codecompletion
 *@author Jonas B. Jacobi <j.jacobi@gmx.de>
 */
class KYZisCCListBox : public QListBox
{
  public:
    /**
      @short Create a new CCListBox
      @param view The KYZisView, CCListBox is displayed in
    */
    KYZisCCListBox (QWidget* parent = 0, const char* name = 0, WFlags f = 0):QListBox(parent, name, f)
    {
    }

    QSize sizeHint()  const
    {
        int count = this->count();
        int height = 20;
        int tmpwidth = 8;
        //FIXME the height is for some reasons at least 3 items heigh, even if there is only one item in the list
        if (count > 0)
            if(count < 11)
                height =  count * itemHeight(0);
            else  {
                height = 10 * itemHeight(0);
                tmpwidth += verticalScrollBar()->width();
            }

        int maxcount = 0, tmpcount = 0;
        for (int i = 0; i < count; ++i)
            if ( (tmpcount = fontMetrics().width(text(i)) ) > maxcount)
                    maxcount = tmpcount;

        if (maxcount > QApplication::desktop()->width()){
            tmpwidth = QApplication::desktop()->width() - 5;
            height += horizontalScrollBar()->height();
        } else
            tmpwidth += maxcount;
        return QSize(tmpwidth,height);

    }
};

class KYZisCompletionItem : public QListBoxText
{
  public:
    KYZisCompletionItem( QListBox* lb, KTextEditor::CompletionEntry entry )
      : QListBoxText( lb )
      , m_entry( entry )
    {
      if( entry.postfix == "()" ) { // should be configurable
        setText( entry.prefix + " " + entry.text + entry.postfix );
      } else {
        setText( entry.prefix + " " + entry.text + " " + entry.postfix);
      }
    }

    KTextEditor::CompletionEntry m_entry;
};


KYZisCodeCompletion::KYZisCodeCompletion( KYZisView* view )
  : QObject( view, "KYZis Code Completion" )
  , m_view( view )
  , m_commentLabel( 0 )
{
  m_completionPopup = new QVBox( 0, 0, WType_Popup );
  m_completionPopup->setFrameStyle( QFrame::Box | QFrame::Plain );
  m_completionPopup->setLineWidth( 1 );

  m_completionListBox = new KYZisCCListBox( m_completionPopup );
  m_completionListBox->setFrameStyle( QFrame::NoFrame );
  m_completionListBox->setCornerWidget( new QSizeGrip( m_completionListBox) );

  m_completionListBox->installEventFilter( this );

  m_completionPopup->resize(m_completionListBox->sizeHint() + QSize(2,2));
  m_completionPopup->installEventFilter( this );
  m_completionPopup->setFocusProxy( m_completionListBox );

  m_pArgHint = new KYZisArgHint( m_view );
  connect( m_pArgHint, SIGNAL(argHintHidden()),
           this, SIGNAL(argHintHidden()) );

  connect( m_view, SIGNAL(cursorPositionChanged()),
           this, SLOT(slotCursorPosChanged()) );
}

bool KYZisCodeCompletion::codeCompletionVisible () {
  return m_completionPopup->isVisible();
}

void KYZisCodeCompletion::showCompletionBox(
    QValueList<KTextEditor::CompletionEntry> complList, int offset, bool casesensitive )
{
  kdDebug(13035) << "showCompletionBox " << endl;

  m_caseSensitive = casesensitive;
  m_complList = complList;
  m_offset = offset;
  m_view->cursorPositionReal( &m_lineCursor, &m_colCursor );
  m_colCursor -= offset;

  updateBox( true );
}

bool KYZisCodeCompletion::eventFilter( QObject *o, QEvent *e )
{
  if ( o != m_completionPopup &&
       o != m_completionListBox &&
       o != m_completionListBox->viewport() )
    return false;

   if( e->type() == QEvent::FocusOut )
   {
     abortCompletion();
     m_view->setFocus();
     return false;
   }

   if ( e->type() == QEvent::MouseButtonDblClick  ) {
    doComplete();
    return false;
   }

   if ( e->type() == QEvent::MouseButtonPress ) {
    QTimer::singleShot(0, this, SLOT(showComment()));
    return false;
   }

   if ( e->type() == QEvent::KeyPress ) {
    QKeyEvent *ke = (QKeyEvent*)e;
    if( /*(ke->key() == Key_Left)  || (ke->key() == Key_Right) ||*///what are <- and -> used for??
        (ke->key() == Key_Up)    || (ke->key() == Key_Down ) ||
        (ke->key() == Key_Home ) || (ke->key() == Key_End)   ||
        (ke->key() == Key_Prior) || (ke->key() == Key_Next )) {
      QTimer::singleShot(0,this,SLOT(showComment()));
      return false;
    }
    if( ke->key() == Key_Enter || ke->key() == Key_Return ) {
      doComplete();
      return false;
    }

    if( ke->key() == Key_Escape ) {
      abortCompletion();
      m_view->setFocus();
      return false;
    }

    int qtKeyCode = ke->key() | ((ke->state() & Qt::ShiftButton) ? Qt::SHIFT : 0) | ((ke->state() & Qt::ControlButton) ? Qt::CTRL : 0) | ((ke->state() & Qt::AltButton) ? Qt::ALT : 0) | ((ke->state() & Qt::MetaButton) ? Qt::META : 0);

    // redirect the event to the editor
/*    if( ke->key() == Key_Backspace) {
      m_view->backspace();
    } else if (qtKeyCode == m_view->m_editUndo->shortcut().keyCodeQt()) {
      m_view->m_editUndo->activate();
    } else {*/
      QApplication::sendEvent( m_view->editor()/*->m_viewInternal*/, e );
//     }

    if( m_colCursor > m_view->cursorColumnReal() ) {
      // the cursor is too far left
      kdDebug(13035) << "Aborting Codecompletion after sendEvent" << endl;
      kdDebug(13035) << m_view->cursorColumnReal() << endl;
      abortCompletion();
      m_view->setFocus();
      return true;
    }

    updateBox();
    return true;
  }

  return false;
}

void KYZisCodeCompletion::doComplete()
{
  KYZisCompletionItem* item = static_cast<KYZisCompletionItem*>(
     m_completionListBox->item(m_completionListBox->currentItem()));

  if( item == 0 )
    return;

  QString text = item->m_entry.text;
  QString currentLine = static_cast<KYZisDoc*>(m_view->document())->textLine(m_view->cursorLine());
  int len = m_view->cursorColumnReal() - m_colCursor;
  QString currentComplText = currentLine.mid(m_colCursor,len);
  QString add = text.mid(currentComplText.length());
  if( item->m_entry.postfix == "()" )
    add += "(";

  emit filterInsertString(&(item->m_entry),&add);
  
  m_view->gotoInsertMode();
  static_cast<KYZisDoc*>(m_view->document())->YZBuffer::action()->insertChar(m_view, m_view->getCursor(), add);

  complete( item->m_entry );
  m_view->setFocus();
}

void KYZisCodeCompletion::abortCompletion()
{
  m_completionPopup->hide();
  delete m_commentLabel;
  m_commentLabel = 0;
  emit completionAborted();
}

void KYZisCodeCompletion::complete( KTextEditor::CompletionEntry entry )
{
  m_completionPopup->hide();
  delete m_commentLabel;
  m_commentLabel = 0;
  emit completionDone( entry );
  emit completionDone();
}

void KYZisCodeCompletion::updateBox( bool )
{
  m_completionListBox->clear();

  QString currentLine = static_cast<KYZisDoc*>(m_view->document())->textLine(m_view->cursorLine());
  int len = m_view->cursorColumnReal() - m_colCursor;
  QString currentComplText = currentLine.mid(m_colCursor,len);
/* No-one really badly wants those, or?
  kdDebug(13035) << "Column: " << m_colCursor << endl;
  kdDebug(13035) << "Line: " << currentLine << endl;
  kdDebug(13035) << "CurrentColumn: " << m_view->cursorColumnReal() << endl;
  kdDebug(13035) << "Len: " << len << endl;
  kdDebug(13035) << "Text: '" << currentComplText << "'" << endl;
  kdDebug(13035) << "Count: " << m_complList.count() << endl;
*/
  QValueList<KTextEditor::CompletionEntry>::Iterator it;
  if( m_caseSensitive ) {
    for( it = m_complList.begin(); it != m_complList.end(); ++it ) {
      if( (*it).text.startsWith(currentComplText) ) {
        new KYZisCompletionItem(m_completionListBox,*it);
      }
    }
  } else {
    currentComplText = currentComplText.upper();
    for( it = m_complList.begin(); it != m_complList.end(); ++it ) {
      if( (*it).text.upper().startsWith(currentComplText) ) {
        new KYZisCompletionItem(m_completionListBox,*it);
      }
    }
  }

  if( m_completionListBox->count() == 0 ||
      ( m_completionListBox->count() == 1 && // abort if we equaled the last item
        currentComplText == m_completionListBox->text(0).stripWhiteSpace() ) ) {
    abortCompletion();
    m_view->setFocus();
    return;
  }

    kdDebug(13035)<<"KYZisCodeCompletion::updateBox: Resizing widget"<<endl;
        m_completionPopup->resize(m_completionListBox->sizeHint() + QSize(2,2));
    QPoint p = m_view->mapToGlobal( m_view->cursorCoordinates() );
        int x = p.x();
        int y = p.y() ;
        kdDebug() << "x: " << x << endl
          << "y: " << y << endl;
        if ( y + m_completionPopup->height() + m_view->editorFontMetrics( ).height() > QApplication::desktop()->height() )
                y -= (m_completionPopup->height() );
         else
                y += m_view->editorFontMetrics( ).height();

        if (x + m_completionPopup->width() > QApplication::desktop()->width())
                x = QApplication::desktop()->width() - m_completionPopup->width();

        m_completionPopup->move( QPoint(x,y) );

  m_completionListBox->setCurrentItem( 0 );
  m_completionListBox->setSelected( 0, true );
  m_completionListBox->setFocus();
  m_completionPopup->show();

  QTimer::singleShot(0,this,SLOT(showComment()));
}

void KYZisCodeCompletion::showArgHint ( QStringList functionList, const QString& strWrapping, const QString& strDelimiter )
{
  unsigned int line, col;
  m_view->cursorPositionReal( &line, &col );
  m_pArgHint->reset( line, col );
  m_pArgHint->setArgMarkInfos( strWrapping, strDelimiter );

  int nNum = 0;
  for( QStringList::Iterator it = functionList.begin(); it != functionList.end(); it++ )
  {
    kdDebug(13035) << "Insert function text: " << *it << endl;

    m_pArgHint->addFunction( nNum, ( *it ) );

    nNum++;
  }

  m_pArgHint->move(m_view->mapToGlobal(m_view->cursorCoordinates() + QPoint(0,m_view->editorFontMetrics( ).height())) );
  m_pArgHint->show();
}

void KYZisCodeCompletion::slotCursorPosChanged()
{
  m_pArgHint->cursorPositionChanged ( m_view, m_view->cursorLine(), m_view->cursorColumnReal() );
}

void KYZisCodeCompletion::showComment()
{
  if (!m_completionPopup->isVisible())
    return;

  KYZisCompletionItem* item = static_cast<KYZisCompletionItem*>(m_completionListBox->item(m_completionListBox->currentItem()));

  if( !item )
    return;

  if( item->m_entry.comment.isEmpty() )
    return;

  delete m_commentLabel;
  m_commentLabel = new KYZisCodeCompletionCommentLabel( 0, item->m_entry.comment );
  m_commentLabel->setFont(QToolTip::font());
  m_commentLabel->setPalette(QToolTip::palette());

  QPoint rightPoint = m_completionPopup->mapToGlobal(QPoint(m_completionPopup->width(),0));
  QPoint leftPoint = m_completionPopup->mapToGlobal(QPoint(0,0));
  QRect screen = QApplication::desktop()->screenGeometry ( m_commentLabel );
  QPoint finalPoint;
  if (rightPoint.x()+m_commentLabel->width() > screen.x() + screen.width())
    finalPoint.setX(leftPoint.x()-m_commentLabel->width());
  else
    finalPoint.setX(rightPoint.x());

  m_completionListBox->ensureCurrentVisible();

  finalPoint.setY(
    m_completionListBox->viewport()->mapToGlobal(m_completionListBox->itemRect(
      m_completionListBox->item(m_completionListBox->currentItem())).topLeft()).y());

  m_commentLabel->move(finalPoint);
  m_commentLabel->show();
}

KYZisArgHint::KYZisArgHint( KYZisView* parent, const char* name )
    : QFrame( parent, name, WType_Popup )
{
    setBackgroundColor( black );

    labelDict.setAutoDelete( true );
    layout = new QVBoxLayout( this, 1, 2 );
    layout->setAutoAdd( true );
    editorView = parent;

    m_markCurrentFunction = true;

    setFocusPolicy( StrongFocus );
    setFocusProxy( parent );

    reset( -1, -1 );
}

KYZisArgHint::~KYZisArgHint()
{
}

void KYZisArgHint::setArgMarkInfos( const QString& wrapping, const QString& delimiter )
{
    m_wrapping = wrapping;
    m_delimiter = delimiter;
    m_markCurrentFunction = true;
}

void KYZisArgHint::reset( int line, int col )
{
    m_functionMap.clear();
    m_currentFunction = -1;
    labelDict.clear();

    m_currentLine = line;
    m_currentCol = col - 1;
}

void KYZisArgHint::slotDone(bool completed)
{
    hide();

    m_currentLine = m_currentCol = -1;

    emit argHintHidden();
    if (completed)
        emit argHintCompleted();
    else
        emit argHintAborted();
}

void KYZisArgHint::cursorPositionChanged( KYZisView* view, int line, int col )
{
    if( m_currentCol == -1 || m_currentLine == -1 ){
        slotDone(false);
        return;
    }

    int nCountDelimiter = 0;
    int count = 0;

    QString currentTextLine = static_cast<KYZisDoc*>(view->document())->textLine( line );
    QString text = currentTextLine.mid( m_currentCol, col - m_currentCol );
    QRegExp strconst_rx( "\"[^\"]*\"" );
    QRegExp chrconst_rx( "'[^']*'" );

    text = text
        .replace( strconst_rx, "\"\"" )
        .replace( chrconst_rx, "''" );

    int index = 0;
    while( index < (int)text.length() ){
        if( text[index] == m_wrapping[0] ){
            ++count;
        } else if( text[index] == m_wrapping[1] ){
            --count;
        } else if( count > 0 && text[index] == m_delimiter[0] ){
            ++nCountDelimiter;
        }
        ++index;
    }

    if( (m_currentLine > 0 && m_currentLine != line) || (m_currentLine < col) || (count == 0) ){
        slotDone(count == 0);
        return;
    }

    // setCurArg ( nCountDelimiter + 1 );

}

void KYZisArgHint::addFunction( int id, const QString& prot )
{
    m_functionMap[ id ] = prot;
    QLabel* label = new QLabel( prot.stripWhiteSpace().simplifyWhiteSpace(), this );
    label->setBackgroundColor( QColor(255, 255, 238) );
    label->show();
    labelDict.insert( id, label );

    if( m_currentFunction < 0 )
        setCurrentFunction( id );
}

void KYZisArgHint::setCurrentFunction( int currentFunction )
{
    if( m_currentFunction != currentFunction ){

        if( currentFunction < 0 )
            currentFunction = (int)m_functionMap.size() - 1;

        if( currentFunction > (int)m_functionMap.size()-1 )
            currentFunction = 0;

        if( m_markCurrentFunction && m_currentFunction >= 0 ){
            QLabel* label = labelDict[ m_currentFunction ];
            label->setFont( font() );
        }

        m_currentFunction = currentFunction;

        if( m_markCurrentFunction ){
            QLabel* label = labelDict[ currentFunction ];
            QFont fnt( font() );
            fnt.setBold( true );
            label->setFont( fnt );
        }

        adjustSize();
    }
}

void KYZisArgHint::show()
{
    QFrame::show();
    adjustSize();
}

bool KYZisArgHint::eventFilter( QObject*, QEvent* e )
{
    if( isVisible() && e->type() == QEvent::KeyPress ){
        QKeyEvent* ke = static_cast<QKeyEvent*>( e );
        if( (ke->state() & ControlButton) && ke->key() == Key_Left ){
            setCurrentFunction( currentFunction() - 1 );
            ke->accept();
            return true;
        } else if( ke->key() == Key_Escape ){
            slotDone(false);
            return false;
        } else if( (ke->state() & ControlButton) && ke->key() == Key_Right ){
            setCurrentFunction( currentFunction() + 1 );
            ke->accept();
            return true;
        }
    }

    return false;
}

void KYZisArgHint::adjustSize( )
{
    QRect screen = QApplication::desktop()->screenGeometry( pos() );

    QFrame::adjustSize();
    if( width() > screen.width() )
        resize( screen.width(), height() );

    if( x() + width() > screen.width() )
        move( screen.width() - width(), y() );
}

// kate: space-indent on; indent-width 2; replace-tabs on;
