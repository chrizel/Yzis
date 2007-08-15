/* This file is part of the Yzis libraries
*  Copyright (C) 2003-2005 Mickael Marchand <marchand@kde.org>
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

#ifndef QYZISVIEW_H
#define QYZISVIEW_H

#include <QStatusBar>
#include <qevent.h>
#include <qscrollbar.h>
#include <qlayout.h>
#include "cursor.h"
#include "commandwidget.h"
#include "view.h"

class QYZisEdit;
class YZBuffer;
class QLabel;
class QYZisCommand;
class QYZisCodeCompletion;
class QYZisLineNumbers;

class QSettings;

/**
  * @short Implementation of YZView for the Qt GUI
  *
  * In QYzis, the view is implemented using a QWidget
  */
class QYZisView: public QWidget, public YZView
{
    Q_OBJECT

    friend class QYZisCursor;
    friend class QYZisEdit;

signals :
    void cursorPositionChanged();
    void newStatus();

public:
    QYZisView(YZBuffer *doc, QWidget *parent, const char *name = 0);
    virtual ~QYZisView();
    //  KTextEditor::Document *document () { return dynamic_cast<KTextEditor::Document*>( buffer ); }
    void guiSetCommandLineText( const QString& text );
    QString guiGetCommandLineText() const;
    void guiSetFocusCommandLine();
    void guiSetFocusMainWindow();
    void guiScroll( int dx, int dy );

    void setVisibleArea( int columns, int lines );

    virtual void guiModeChanged(void);
    virtual void guiSyncViewInfo();
    void guiDisplayInfo( const QString& info );

    void wheelEvent( QWheelEvent * e );
    //  void contextMenuEvent( QContextMenuEvent * e );

    void applyConfig( const QSettings& settings, bool refresh = true );

    QChar currentChar() const;

    QYZisEdit *editor()
    {
        return m_editor;
    }

    virtual void registerModifierKeys( const QString& keys );
    virtual void unregisterModifierKeys( const QString& keys );

    void refreshScreen();

    bool guiPopupFileSaveAs();
    void guiFilenameChanged();
    void guiHighlightingChanged();

protected:
    void guiDrawSetMaxLineNumber( int max );
    void guiDrawSetLineNumber( int y, int n, int h );
    virtual void guiPreparePaintEvent( int y_min, int y_max );
    virtual void guiEndPaintEvent();
    virtual void guiDrawCell( QPoint , const YZDrawCell& cell, void* arg );
    virtual void guiDrawClearToEOL( QPoint, const QChar& clearChar );

    void guiPaintEvent( const YZSelection& s );


    virtual void guiNotifyContentChanged( const YZSelection& i );

    /**
     * Get the screen coordinates of the cursor position
     * @return cursor screen coordinates
     */ 
    //    virtual QPoint cursorPositionCoordinates () const;

public slots:
    //  void cursorPosition ( int &line, int &col) const;
    //  virtual void cursorPositionReal ( int &line, int &col) const;

    void fileSave();
    void fileSaveAs();
    //  void resetInfo();
    void scrollView( int );
    //  void scrollLineUp();
    //  void scrollLineDown();

private:
    YZBuffer *buffer;
    bool m_customComplete;
    bool m_cc_cleanup;
    QYZisEdit *m_editor;
    QStatusBar *status;
    QYZisCommand *command;
    QScrollBar *mVScroll; //vertical scroll
    QMenu *m_popup;
    QYZisCodeCompletion *m_codeCompletion;
    QGridLayout *g ;
    QLabel *m_central;
    QLabel *l_mode, *l_fileinfo, *l_linestatus;

    QYZisLineNumbers* m_lineNumbers;

    QPainter* m_painter;
};

#endif

