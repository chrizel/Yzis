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

//BEGIN Includes
#include "hlconfig.h"

#include <klocale.h>
#include <kdialogbase.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kinputdialog.h>
#include <kfontdialog.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kcolordialog.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <ktexteditor/markinterface.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qptrcollection.h>
#include <qdialog.h>
#include <qgrid.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtextcodec.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qheader.h>
#include <qlistbox.h>
#include <qhbox.h>
#include <qpainter.h>
#include <qobjectlist.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qwhatsthis.h>
#include <qvaluevector.h>
#include "session.h"
#include "schema.h"
//END

//BEGIN YzisStyleListViewItem decl
/*
    QListViewItem subclass to display/edit a style, bold/italic is check boxes,
    normal and selected colors are boxes, which will display a color chooser when
    activated.
    The context name for the style will be drawn using the editor default font and
    the chosen colors.
    This widget id designed to handle the default as well as the individual hl style
    lists.
    This widget is designed to work with the YzisStyleListView class exclusively.
    Added by anders, jan 23 2002.
*/
class YzisStyleListItem : public QListViewItem
{
  public:
    YzisStyleListItem( QListViewItem *parent=0, const QString & stylename=0,
                   class YzisAttribute* defaultstyle=0, class YzisHlItemData *data=0 );
    YzisStyleListItem( QListView *parent, const QString & stylename=0,
                   class YzisAttribute* defaultstyle=0, class YzisHlItemData *data=0 );
    ~YzisStyleListItem() { if (st) delete is; };

    /* mainly for readability */
    enum Property { ContextName, Bold, Italic, Underline, Strikeout, Color, SelColor, BgColor, SelBgColor, UseDefStyle };

    /* initializes the style from the default and the hldata */
    void initStyle();
    /* updates the hldata's style */
    void updateStyle();
    /* reimp */
    virtual int width ( const QFontMetrics & fm, const QListView * lv, int c ) const;
    /* calls changeProperty() if it makes sense considering pos. */
    void activate( int column, const QPoint &localPos );
    /* For bool fields, toggles them, for color fields, display a color chooser */
    void changeProperty( Property p );
    /** unset a color.
     * c is 100 (BGColor) or 101 (SelectedBGColor) for now.
     */
    void unsetColor( int c );
    /* style context name */
    QString contextName() { return text(0); };
    /* only true for a hl mode item using it's default style */
    bool defStyle();
    /* true for default styles */
    bool isDefault();
    /* whichever style is active (st for hl mode styles not using
       the default style, ds otherwise) */
    class YzisAttribute* style() { return is; };

  protected:
    /* reimp */
    void paintCell(QPainter *p, const QColorGroup& cg, int col, int width, int align);

  private:
    /* private methods to change properties */
    void toggleDefStyle();
    void setColor( int );
    /* helper function to copy the default style into the YzisHlItemData,
       when a property is changed and we are using default style. */

    class YzisAttribute *is, // the style currently in use
              *ds;           // default style for hl mode contexts and default styles
    class YzisHlItemData *st;      // itemdata for hl mode contexts
};
//END

//BEGIN YzisStyleListCaption decl
/*
    This is a simple subclass for drawing the language names in a nice treeview
    with the styles.  It is needed because we do not like to mess with the default
    palette of the containing ListView.  Only the paintCell method is overwritten
    to use our own palette (that is set on the viewport rather than on the listview
    itself).
*/
class YzisStyleListCaption : public QListViewItem
{
  public:
    YzisStyleListCaption( QListView *parent, const QString & name );
    ~YzisStyleListCaption() {};

  protected:
    void paintCell(QPainter *p, const QColorGroup& cg, int col, int width, int align);
};
//END

//
//
//
// DIALOGS !!!
//
//

//BEGIN KateConfigPage
YzisConfigPage::YzisConfigPage ( QWidget *parent, const char *name )
	: KTextEditor::ConfigPage (parent, name) , m_changed (false)
{
	connect (this, SIGNAL(changed()), this, SLOT(somethingHasChanged ()));
}

YzisConfigPage::~YzisConfigPage ()
{
}

void YzisConfigPage::somethingHasChanged ()
{
	m_changed = true;
	kdDebug () << "TEST: something changed on the config page: " << this << endl;
}
//END KateConfigPage

//BEGIN YzisSchemaConfigColorTab
YzisSchemaConfigColorTab::YzisSchemaConfigColorTab( QWidget *parent, const char * )
  : QWidget (parent)
{
  QHBox *b;
  QLabel *label;

  QVBoxLayout *blay=new QVBoxLayout(this, 0, KDialog::spacingHint());

  QVGroupBox *gbTextArea = new QVGroupBox(i18n("Text Area Background"), this);

  b = new QHBox (gbTextArea);
  b->setSpacing(KDialog::spacingHint());
  label = new QLabel( i18n("Normal text:"), b);
  label->setAlignment( AlignLeft|AlignVCenter);
  m_back = new KColorButton(b);

  b = new QHBox (gbTextArea);
  b->setSpacing(KDialog::spacingHint());
  label = new QLabel( i18n("Selected text:"), b);
  label->setAlignment( AlignLeft|AlignVCenter);
  m_selected = new KColorButton(b);

  b = new QHBox (gbTextArea);
  b->setSpacing(KDialog::spacingHint());
  label = new QLabel( i18n("Current line:"), b);
  label->setAlignment( AlignLeft|AlignVCenter);
  m_current = new KColorButton(b);

  // Markers from kdelibs/interfaces/ktextinterface/markinterface.h
  b = new QHBox (gbTextArea);
  b->setSpacing(KDialog::spacingHint());
  m_combobox = new KComboBox(b, "color_combo_box");
  // add the predefined mark types as defined in markinterface.h
  m_combobox->insertItem(i18n("Bookmark"));            // markType01
  m_combobox->insertItem(i18n("Active Breakpoint"));   // markType02
  m_combobox->insertItem(i18n("Reached Breakpoint"));  // markType03
  m_combobox->insertItem(i18n("Disabled Breakpoint")); // markType04
  m_combobox->insertItem(i18n("Execution"));           // markType05
  m_combobox->insertItem(i18n("Warning"));             // markType06
  m_combobox->insertItem(i18n("Error"));               // markType07
  m_combobox->setCurrentItem(0);
  m_markers = new KColorButton(b, "marker_color_button");
  connect( m_combobox, SIGNAL( activated( int ) ), SLOT( slotComboBoxChanged( int ) ) );

  blay->addWidget(gbTextArea);

  QVGroupBox *gbBorder = new QVGroupBox(i18n("Additional Elements"), this);

  b = new QHBox (gbBorder);
  b->setSpacing(KDialog::spacingHint());
  label = new QLabel( i18n("Left border background:"), b);
  label->setAlignment( AlignLeft|AlignVCenter);
  m_iconborder = new KColorButton(b);

  b = new QHBox (gbBorder);
  b->setSpacing(KDialog::spacingHint());
  label = new QLabel( i18n("Line numbers:"), b);
  label->setAlignment( AlignLeft|AlignVCenter);
  m_linenumber = new KColorButton(b);

  b = new QHBox (gbBorder);
  b->setSpacing(KDialog::spacingHint());
  label = new QLabel( i18n("Bracket highlight:"), b);
  label->setAlignment( AlignLeft|AlignVCenter);
  m_bracket = new KColorButton(b);

  b = new QHBox (gbBorder);
  b->setSpacing(KDialog::spacingHint());
  label = new QLabel( i18n("Word wrap markers:"), b);
  label->setAlignment( AlignLeft|AlignVCenter);
  m_wwmarker = new KColorButton(b);

  b = new QHBox (gbBorder);
  b->setSpacing(KDialog::spacingHint());
  label = new QLabel( i18n("Tab markers:"), b);
  label->setAlignment( AlignLeft|AlignVCenter);
  m_tmarker = new KColorButton(b);

  blay->addWidget(gbBorder);

  blay->addStretch();

  // connect signal changed(); changed is emitted by a ColorButton change!
  connect( this, SIGNAL( changed() ), parent->parentWidget(), SLOT( slotChanged() ) );

  // QWhatsThis help
  QWhatsThis::add(m_back, i18n("<p>Sets the background color of the editing area.</p>"));
  QWhatsThis::add(m_selected, i18n("<p>Sets the background color of the selection.</p>"
        "<p>To set the text color for selected text, use the \"<b>Configure "
        "Highlighting</b>\" dialog.</p>"));
  QWhatsThis::add(m_markers, i18n("<p>Sets the background color of the selected "
        "marker type.</p><p><b>Note</b>: The marker color is displayed lightly because "
        "of transparency.</p>"));
  QWhatsThis::add(m_combobox, i18n("<p>Select the marker type you want to change.</p>"));
  QWhatsThis::add(m_current, i18n("<p>Sets the background color of the currently "
        "active line, which means the line where your cursor is positioned.</p>"));
  QWhatsThis::add( m_linenumber, i18n(
        "<p>This color will be used to draw the line numbers (if enabled) and the "
        "lines in the code-folding pane.</p>" ) );
  QWhatsThis::add(m_bracket, i18n("<p>Sets the bracket matching color. This means, "
        "if you place the cursor e.g. at a <b>(</b>, the matching <b>)</b> will "
        "be highlighted with this color.</p>"));
  QWhatsThis::add(m_wwmarker, i18n(
        "<p>Sets the color of Word Wrap-related markers:</p>"
        "<dl><dt>Static Word Wrap</dt><dd>A vertical line which shows the column where "
        "text is going to be wrapped</dd>"
        "<dt>Dynamic Word Wrap</dt><dd>An arrow shown to the left of "
        "visually-wrapped lines</dd></dl>"));
  QWhatsThis::add(m_tmarker, i18n(
        "<p>Sets the color of the tabulator marks:</p>"));
}

YzisSchemaConfigColorTab::~YzisSchemaConfigColorTab()
{
}

void YzisSchemaConfigColorTab::readConfig (const QString& config)
{
  // first disconnect all signals otherwise setColor emits changed
  m_back      ->disconnect( SIGNAL( changed( const QColor & ) ) );
  m_selected  ->disconnect( SIGNAL( changed( const QColor & ) ) );
  m_current   ->disconnect( SIGNAL( changed( const QColor & ) ) );
  m_bracket   ->disconnect( SIGNAL( changed( const QColor & ) ) );
  m_wwmarker  ->disconnect( SIGNAL( changed( const QColor & ) ) );
  m_iconborder->disconnect( SIGNAL( changed( const QColor & ) ) );
  m_tmarker   ->disconnect( SIGNAL( changed( const QColor & ) ) );
  m_markers   ->disconnect( SIGNAL( changed( const QColor & ) ) );
  m_linenumber->disconnect( SIGNAL( changed( const QColor & ) ) );

  QColor tmp0 (KGlobalSettings::baseColor());
  QColor tmp1 (KGlobalSettings::highlightColor());
  QColor tmp2 (KGlobalSettings::alternateBackgroundColor());
  QColor tmp3 ( "#FFFF99" );
  QColor tmp4 (tmp2.dark());
  QColor tmp5 ( KGlobalSettings::textColor() );
  QColor tmp6 ( "#EAE9E8" );
  QColor tmp7 ( "#000000" );

  YZSession::mOptions.setGroup( config );
  m_back->setColor(YZSession::mOptions.readQColorEntry("Color Background", tmp0));
  m_selected->setColor(YZSession::mOptions.readQColorEntry("Color Selection", tmp1));
  m_current->setColor(YZSession::mOptions.readQColorEntry("Color Highlighted Line", tmp2));
  m_bracket->setColor(YZSession::mOptions.readQColorEntry("Color Highlighted Bracket", tmp3));
  m_wwmarker->setColor(YZSession::mOptions.readQColorEntry("Color Word Wrap Marker", tmp4));
  m_tmarker->setColor(YZSession::mOptions.readQColorEntry("Color Tab Marker", tmp5));
  m_iconborder->setColor(YZSession::mOptions.readQColorEntry("Color Icon Bar", tmp6));
  m_linenumber->setColor(YZSession::mOptions.readQColorEntry("Color Line Number", tmp7));

  // same std colors like in YzisDocument::markColor
  QValueVector <QColor> mark(KTextEditor::MarkInterface::reservedMarkersCount());
  Q_ASSERT(mark.size() > 6);
  mark[0] = Qt::blue;
  mark[1] = Qt::red;
  mark[2] = Qt::yellow;
  mark[3] = Qt::magenta;
  mark[4] = Qt::gray;
  mark[5] = Qt::green;
  mark[6] = Qt::red;

  // map from 0..reservedMarkersCount()-1 - the same index as in markInterface
  for (int i = 0; i < KTextEditor::MarkInterface::reservedMarkersCount(); i++)
  {
    // arg(i+1) to match the number conventions in markinterface.h
    m_markerColors[i] = YZSession::mOptions.readQColorEntry(QString("Color MarkType%1").arg(i + 1), mark[i]);
    QPixmap pix(16, 16);
    pix.fill(m_markerColors[i]);
    m_combobox->changeItem(pix, m_combobox->text(i), i);
  }
  m_markers->setColor( m_markerColors[ m_combobox->currentItem() ] );

  connect( m_back      , SIGNAL( changed( const QColor& ) ), SIGNAL( changed() ) );
  connect( m_selected  , SIGNAL( changed( const QColor& ) ), SIGNAL( changed() ) );
  connect( m_current   , SIGNAL( changed( const QColor& ) ), SIGNAL( changed() ) );
  connect( m_bracket   , SIGNAL( changed( const QColor& ) ), SIGNAL( changed() ) );
  connect( m_wwmarker  , SIGNAL( changed( const QColor& ) ), SIGNAL( changed() ) );
  connect( m_iconborder, SIGNAL( changed( const QColor& ) ), SIGNAL( changed() ) );
  connect( m_tmarker   , SIGNAL( changed( const QColor& ) ), SIGNAL( changed() ) );
  connect( m_linenumber, SIGNAL( changed( const QColor& ) ), SIGNAL( changed() ) );
  connect( m_markers   , SIGNAL( changed( const QColor& ) ), SLOT( slotMarkerColorChanged( const QColor& ) ) );
  YZSession::mOptions.setGroup( "Global" );
}

void YzisSchemaConfigColorTab::writeConfig (const QString& config)
{
  YZSession::mOptions.setGroup( config );
  YZSession::mOptions.setQColorOption("Color Background", m_back->color());
  YZSession::mOptions.setQColorOption("Color Selection", m_selected->color());
  YZSession::mOptions.setQColorOption("Color Highlighted Line", m_current->color());
  YZSession::mOptions.setQColorOption("Color Highlighted Bracket", m_bracket->color());
  YZSession::mOptions.setQColorOption("Color Word Wrap Marker", m_wwmarker->color());
  YZSession::mOptions.setQColorOption("Color Tab Marker", m_tmarker->color());
  YZSession::mOptions.setQColorOption("Color Icon Bar", m_iconborder->color());
  YZSession::mOptions.setQColorOption("Color Line Number", m_linenumber->color());

  for (int i = 0; i < KTextEditor::MarkInterface::reservedMarkersCount(); i++)
  {
    YZSession::mOptions.setQColorOption(QString("Color MarkType%1").arg(i + 1), m_markerColors[i]);
  }
  YZSession::mOptions.setGroup( "Global" );
}

void YzisSchemaConfigColorTab::slotMarkerColorChanged( const QColor& color)
{
  int index = m_combobox->currentItem();
  m_markerColors[ index ] = color;
  QPixmap pix(16, 16);
  pix.fill(color);
  m_combobox->changeItem(pix, m_combobox->text(index), index);

  emit changed();
}

void YzisSchemaConfigColorTab::slotComboBoxChanged(int index)
{
  // temporarily disconnect the changed-signal because setColor emits changed as well
  m_markers->disconnect( SIGNAL( changed( const QColor& ) ) );
  m_markers->setColor( m_markerColors[index] );
  connect( m_markers, SIGNAL( changed( const QColor& ) ), SLOT( slotMarkerColorChanged( const QColor& ) ) );
}

//END YzisSchemaConfigColorTab

//BEGIN FontConfig
YzisSchemaConfigFontTab::YzisSchemaConfigFontTab( QWidget *parent, const char * )
  : QWidget (parent)
{
    // sizemanagment
  QGridLayout *grid = new QGridLayout( this, 1, 1 );

  m_fontchooser = new KFontChooser ( this, 0L, false, QStringList(), false );
  m_fontchooser->enableColumn(KFontChooser::StyleList, false);
  grid->addWidget( m_fontchooser, 0, 0);

  connect (this, SIGNAL( changed()), parent->parentWidget(), SLOT (slotChanged()));
}

YzisSchemaConfigFontTab::~YzisSchemaConfigFontTab()
{
}

void YzisSchemaConfigFontTab::slotFontSelected( const QFont &font )
{
  myFont = font;

  emit changed();
}

void YzisSchemaConfigFontTab::readConfig (const QString& config)
{
  QFont f (KGlobalSettings::fixedFont());

  m_fontchooser->disconnect ( this );
  YZSession::mOptions.setGroup( config );
  m_fontchooser->setFont ( YZSession::mOptions.readQStringEntry("Font", f.toString()) );
  myFont = m_fontchooser->font();
  connect (m_fontchooser, SIGNAL (fontSelected( const QFont & )), this, SLOT (slotFontSelected( const QFont & )));
  YZSession::mOptions.setGroup( "Global" );
}

void YzisSchemaConfigFontTab::writeConfig (const QString& config)
{
  YZSession::mOptions.setGroup( config );
  YZSession::mOptions.setQStringOption("Font", myFont.toString() );
  YZSession::mOptions.setGroup( "Global" );
}

//END FontConfig

//BEGIN FontColorConfig
YzisSchemaConfigFontColorTab::YzisSchemaConfigFontColorTab( QWidget *parent, const char * )
  : QWidget (parent)
{
  m_defaultStyleLists.setAutoDelete(true);

  // sizemanagment
  QGridLayout *grid = new QGridLayout( this, 1, 1 );

  m_defaultStyles = new YzisStyleListView( this, false );
  grid->addWidget( m_defaultStyles, 0, 0);

  connect (m_defaultStyles, SIGNAL (changed()), parent->parentWidget(), SLOT (slotChanged()));
}

YzisSchemaConfigFontColorTab::~YzisSchemaConfigFontColorTab()
{
}

YzisAttributeList *YzisSchemaConfigFontColorTab::attributeList (uint schema)
{
  if (!m_defaultStyleLists[schema])
  {
    YzisAttributeList *list = new YzisAttributeList ();
    YzisHlManager::self()->getDefaults(schema, *list);

    m_defaultStyleLists.insert (schema, list);
  }

  return m_defaultStyleLists[schema];
}

void YzisSchemaConfigFontColorTab::schemaChanged (uint schema)
{
  m_defaultStyles->clear ();

  YzisAttributeList *l = attributeList (schema);

  // set colors
/*DISABLED FOR NOW mik ...  QPalette p ( m_defaultStyles->palette() );
  QColor _c ( "black" );//XXX? KGlobalSettings::baseColor() );
  YZSession::mOptions.setGroup( YZSession::me->schemaManager()->schema(schema) );
  p.setColor( QColorGroup::Base, YZSession::mOptions.readQColorEntry( "Color Background", _c ) );
  _c = QColor("darkblue");//XXX? KGlobalSettings::highlightColor();
  p.setColor( QColorGroup::Highlight, YZSession::mOptions.readQColorEntry( "Color Selection", _c ) );
  _c = QColor( "black" );//XXX? l->at(0)->textColor(); // not quite as much of an assumption ;)
  p.setColor( QColorGroup::Text, _c );
  m_defaultStyles->viewport()->setPalette( p ); */

  // insert the default styles backwards to get them in the right order
  for ( int i = YzisHlManager::self()->defaultStyles() - 1; i >= 0; i-- )
  {
    new YzisStyleListItem( m_defaultStyles, YzisHlManager::self()->defaultStyleName(i, true), l->at( i ) );
  }

  QWhatsThis::add( m_defaultStyles,  i18n(
    "This list displays the default styles for the current schema and "
    "offers the means to edit them. The style name reflects the current "
    "style settings.<p>To edit using the keyboard, press "
    "<strong>&lt;SPACE&gt;</strong> and choose a property from the popup menu."
    "<p>To edit the colors, click the colored squares, or select the color "
    "to edit from the popup menu.<p>You can unset the Background and Selected "
    "Background colors from the popup menu when appropriate.") );

  YZSession::mOptions.setGroup( "Global" );
}

void YzisSchemaConfigFontColorTab::reload ()
{
  m_defaultStyles->clear ();
  m_defaultStyleLists.clear ();
}

void YzisSchemaConfigFontColorTab::apply ()
{
  for ( QIntDictIterator<YzisAttributeList> it( m_defaultStyleLists ); it.current(); ++it )
    YzisHlManager::self()->setDefaults(it.currentKey(), *(it.current()));
}

//END FontColorConfig

//BEGIN YzisSchemaConfigHighlightTab
YzisSchemaConfigHighlightTab::YzisSchemaConfigHighlightTab( QWidget *parent, const char *, YzisSchemaConfigFontColorTab *page, uint hl )
  : QWidget (parent)
{
  m_defaults = page;

  m_schema = 0;
  m_hl = 0;

  m_hlDict.setAutoDelete (true);

  QVBoxLayout *layout = new QVBoxLayout(this, 0, KDialog::spacingHint() );

  // hl chooser
  QHBox *hbHl = new QHBox( this );
  layout->add (hbHl);

  hbHl->setSpacing( KDialog::spacingHint() );
  QLabel *lHl = new QLabel( i18n("H&ighlight:"), hbHl );
  hlCombo = new QComboBox( false, hbHl );
  lHl->setBuddy( hlCombo );
  connect( hlCombo, SIGNAL(activated(int)), this, SLOT(hlChanged(int)) );

  for( int i = 0; i < YzisHlManager::self()->highlights(); i++) {
    if (YzisHlManager::self()->hlSection(i).length() > 0)
      hlCombo->insertItem(YzisHlManager::self()->hlSection(i) + QString ("/") + YzisHlManager::self()->hlName(i));
    else
      hlCombo->insertItem(YzisHlManager::self()->hlName(i));
  }
  hlCombo->setCurrentItem(0);

  // styles listview
  m_styles = new YzisStyleListView( this, true );
  layout->addWidget (m_styles, 999);

  hlCombo->setCurrentItem ( hl );
  hlChanged ( hl );

  QWhatsThis::add( m_styles,  i18n(
    "This list displays the contexts of the current syntax highlight mode and "
    "offers the means to edit them. The context name reflects the current "
    "style settings.<p>To edit using the keyboard, press "
    "<strong>&lt;SPACE&gt;</strong> and choose a property from the popup menu."
    "<p>To edit the colors, click the colored squares, or select the color "
    "to edit from the popup menu.<p>You can unset the Background and Selected "
    "Background colors from the context menu when appropriate.") );

  connect (m_styles, SIGNAL (changed()), parent->parentWidget(), SLOT (slotChanged()));
}

YzisSchemaConfigHighlightTab::~YzisSchemaConfigHighlightTab()
{
}

void YzisSchemaConfigHighlightTab::hlChanged(int z)
{
  m_hl = z;

  schemaChanged (m_schema);
}

void YzisSchemaConfigHighlightTab::schemaChanged (uint schema)
{
  m_schema = schema;

  kdDebug () << "NEW SCHEMA: " << m_schema << " NEW HL: " << m_hl << endl;

  m_styles->clear ();

  if (!m_hlDict[m_schema])
  {
    kdDebug () << "NEW SCHEMA, create dict" << endl;

    m_hlDict.insert (schema, new QIntDict<YzisHlItemDataList>);
    m_hlDict[m_schema]->setAutoDelete (true);
  }

  if (!m_hlDict[m_schema]->find(m_hl))
  {
    kdDebug () << "NEW HL, create list" << endl;

    YzisHlItemDataList *list = new YzisHlItemDataList ();
    YzisHlManager::self()->getHl( m_hl )->getYzisHlItemDataListCopy (m_schema, *list);
    m_hlDict[m_schema]->insert (m_hl, list);
  }

  YzisAttributeList *l = m_defaults->attributeList (schema);

  // Set listview colors
  // We do that now, because we can now get the "normal text" color.
  // TODO this reads of the KConfig object, which should be changed when
  // the color tab is fixed.
/*DISABLE FOR NOW ... mik  QPalette p ( m_styles->palette() );
  QColor _c ( "black" );//XXX? KGlobalSettings::baseColor() );
  YZSession::mOptions.setGroup(YZSession::me->schemaManager()->schema(m_schema) );
  p.setColor( QColorGroup::Base, YZSession::mOptions.readQColorEntry( "Color Background", _c ) );
  _c = QColor("darkblue");//XXX? KGlobalSettings::highlightColor();
  p.setColor( QColorGroup::Highlight, YZSession::mOptions.readQColorEntry( "Color Selection", _c ) );
  _c = QColor( "black" );//l->at(0)->textColor(); // not quite as much of an assumption ;)
  p.setColor( QColorGroup::Text, _c );
  m_styles->viewport()->setPalette( p );*/

  QDict<YzisStyleListCaption> prefixes;
  for ( YzisHlItemData *itemData = m_hlDict[m_schema]->find(m_hl)->last();
        itemData != 0L;
        itemData = m_hlDict[m_schema]->find(m_hl)->prev())
  {
    kdDebug () << "insert items " << itemData->name << endl;

    // All stylenames have their language mode prefixed, e.g. HTML:Comment
    // split them and put them into nice substructures.
    int c = itemData->name.find(':');
    if ( c > 0 ) {
      QString prefix = itemData->name.left(c);
      QString name   = itemData->name.mid(c+1);

      YzisStyleListCaption *parent = prefixes.find( prefix );
      if ( ! parent )
      {
        parent = new YzisStyleListCaption( m_styles, prefix );
        parent->setOpen(true);
        prefixes.insert( prefix, parent );
      }
      new YzisStyleListItem( parent, name, l->at(itemData->defStyleNum), itemData );
    } else {
      new YzisStyleListItem( m_styles, itemData->name, l->at(itemData->defStyleNum), itemData );
    }
  }
  YZSession::mOptions.setGroup( "Global" );
}

void YzisSchemaConfigHighlightTab::reload ()
{
  m_styles->clear ();
  m_hlDict.clear ();

  hlChanged (0);
}

void YzisSchemaConfigHighlightTab::apply ()
{
  for ( QIntDictIterator< QIntDict<YzisHlItemDataList> > it( m_hlDict ); it.current(); ++it )
    for ( QIntDictIterator< YzisHlItemDataList > it2( *it.current() ); it2.current(); ++it2 )
       YzisHlManager::self()->getHl( it2.currentKey() )->setYzisHlItemDataList (it.currentKey(), *(it2.current()));
}

//END YzisSchemaConfigHighlightTab

//BEGIN YzisSchemaConfigPage
YzisSchemaConfigPage::YzisSchemaConfigPage( QWidget *parent, KYZisDoc *doc )
  : YzisConfigPage( parent ),
    m_lastSchema (-1)
{
  QVBoxLayout *layout = new QVBoxLayout(this, 0, KDialog::spacingHint() );

  QHBox *hbHl = new QHBox( this );
  layout->add (hbHl);
  hbHl->setSpacing( KDialog::spacingHint() );
  QLabel *lHl = new QLabel( i18n("&Schema:"), hbHl );
  schemaCombo = new QComboBox( false, hbHl );
  lHl->setBuddy( schemaCombo );
  connect( schemaCombo, SIGNAL(activated(int)),
           this, SLOT(schemaChanged(int)) );

  QPushButton *btnnew = new QPushButton( i18n("&New..."), hbHl );
  connect( btnnew, SIGNAL(clicked()), this, SLOT(newSchema()) );

  btndel = new QPushButton( i18n("&Delete"), hbHl );
  connect( btndel, SIGNAL(clicked()), this, SLOT(deleteSchema()) );

  m_tabWidget = new QTabWidget ( this );
  m_tabWidget->setMargin (KDialog::marginHint());
  layout->add (m_tabWidget);

  connect (m_tabWidget, SIGNAL (currentChanged (QWidget *)), this, SLOT (newCurrentPage (QWidget *)));

  m_colorTab = new YzisSchemaConfigColorTab (m_tabWidget);
  m_tabWidget->addTab (m_colorTab, i18n("Colors"));

  m_fontTab = new YzisSchemaConfigFontTab (m_tabWidget);
  m_tabWidget->addTab (m_fontTab, i18n("Font"));

  m_fontColorTab = new YzisSchemaConfigFontColorTab (m_tabWidget);
  m_tabWidget->addTab (m_fontColorTab, i18n("Normal Text Styles"));

  uint hl = doc ? doc->hlMode() : 0;
  m_highlightTab = new YzisSchemaConfigHighlightTab (m_tabWidget, "", m_fontColorTab, hl );
  m_tabWidget->addTab (m_highlightTab, i18n("Highlighting Text Styles"));

  hbHl = new QHBox( this );
  layout->add (hbHl);
  hbHl->setSpacing( KDialog::spacingHint() );
  lHl = new QLabel( i18n("&Default schema for %1:").arg(KApplication::kApplication()->aboutData()->programName ()), hbHl );
  defaultSchemaCombo = new QComboBox( false, hbHl );
  lHl->setBuddy( defaultSchemaCombo );


  m_defaultSchema = 0;//(doc && doc->activeView()) ? doc->activeView()->renderer()->config()->schema() : YzisRendererConfig::global()->schema();

  reload();

  connect( defaultSchemaCombo, SIGNAL(activated(int)), this, SLOT(slotChanged()) );
}

YzisSchemaConfigPage::~YzisSchemaConfigPage ()
{
  // just reload config from disc
  YZSession::me->schemaManager()->update ();
}

void YzisSchemaConfigPage::apply()
{
  if (m_lastSchema > -1)
  {
    m_colorTab->writeConfig (YZSession::me->schemaManager()->schema(m_lastSchema));
    m_fontTab->writeConfig (YZSession::me->schemaManager()->schema(m_lastSchema));
  }

  // just sync the config
//  YZSession::me->schemaManager()->schema(0)->sync();
  YZSession::me->schemaManager()->update ();

//XXX?  YzisRendererConfig::global()->setSchema (defaultSchemaCombo->currentItem());

  // special for the highlighting stuff
  m_fontColorTab->apply ();
  m_highlightTab->apply ();

  // sync the hl config for real
//  YzisHlManager::self()->getKConfig()->sync ();
}

void YzisSchemaConfigPage::reload()
{
  // just reload the config from disc
  YZSession::me->schemaManager()->update ();

  // special for the highlighting stuff
  m_fontColorTab->reload ();

  update ();

  defaultSchemaCombo->setCurrentItem (0 );//XXX? YzisRendererConfig::global()->schema());

  // initialize to the schema in the current document, or default schema
  schemaCombo->setCurrentItem( 0 ); //XXX? m_defaultSchema );
  schemaChanged( 0 ); //XXX? m_defaultSchema );
}

void YzisSchemaConfigPage::reset()
{
  reload ();
}

void YzisSchemaConfigPage::defaults()
{
  reload ();
}

void YzisSchemaConfigPage::update ()
{
  // soft update, no load from disk
  YZSession::me->schemaManager()->update (false);

  schemaCombo->clear ();
  schemaCombo->insertStringList (YZSession::me->schemaManager()->list ());

  defaultSchemaCombo->clear ();
  defaultSchemaCombo->insertStringList (YZSession::me->schemaManager()->list ());

  schemaCombo->setCurrentItem (0);
  schemaChanged (0);

  schemaCombo->setEnabled (schemaCombo->count() > 0);
}

void YzisSchemaConfigPage::deleteSchema ()
{
  int t = schemaCombo->currentItem ();

  YZSession::me->schemaManager()->removeSchema (t);

  update ();
}

void YzisSchemaConfigPage::newSchema ()
{
  QString t = KInputDialog::getText (i18n("Name for New Schema"), i18n ("Name:"), i18n("New Schema"), 0, this);

  YZSession::me->schemaManager()->addSchema (t);

  // soft update, no load from disk
  YZSession::me->schemaManager()->update (false);
  int i = YZSession::me->schemaManager()->list ().findIndex (t);

  update ();
  if (i > -1)
  {
    schemaCombo->setCurrentItem (i);
    schemaChanged (i);
  }
}

void YzisSchemaConfigPage::schemaChanged (int schema)
{
  if (schema < 2)
  {
    btndel->setEnabled (false);
  }
  else
  {
    btndel->setEnabled (true);
  }

  if (m_lastSchema > -1) // so, pressing "Cancel" is no good here?
  {
    m_colorTab->writeConfig (YZSession::me->schemaManager()->schema(m_lastSchema));
    m_fontTab->writeConfig (YZSession::me->schemaManager()->schema(m_lastSchema));
  }

  m_colorTab->readConfig (YZSession::me->schemaManager()->schema(schema));
  m_fontTab->readConfig (YZSession::me->schemaManager()->schema(schema));
  m_fontColorTab->schemaChanged (schema);
  m_highlightTab->schemaChanged (schema);

  m_lastSchema = schema;
}

void YzisSchemaConfigPage::newCurrentPage (QWidget *w)
{
  if (w == m_highlightTab)
    m_highlightTab->schemaChanged (m_lastSchema);
}
//END YzisSchemaConfigPage

// BEGIN SCHEMA ACTION
void YzisViewSchemaAction::init()
{
  m_view = 0;
  last = 0;

  connect(popupMenu(),SIGNAL(aboutToShow()),this,SLOT(slotAboutToShow()));
}

void YzisViewSchemaAction::updateMenu (YZView *view)
{
  m_view = view;
}

void YzisViewSchemaAction::slotAboutToShow()
{
  YZView *view=m_view;
  int count = YZSession::me->schemaManager()->list().count();

  for (int z=0; z<count; z++)
  {
    QString hlName = YZSession::me->schemaManager()->list().operator[](z);

    if (names.contains(hlName) < 1)
    {
      names << hlName;
      popupMenu()->insertItem ( hlName, this, SLOT(setSchema(int)), 0,  z+1);
    }
  }

  if (!view) return;

  popupMenu()->setItemChecked (last, false);
  popupMenu()->setItemChecked (/*XXX?view->renderer()->config()->schema()+*/1, true);

  last = /*XXX?view->renderer()->config()->schema()+*/1;
}

void YzisViewSchemaAction::setSchema (int mode)
{
  YZView *view=m_view;

//XXX?  if (view)
  //XXX?  view->renderer()->config()->setSchema (mode-1);
}
// END SCHEMA ACTION

//BEGIN YzisStyleListView
YzisStyleListView::YzisStyleListView( QWidget *parent, bool showUseDefaults )
    : QListView( parent )
{
  setSorting( -1 ); // disable sorting, let the styles appear in their defined order
  addColumn( i18n("Context") );
  addColumn( SmallIconSet("text_bold"), QString::null );
  addColumn( SmallIconSet("text_italic"), QString::null );
  addColumn( SmallIconSet("text_under"), QString::null );
  addColumn( SmallIconSet("text_strike"), QString::null );
  addColumn( i18n("Normal") );
  addColumn( i18n("Selected") );
  addColumn( i18n("Background") );
  addColumn( i18n("Background Selected") );
  if ( showUseDefaults )
    addColumn( i18n("Use Default Style") );
  connect( this, SIGNAL(mouseButtonPressed(int, QListViewItem*, const QPoint&, int)),
           this, SLOT(slotMousePressed(int, QListViewItem*, const QPoint&, int)) );
  connect( this, SIGNAL(spacePressed(QListViewItem*)),
           this, SLOT(showPopupMenu(QListViewItem*)) );
  // grap the bg color, selected color and default font
  normalcol = KGlobalSettings::textColor();
  bgcol = QColor( "white" );//XXX?YzisRendererConfig::global()->backgroundColor();
  selcol =QColor("black");//XXX?YzisRendererConfig::global()->selectionColor();
  docfont = KGlobalSettings::fixedFont();
//XXX?  docfont = *YzisRendererConfig::global()->font();

  viewport()->setPaletteBackgroundColor( bgcol );
}

void YzisStyleListView::showPopupMenu( YzisStyleListItem *i, const QPoint &globalPos, bool showtitle )
{
  if ( !dynamic_cast<YzisStyleListItem*>(i) ) return;

  KPopupMenu m( this );
  YzisAttribute *is = i->style();
  int id;
  // the title is used, because the menu obscures the context name when
  // displayed on behalf of spacePressed().
  QPixmap cl(16,16);
  cl.fill( i->style()->textColor() );
  QPixmap scl(16,16);
  scl.fill( i->style()->selectedTextColor() );
  QPixmap bgcl(16,16);
  bgcl.fill( i->style()->itemSet(YzisAttribute::BGColor) ? i->style()->bgColor() : viewport()->colorGroup().base() );
  QPixmap sbgcl(16,16);
  sbgcl.fill( i->style()->itemSet(YzisAttribute::SelectedBGColor) ? i->style()->selectedBGColor() : viewport()->colorGroup().base() );

  if ( showtitle )
    m.insertTitle( i->contextName(), YzisStyleListItem::ContextName );
  id = m.insertItem( i18n("&Bold"), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::Bold );
  m.setItemChecked( id, is->bold() );
  id = m.insertItem( i18n("&Italic"), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::Italic );
  m.setItemChecked( id, is->italic() );
  id = m.insertItem( i18n("&Underline"), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::Underline );
  m.setItemChecked( id, is->underline() );
  id = m.insertItem( i18n("S&trikeout"), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::Strikeout );
  m.setItemChecked( id, is->strikeOut() );

  m.insertSeparator();

  m.insertItem( QIconSet(cl), i18n("Normal &Color..."), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::Color );
  m.insertItem( QIconSet(scl), i18n("&Selected Color..."), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::SelColor );
  m.insertItem( QIconSet(bgcl), i18n("&Background Color..."), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::BgColor );
  m.insertItem( QIconSet(sbgcl), i18n("S&elected Background Color..."), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::SelBgColor );

  // Unset [some] colors. I could show one only if that button was clicked, but that
  // would disable setting this with the keyboard (how many aren't doing just
  // that every day? ;)
  // ANY ideas for doing this in a nicer way will be warmly wellcomed.
  YzisAttribute *style = i->style();
  if ( style->itemSet( YzisAttribute::BGColor) || style->itemSet( YzisAttribute::SelectedBGColor ) )
  {
    m.insertSeparator();
    if ( style->itemSet( YzisAttribute::BGColor) )
      m.insertItem( i18n("Unset Background Color"), this, SLOT(unsetColor(int)), 0, 100 );
    if ( style->itemSet( YzisAttribute::SelectedBGColor ) )
      m.insertItem( i18n("Unset Selected Background Color"), this, SLOT(unsetColor(int)), 0, 101 );
  }

  if ( ! i->isDefault() && ! i->defStyle() ) {
    m.insertSeparator();
    id = m.insertItem( i18n("Use &Default Style"), this, SLOT(mSlotPopupHandler(int)), 0, YzisStyleListItem::UseDefStyle );
    m.setItemChecked( id, i->defStyle() );
  }
  m.exec( globalPos );
}

void YzisStyleListView::showPopupMenu( QListViewItem *i )
{
  if ( dynamic_cast<YzisStyleListItem*>(i) )
    showPopupMenu( (YzisStyleListItem*)i, viewport()->mapToGlobal(itemRect(i).topLeft()), true );
}

void YzisStyleListView::mSlotPopupHandler( int z )
{
  ((YzisStyleListItem*)currentItem())->changeProperty( (YzisStyleListItem::Property)z );
}

void YzisStyleListView::unsetColor( int c )
{
  ((YzisStyleListItem*)currentItem())->unsetColor( c );
}

// Because QListViewItem::activatePos() is going to become deprecated,
// and also because this attempt offers more control, I connect mousePressed to this.
void YzisStyleListView::slotMousePressed(int btn, QListViewItem* i, const QPoint& pos, int c)
{
  if ( dynamic_cast<YzisStyleListItem*>(i) ) {
    if ( btn == Qt::RightButton ) {
      showPopupMenu( (YzisStyleListItem*)i, /*mapToGlobal(*/pos/*)*/ );
    }
    else if ( btn == Qt::LeftButton && c > 0 ) {
      // map pos to item/column and call YzisStyleListItem::activate(col, pos)
      ((YzisStyleListItem*)i)->activate( c, viewport()->mapFromGlobal( pos ) - QPoint( 0, itemRect(i).top() ) );
    }
  }
}

//END

//BEGIN YzisStyleListItem
static const int BoxSize = 16;
static const int ColorBtnWidth = 32;

YzisStyleListItem::YzisStyleListItem( QListViewItem *parent, const QString & stylename,
                              YzisAttribute *style, YzisHlItemData *data )
        : QListViewItem( parent, stylename ),
          ds( style ),
          st( data )
{
  initStyle();
}

YzisStyleListItem::YzisStyleListItem( QListView *parent, const QString & stylename,
                              YzisAttribute *style, YzisHlItemData *data )
        : QListViewItem( parent, stylename ),
          ds( style ),
          st( data )
{
  initStyle();
}

void YzisStyleListItem::initStyle()
{
  if (!st)
    is = ds;
  else
  {
    is = new YzisAttribute (*ds);

    if (st->isSomethingSet())
      *is += *st;
  }
}

void YzisStyleListItem::updateStyle()
{
  // nothing there, not update it, will crash
  if (!st)
    return;

  if ( is->itemSet(YzisAttribute::Weight) )
  {
    if ( is->weight() != st->weight())
      st->setWeight( is->weight() );
  }

  if ( is->itemSet(YzisAttribute::Italic) )
  {
    if ( is->italic() != st->italic())
      st->setItalic( is->italic() );
  }

  if ( is->itemSet(YzisAttribute::StrikeOut) )
  {
    if ( is->strikeOut() != st->strikeOut())

      st->setStrikeOut( is->strikeOut() );
  }

  if ( is->itemSet(YzisAttribute::Underline) )
  {
    if ( is->underline() != st->underline())
      st->setUnderline( is->underline() );
  }

  if ( is->itemSet(YzisAttribute::Outline) )
  {
    if ( is->outline() != st->outline())
      st->setOutline( is->outline() );
  }

  if ( is->itemSet(YzisAttribute::TextColor) )
  {
    if ( is->textColor() != st->textColor())
      st->setTextColor( is->textColor() );
  }

  if ( is->itemSet(YzisAttribute::SelectedTextColor) )
  {
    if ( is->selectedTextColor() != st->selectedTextColor())
      st->setSelectedTextColor( is->selectedTextColor() );
  }

  if ( is->itemSet(YzisAttribute::BGColor) )
  {
    if ( is->bgColor() != st->bgColor())
      st->setBGColor( is->bgColor() );
  }

  if ( is->itemSet(YzisAttribute::SelectedBGColor) )
  {
    if ( is->selectedBGColor() != st->selectedBGColor())
      st->setSelectedBGColor( is->selectedBGColor() );
  }
}

/* only true for a hl mode item using it's default style */
bool YzisStyleListItem::defStyle() { return st && st->itemsSet() != ds->itemsSet(); }

/* true for default styles */
bool YzisStyleListItem::isDefault() { return st ? false : true; }

int YzisStyleListItem::width( const QFontMetrics & /*fm*/, const QListView * lv, int col ) const
{
  int m = lv->itemMargin() * 2;
  switch ( col ) {
    case ContextName:
      // FIXME: width for name column should reflect bold/italic
      // (relevant for non-fixed fonts only - nessecary?)
      return QListViewItem::width( QFontMetrics( ((YzisStyleListView*)lv)->docfont), lv, col);
    case Bold:
    case Italic:
    case UseDefStyle:
      return BoxSize + m;
    case Color:
    case SelColor:
    case BgColor:
    case SelBgColor:
      return ColorBtnWidth +m;
    default:
      return 0;
  }
}

void YzisStyleListItem::activate( int column, const QPoint &localPos )
{
  QListView *lv = listView();
  int x = 0;
  for( int c = 0; c < column-1; c++ )
    x += lv->columnWidth( c );
  int w;
  switch( column ) {
    case Bold:
    case Italic:
    case Underline:
    case Strikeout:
    case UseDefStyle:
      w = BoxSize;
      break;
    case Color:
    case SelColor:
    case BgColor:
    case SelBgColor:
      w = ColorBtnWidth;
      break;
    default:
      return;
  }
  if ( !QRect( x, 0, w, BoxSize ).contains( localPos ) )
  changeProperty( (Property)column );
}

void YzisStyleListItem::changeProperty( Property p )
{
  if ( p == Bold )
    is->setBold( ! is->bold() );
  else if ( p == Italic )
    is->setItalic( ! is->italic() );
  else if ( p == Underline )
    is->setUnderline( ! is->underline() );
  else if ( p == Strikeout )
    is->setStrikeOut( ! is->strikeOut() );
  else if ( p == UseDefStyle )
    toggleDefStyle();
  else
    setColor( p );

  updateStyle ();

  ((YzisStyleListView*)listView())->emitChanged();
}

void YzisStyleListItem::toggleDefStyle()
{
  if ( *is == *ds ) {
    KMessageBox::information( listView(),
         i18n("\"Use Default Style\" will be automatically unset when you change any style properties."),
         i18n("Yzis Styles"),
         "Yzis hl config use defaults" );
  }
  else {
    delete is;
    is = new YzisAttribute( *ds );
    repaint();
  }
}

void YzisStyleListItem::setColor( int column )
{
  QColor c; // use this
  QColor d; // default color
  if ( column == Color)
  {
    c = is->textColor();
    d = ds->textColor();
  }
  else if ( column == SelColor )
  {
    c = is->selectedTextColor();
    d = is->selectedTextColor();
  }
  else if ( column == BgColor )
  {
    c = is->bgColor();
    d = ds->bgColor();
  }
  else if ( column == SelBgColor )
  {
    c = is->selectedBGColor();
    d = ds->selectedBGColor();
  }

  if ( KColorDialog::getColor( c, d, listView() ) != QDialog::Accepted) return;

  bool def = ! c.isValid();

  // if set default, and the attrib is set in the default style use it
  // else if set default, unset it
  // else set the selected color
  switch (column)
  {
    case Color:
      if ( def )
      {
        if ( ds->itemSet(YzisAttribute::TextColor) )
          is->setTextColor( ds->textColor());
        else
          is->clearAttribute(YzisAttribute::TextColor);
      }
      else
        is->setTextColor( c );
    break;
    case SelColor:
      if ( def )
      {
        if ( ds->itemSet(YzisAttribute::SelectedTextColor) )
          is->setSelectedTextColor( ds->selectedTextColor());
        else
          is->clearAttribute(YzisAttribute::SelectedTextColor);
      }
      else
        is->setSelectedTextColor( c );
    break;
    case BgColor:
      if ( def )
      {
        if ( ds->itemSet(YzisAttribute::BGColor) )
          is->setBGColor( ds->bgColor());
        else
          is->clearAttribute(YzisAttribute::BGColor);
      }
      else
        is->setBGColor( c );
    break;
    case SelBgColor:
      if ( def )
      {
        if ( ds->itemSet(YzisAttribute::SelectedBGColor) )
          is->setSelectedBGColor( ds->selectedBGColor());
        else
          is->clearAttribute(YzisAttribute::SelectedBGColor);
      }
      else
        is->setSelectedBGColor( c );
    break;
  }

  repaint();
}

void YzisStyleListItem::unsetColor( int c )
{
  if ( c == 100 && is->itemSet(YzisAttribute::BGColor) )
    is->clearAttribute(YzisAttribute::BGColor);
  else if ( c == 101 && is->itemSet(YzisAttribute::SelectedBGColor) )
    is->clearAttribute(YzisAttribute::SelectedBGColor);
}

void YzisStyleListItem::paintCell( QPainter *p, const QColorGroup& /*cg*/, int col, int width, int align )
{

  if ( !p )
    return;

  QListView *lv = listView();
  if ( !lv )
    return;
  Q_ASSERT( lv ); //###

  // use a private color group and set the text/highlighted text colors
  QColorGroup mcg = lv->viewport()->colorGroup();

  if ( col ) // col 0 is drawn by the superclass method
    p->fillRect( 0, 0, width, height(), QBrush( mcg.base() ) );

  int marg = lv->itemMargin();

  QColor c;

  switch ( col )
  {
    case ContextName:
    {
      mcg.setColor(QColorGroup::Text, is->textColor());
      mcg.setColor(QColorGroup::HighlightedText, is->selectedTextColor());
      // text background color
      c = is->bgColor();
      if ( c.isValid() && is->itemSet(YzisAttribute::BGColor) )
        mcg.setColor( QColorGroup::Base, c );
      if ( isSelected() && is->itemSet(YzisAttribute::SelectedBGColor) )
      {
        c = is->selectedBGColor();
        if ( c.isValid() )
          mcg.setColor( QColorGroup::Highlight, c );
      }
      QFont f ( ((YzisStyleListView*)lv)->docfont );
      p->setFont( is->font(f) );
      // FIXME - repainting when text is cropped, and the column is enlarged is buggy.
      // Maybe I need painting the string myself :(
      // (wilbert) it depends on the font used
      QListViewItem::paintCell( p, mcg, col, width, align );
    }
    break;
    case Bold:
    case Italic:
    case Underline:
    case Strikeout:
    case UseDefStyle:
    {
      // Bold/Italic/use default checkboxes
      // code allmost identical to QCheckListItem
      int x = 0;
      int y = (height() - BoxSize) / 2;

      if ( isEnabled() )
        p->setPen( QPen( mcg.text(), 2 ) );
      else
        p->setPen( QPen( lv->palette().color( QPalette::Disabled, QColorGroup::Text ), 2 ) );

      p->drawRect( x+marg, y+2, BoxSize-4, BoxSize-4 );
      x++;
      y++;
      if ( (col == Bold && is->bold()) ||
          (col == Italic && is->italic()) ||
          (col == Underline && is->underline()) ||
          (col == Strikeout && is->strikeOut()) ||
          (col == UseDefStyle && *is == *ds ) )
      {
        QPointArray a( 7*2 );
        int i, xx, yy;
        xx = x+1+marg;
        yy = y+5;
        for ( i=0; i<3; i++ ) {
          a.setPoint( 2*i,   xx, yy );
          a.setPoint( 2*i+1, xx, yy+2 );
          xx++; yy++;
        }
        yy -= 2;
        for ( i=3; i<7; i++ ) {
          a.setPoint( 2*i,   xx, yy );
          a.setPoint( 2*i+1, xx, yy+2 );
          xx++; yy--;
        }
        p->drawLineSegments( a );
      }
    }
    break;
    case Color:
    case SelColor:
    case BgColor:
    case SelBgColor:
    {
      bool set( false );
      if ( col == Color)
      {
        c = is->textColor();
        set = is->itemSet(YzisAttribute::TextColor);
      }
      else if ( col == SelColor )
      {
        c = is->selectedTextColor();
        set = is->itemSet( YzisAttribute::SelectedTextColor);
      }
      else if ( col == BgColor )
      {
        set = is->itemSet(YzisAttribute::BGColor);
        c = set ? is->bgColor() : mcg.base();
      }
      else if ( col == SelBgColor )
      {
        set = is->itemSet(YzisAttribute::SelectedBGColor);
        c = set ? is->selectedBGColor(): mcg.base();
      }

      // color "buttons"
      int x = 0;
      int y = (height() - BoxSize) / 2;
      if ( isEnabled() )
        p->setPen( QPen( mcg.text(), 2 ) );
      else
        p->setPen( QPen( lv->palette().color( QPalette::Disabled, QColorGroup::Text ), 2 ) );

      p->drawRect( x+marg, y+2, ColorBtnWidth-4, BoxSize-4 );
      p->fillRect( x+marg+1,y+3,ColorBtnWidth-7,BoxSize-7,QBrush( c ) );
      // if this item is unset, draw a diagonal line over the button
      if ( ! set )
        p->drawLine( x+marg-1, BoxSize-3, ColorBtnWidth-4, y+1 );
    }
    //case default: // no warning...
  }
}
//END

//BEGIN YzisStyleListCaption
YzisStyleListCaption::YzisStyleListCaption( QListView *parent, const QString & name )
      :  QListViewItem( parent, name )
{
}

void YzisStyleListCaption::paintCell( QPainter *p, const QColorGroup& /*cg*/, int col, int width, int align )
{
  QListView *lv = listView();
  if ( !lv )
    return;
  Q_ASSERT( lv ); //###

  // use the same colorgroup as the other items in the viewport
  QColorGroup mcg = lv->viewport()->colorGroup();

  QListViewItem::paintCell( p, mcg, col, width, align );
}
//END

// kate: space-indent on; indent-width 2; replace-tabs on;
