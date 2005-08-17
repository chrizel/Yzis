/* This file is part of the KDE libraries
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>

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

#ifndef __KATE_SCHEMA_H__
#define __KATE_SCHEMA_H__

#include "syntaxhighlight.h"
#include "document.h"
#include "viewwidget.h"

#include <qstringlist.h>
#include <qintdict.h>
#include <qlistview.h>

#include <kconfig.h>
#include <kaction.h>
#include <ktexteditor/configinterfaceextension.h>

class YzisStyleListItem;

class KColorButton;

class QPopupMenu;
class KComboBox;
class YZView;

class YzisViewSchemaAction : public KActionMenu
{
  Q_OBJECT

  public:
    YzisViewSchemaAction(const QString& text, QObject* parent = 0, const char* name = 0)
       : KActionMenu(text, parent, name) { init(); };

    ~YzisViewSchemaAction(){;};

    void updateMenu (YZView *view);

  private:
    void init();

    QGuardedPtr<YZView> m_view;
    QStringList names;
    int last;

  public  slots:
    void slotAboutToShow();

  private slots:
    void setSchema (int mode);
};

//
// DIALOGS
//

/*
    QListView that automatically adds columns for YzisStyleListItems and provides a
    popup menu and a slot to edit a style using the keyboard.
    Added by anders, jan 23 2002.
*/
class YzisStyleListView : public QListView
{
  Q_OBJECT

  friend class YzisStyleListItem;

  public:
    YzisStyleListView( QWidget *parent=0, bool showUseDefaults=false);
    ~YzisStyleListView() {};
    /* Display a popupmenu for item i at the specified global position, eventually with a title,
       promoting the context name of that item */
    void showPopupMenu( YzisStyleListItem *i, const QPoint &globalPos, bool showtitle=false );
    void emitChanged() { emit changed(); };

    void setBgCol( const QColor &c ) { bgcol = c; }
    void setSelCol( const QColor &c ) { selcol = c; }
    void setNormalCol( const QColor &c ) { normalcol = c; }

  private slots:
    /* Display a popupmenu for item i at item position */
    void showPopupMenu( QListViewItem *i );
    /* call item to change a property, or display a menu */
    void slotMousePressed( int, QListViewItem*, const QPoint&, int );
    /* asks item to change the property in q */
    void mSlotPopupHandler( int z );
    void unsetColor( int );

  signals:
    void changed();

  private:
    QColor bgcol, selcol, normalcol;
    QFont docfont;
};

class YzisSchemaConfigColorTab : public QWidget
{
  Q_OBJECT

  public:
    YzisSchemaConfigColorTab( QWidget *parent = 0, const char *name = 0 );
    ~YzisSchemaConfigColorTab();

  private:
    KColorButton *m_back;
    KColorButton *m_selected;
    KColorButton *m_current;
    KColorButton *m_bracket;
    KColorButton *m_wwmarker;
    KColorButton *m_iconborder;
    KColorButton *m_tmarker;
    KColorButton *m_linenumber;

    KColorButton *m_markers;           // bg color for current selected marker
    QMap<int, QColor> m_markerColors;  // stores all markerColors
    KComboBox* m_combobox;             // switch marker type

  public:
    void readConfig (const QString& config);
    void writeConfig (const QString& config);

  signals:
    void changed(); // connected to parentWidget()->parentWidget() SLOT(slotChanged)

  protected slots:
    void slotMarkerColorChanged(const QColor&);
    void slotComboBoxChanged(int index);
};

class YzisSchemaConfigFontTab : public QWidget
{
  Q_OBJECT

  public:
    YzisSchemaConfigFontTab( QWidget *parent = 0, const char *name = 0 );
    ~YzisSchemaConfigFontTab();

  public:
    void readConfig (const QString& config);
    void writeConfig (const QString& config);

  signals:
    void changed(); // connected to parentWidget()->parentWidget() SLOT(slotChanged)

  private:
    class KFontChooser *m_fontchooser;
    QFont myFont;

  private slots:
    void slotFontSelected( const QFont &font );
};

class YzisSchemaConfigFontColorTab : public QWidget
{
  Q_OBJECT

  public:
    YzisSchemaConfigFontColorTab( QWidget *parent = 0, const char *name = 0 );
    ~YzisSchemaConfigFontColorTab();

  public:
    void schemaChanged (uint schema);
    void reload ();
    void apply ();

    YzisAttributeList *attributeList (uint schema);

  private:
    YzisStyleListView *m_defaultStyles;
    QIntDict<YzisAttributeList> m_defaultStyleLists;
};

class YzisSchemaConfigHighlightTab : public QWidget
{
  Q_OBJECT

  public:
    YzisSchemaConfigHighlightTab( QWidget *parent = 0, const char *name = 0, YzisSchemaConfigFontColorTab *page = 0, uint hl = 0 );
    ~YzisSchemaConfigHighlightTab();

  public:
    void schemaChanged (uint schema);
    void reload ();
    void apply ();

  protected slots:
    void hlChanged(int z);

  private:
    YzisSchemaConfigFontColorTab *m_defaults;

    QComboBox *hlCombo;
    YzisStyleListView *m_styles;

    uint m_schema;
    int m_hl;

    QIntDict< QIntDict<YzisHlItemDataList> > m_hlDict;
};

class YzisConfigPage : public KTextEditor::ConfigPage
{
	Q_OBJECT

	public:
		YzisConfigPage (  QWidget *parent=0, const char *name=0 );
		virtual ~YzisConfigPage ();

	public slots:
		virtual void apply () { ; };
		virtual void reload () { ; };
		virtual void reset () {};
		virtual void defaults () {};

	public:
//		bool changed () { return m_changed; }
//		void changed();

	protected slots:
		void slotChanged() { emit changed( ); }

	private slots:
		void somethingHasChanged ();

	private:
		bool m_changed;
};

class YzisSchemaConfigPage : public YzisConfigPage
{
  Q_OBJECT

  public:
    YzisSchemaConfigPage ( QWidget *parent, class KYZTextEditorIface *doc=0 );
    ~YzisSchemaConfigPage ();

  public slots:
    void apply();
    void reload();
    void reset();
    void defaults();

  private slots:
    void update ();
    void deleteSchema ();
    void newSchema ();
    void schemaChanged (int schema);

    void newCurrentPage (QWidget *w);

  private:
    int m_lastSchema;
    int m_defaultSchema;
    class QTabWidget *m_tabWidget;
    class QPushButton *btndel;
    class QComboBox *defaultSchemaCombo;
    class QComboBox *schemaCombo;
    YzisSchemaConfigColorTab *m_colorTab;
    YzisSchemaConfigFontTab *m_fontTab;
    YzisSchemaConfigFontColorTab *m_fontColorTab;
    YzisSchemaConfigHighlightTab *m_highlightTab;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
