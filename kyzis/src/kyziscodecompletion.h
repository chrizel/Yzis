/* This file is part of the KDE libraries

   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 by Victor Röder <Victor_Roeder@GMX.de>
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/******** Partly based on the ArgHintWidget of Qt3 by Trolltech AS *********/
/* Trolltech doesn't mind, if we license that piece of code as LGPL, because there isn't much
 * left from the desigener code */


#ifndef __KYZisCodeCompletion_H__
#define __KYZisCodeCompletion_H__

#include <ktexteditor/codecompletioninterface.h>

#include <qstringlist.h>
#include <qlabel.h>
#include <qframe.h>
#include <qmap.h>
#include <Q3IntDict>
#include <Q3Frame>
#include <Q3VBox>
#include <QLinkedList>

class KYZisView;
class KYZisArgHint;
class KYZisCCListBox;

class QLayout;

class KYZisCodeCompletionCommentLabel : public QLabel
{
  Q_OBJECT

  public:
    KYZisCodeCompletionCommentLabel( QWidget* parent, const QString& text) : QLabel( parent, "toolTipTip",
             Qt::WStyle_StaysOnTop | Qt::WStyle_Customize | Qt::WStyle_NoBorder | Qt::WStyle_Tool | Qt::WX11BypassWM )
    {
        setMargin(1);
        setIndent(0);
//        setAutoMask( false );
        setFrameStyle( Q3Frame::Plain | Q3Frame::Box );
        setLineWidth( 1 );
        setAlignment( Qt::AlignLeft | Qt::AlignTop );
        polish();
        setText(text);
        adjustSize();
    }
};

class KYZisCodeCompletion : public QObject
{
  Q_OBJECT

  public:
    KYZisCodeCompletion(KYZisView *view);

    bool codeCompletionVisible ();

	void showCompletion(const KTextEditor::Cursor &position, const QLinkedList<KTextEditor::CompletionData> &data);
    void showArgHint( QStringList functionList, const QString& strWrapping, const QString& strDelimiter );
//    void showCompletionBox( Q3ValueList<KTextEditor::CompletionEntry> entries, int offset = 0, bool casesensitive = true );
    bool eventFilter( QObject* o, QEvent* e );
	void handleKey(QKeyEvent *e);

  public slots:
    void slotCursorPosChanged();
    void showComment();
	void updateBox() { updateBox(false); }

  signals:
    void argHintHidden();

  private:
    void doComplete();
    void abortCompletion();
    void complete( KTextEditor::CompletionItem );
    void updateBox( bool newCoordinate );

    KYZisArgHint*    m_pArgHint;
    KYZisView*       m_view;
    Q3VBox*          m_completionPopup;
    KYZisCCListBox*       m_completionListBox;
 //   Q3ValueList<KTextEditor::CompletionEntry> m_complList;
    int            m_lineCursor;
    int            m_colCursor;
    int             m_offset;
    bool            m_caseSensitive;
    KYZisCodeCompletionCommentLabel* m_commentLabel;

	friend class KYZisCompletionItem;
    class CompletionItem {
    public:
      CompletionItem(const KTextEditor::CompletionData* _data,int _index):data(_data),index(_index) {}
      const KTextEditor::CompletionData* data;
      int index;
      int col;
      inline bool operator <(const CompletionItem& other) const {
        const KTextEditor::CompletionItem& oi(other.data->items().at(other.index));
        const KTextEditor::CompletionItem& ti(data->items().at(index));
        //longest match first, implement more accurately
        bool longer=(data->matchStart().column()<other.data->matchStart().column());
        bool equal=(data->matchStart().column()==other.data->matchStart().column());
        bool result= longer || (equal &&(oi.text()>ti.text()));
        return result;

      };
      inline CompletionItem& operator=(const CompletionItem& c) {data=c.data;index=c.index; return *this;} //FIXME
      inline const QString& text() const {
#if 0        
        kdDebug()<<"data="<<data<<endl;
        kdDebug()<<"data->items().size()="<<data->items().size()<<endl;
#endif
        return data->items().at(index).text();
      }
      inline KTextEditor::CompletionItem item() const {
#if 0
        kdDebug()<<"data="<<data<<endl;
        kdDebug()<<"data->items().size()="<<data->items().size()<<endl;
#endif
        return data->items().at(index);
      }
    };
    QList<CompletionItem> m_items;
    QLinkedList<KTextEditor::CompletionData> m_data;
    void buildItemList();
    bool m_blockEvents;
};

class KYZisArgHint: public Q3Frame
{
  Q_OBJECT

  public:
      KYZisArgHint( KYZisView* =0, const char* =0 );
      virtual ~KYZisArgHint();

      virtual void setCurrentFunction( int );
      virtual int currentFunction() const { return m_currentFunction; }

      void setArgMarkInfos( const QString&, const QString& );

      virtual void addFunction( int, const QString& );
      QString functionAt( int id ) const { return m_functionMap[ id ]; }

      virtual void show();
      virtual void adjustSize();
      virtual bool eventFilter( QObject*, QEvent* );

  signals:
      void argHintHidden();
      void argHintCompleted();
      void argHintAborted();

  public slots:
      virtual void reset( int, int );
      virtual void cursorPositionChanged( KYZisView*, int, int );

  private slots:
      void slotDone(bool completed);

  private:
      QMap<int, QString> m_functionMap;
      int m_currentFunction;
      QString m_wrapping;
      QString m_delimiter;
      bool m_markCurrentFunction;
      int m_currentLine;
      int m_currentCol;
      KYZisView* editorView;
      Q3IntDict<QLabel> labelDict;
      QLayout* layout;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
