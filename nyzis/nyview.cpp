/*
   Copyright (c) 2003-2004 Thomas Capricelli <orzel@freehackers.org>,
   Copyright (c) 2003-2006 Loic Pauleve <panard@inzenet.org>
   Copyright (c) 2004-2005 Mickael Marchand <marchand@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of version 2 of the GNU General Public
   License as published by the Free Software Foundation

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


/* Std */
#include <ctype.h>

/* Qt */
#include <qtimer.h>
#include "debug.h"

/* yzis */
#include "nysession.h"
#include "buffer.h"
#include "color.h"

const QRgb RGB_MASK = 0x00ffffff;                // masks RGB values

/*
 * some color used internally
 */
#define attribRed    mAttributesMap[0xff0000]
#define attribWhite  mAttributesMap[0xffffff]
#define attribYellow mAttributesMap[0xffff00]
#define attribMarker attribRed
#define attribBlue mAttributesMap[0x0000ff]
#define attribIntro mAttributesMap[0x00ff00]

#define dbg() yzDebug("NYView")
#define err() yzError("NYView")
#define deepdbg() yzDeepDebug("NYView")

int NYView::attributesMapInitialised = 0;
QMap<QRgb, unsigned long int> NYView::mAttributesMap;


NYView::NYView(YBuffer *b)
        : YView(b, NYSession::self(), 0, 0)
        , editor(0)
        , infobar(this)
{
    dbg() << "NYView( " << b->toString() << " )" << endl;
    statusbarHasCommand = false;

    if ( !attributesMapInitialised ) initialiseAttributesMap();
    YASSERT( b );
    dbg() << "NYView::NYView buffer is : " << (long int)b << endl;
    window = NULL;
    fakeLine = false;
    m_focus = w_editor;
	marginLeft = 0;
}

NYView::~NYView()
{
    dbg() << "~NYView()" << endl;
    if ( window ) unmap();
}

void NYView::map( void )
{
    deepdbg() << "map()" << endl;

	getmaxyx(stdscr, height, width);

    // main editor, fullscreen
    window = newwin( 0, 0, 0, 0 ); YASSERT( window );
    touchwin( window ); // throw away optimisations because we're going to subwin , as said in doc

    editor = subwin( window, height-2, 0, 0, 0); YASSERT( editor );
    wattrset( editor, A_NORMAL );
    wmove( editor, 0, 0 );
    keypad( editor , true); //active symbols for special keycodes
    scrollok( editor, false );

    // creates layout
    /*
     * ------------------ infobar ---------------------
     * ------------------ statusbar -------------------
     */
	infobar.setup(window, height-2);

    statusbar = subwin(window, 1, 0, height-1, 0); YASSERT( statusbar );
    wattrset(statusbar, A_NORMAL | A_BOLD );
    if (has_colors())
        wattron(statusbar, attribWhite );
    statusbarHasCommand = false;

	updateVis();
}


void NYView::unmap( void )
{
    deepdbg() << "unmap()" << endl;
    YASSERT( statusbar ); delwin( statusbar );
    YASSERT( editor ); delwin( editor );
    YASSERT( window ); delwin( window );
    window = editor = statusbar = NULL;
}

void NYView::updateVis()
{
	getmaxyx(stdscr, height, width);
	setVisibleArea(width-marginLeft, height-2);
}

void NYView::guiScroll( int /* dx */, int dy )
{
	/* TODO */
    scrollok( editor, true );
    wscrl( editor, -dy );
    scrollok( editor, false );

    int left = 0;
    int top = 0;
    int n = qAbs(dy);
    if ( dy < 0 ) {
        /* redraw the new bottom */
        top += getLinesVisible() - n;
    }
    sendPaintEvent(YCursor(left, top ), YCursor( left + getColumnsVisible(), top + n ) );
}

void NYView::guiNotifyContentChanged( const YSelection& s )
{
    guiPaintEvent( s );
}
void NYView::guiPreparePaintEvent()
{}
void NYView::guiEndPaintEvent()
{}

void NYView::guiDrawCell( YCursor pos, const YDrawCell& cell )
{
    int x = pos.x();
    if ( !fakeLine ) {
        /* if this line is a fake, don't apply margins */
        x += marginLeft;
    }

	/*
	 * TODO: reverse bg/fg... but... how set bg ? 
	if ( cell.sel & YSelectionPool::Visual ) {
	 }
	 */

	int mAttributes = attribWhite;
	if ( cell.fg.isValid() ) {
		int rawcolor = cell.fg.rgb() & RGB_MASK;
		if ( mAttributesMap.contains( rawcolor ) ) {
			mAttributes = mAttributesMap[ rawcolor ];
		} else {
			yzWarning() << "Unknown color from libyzis, cell.fg.name() is " << cell.fg.name() << endl;
		}
	}
	if ( cell.sel ) mAttributes |= A_REVERSE; // TODO, reverse bg/fg
    //if ( drawUnderline() ) mAttributes |= A_UNDERLINE;

    /* convert string to wide_char */
    QByteArray my_char = cell.c.toLocal8Bit();
    char* from_char = new char[ my_char.length() + 1 ];
    strcpy( from_char, my_char.constData() );
    size_t needed = mbstowcs( NULL, from_char, strlen(from_char) ) + 1;
    wchar_t* wide_char = (wchar_t*)malloc( needed * sizeof(wchar_t) );
    mbstowcs( wide_char, from_char, strlen( from_char ) );
    wide_char[needed - 1] = '\0';

    wattron( editor, mAttributes );
    mvwaddwstr( editor, pos.y(), x, wide_char );
    wattroff( editor, mAttributes );
    free( wide_char );
    delete[] from_char;
}

void NYView::guiPaintEvent( const YSelection& drawMap )
{
    if (!editor) // Avoid segfaults and infinite recursion.
        return ;
    YView::guiPaintEvent( drawMap );
    drawCursor();
}
void NYView::drawCursor()
{
    int x = getCursor().x() + marginLeft;
    wmove( editor, getCursor().y(), x );
    wrefresh( editor );
}

void NYView::guiDrawClearToEOL( YCursor pos, const YDrawCell& cell )
{
	QChar clearChar = cell.c[0]; /* TODO */
    int x = pos.x();
    if ( !fakeLine )
        x += marginLeft;
    if ( clearChar.isSpace() ) {
        /* optimization */
        wmove( editor, pos.y(), x );
        wclrtoeol( editor );
    } else {
        QString erase;
        erase.fill( clearChar, width - x );
        mvwaddstr( editor, pos.y(), x, erase.toLocal8Bit().constData() );
    }
}

void NYView::guiDrawSetMaxLineNumber( int max )
{
    int my_marginLeft = 2 + QString::number( max ).length();
    if ( my_marginLeft != marginLeft ) {
        marginLeft = my_marginLeft;
        updateVis();
    }
}
void NYView::guiDrawSetLineNumber( int y, int n, int h )
{
    fakeLine = n <= 0;

    QString num;
    if ( !fakeLine && h == 0 )
        num = QString::number( n );
    num = num.rightJustified( marginLeft - 1, ' ' ) + ' ';

    wattron( editor, attribYellow );
    mvwaddstr( editor, y, 0, num.toLocal8Bit().constData() );
    wattroff( editor, attribYellow );
}


void NYView::guiSetFocusMainWindow()
{
    if ( statusbarHasCommand ) {
        werase(statusbar);
        wrefresh(statusbar);
    }
    m_focus = w_editor;
    drawCursor();
}
void NYView::guiSetFocusCommandLine()
{
    m_focus = w_statusbar;
    wmove(statusbar, 0, guiGetCommandLineText().length() + 1 );
    wrefresh(statusbar);
}
void NYView::restoreFocus()
{
    switch ( m_focus ) {
    case w_editor :
        guiSetFocusMainWindow();
        break;
    case w_statusbar :
        guiSetFocusCommandLine();
        break;
    }
}

void NYView::guiSetCommandLineText( const QString& text )
{
    yzDebug() << "NYView::guiSetCommandLineText: " << text << endl;
    commandline = text;
    static QChar modeChar = ':';
    switch (modePool()->current()->modeType()) {
    case YMode::ModeEx:
        modeChar = ':';
        break;
    case YMode::ModeSearch:
        modeChar = '/';
        break;
    case YMode::ModeSearchBackward:
        modeChar = '?';
        break;
    default:
        modeChar = ' ';
    }
    werase(statusbar);
    waddstr(statusbar, (modeChar + commandline).toLocal8Bit().constData());
    wrefresh(statusbar);
    statusbarHasCommand = true;
}

YStatusBarIface* NYView::guiStatusBar()
{
    return &infobar;
}

void NYView::initialiseAttributesMap()
{
    attributesMapInitialised = true;

    if ( !has_colors() ) {
        yzWarning() << "Terminal doesn't have color capabilities, disabling syntax highlighting" << endl;
        return ;
    }
    bool changecolorok = (can_change_color() == true);
    yzWarning() << "Terminal can";
    if (!changecolorok)
        yzWarning() << " _not_";
    yzWarning() << " change colors" << endl;

    dbg() << "COLOR_PAIRS is : " << COLOR_PAIRS << endl;
    dbg() << "COLORS      is : " << COLORS << endl;


#undef MAP
#define RAWMAP( nb, rawcolor, color, attributes )               \
    YASSERT( ERR != init_pair( nb, (color), -1 /*COLOR_BLACK*/ ) );    \
    mAttributesMap[(rawcolor)] = COLOR_PAIR((nb)) | (attributes);
#define MAP( nb, qtcolor, color, attributes )               \
    RAWMAP((nb),YColor(qtcolor).rgb() & RGB_MASK,(color),(attributes))
    // first arg is the new rawcolor, second arg is the one that should be used
#define ALIASMAP(rawcolor1,rawcolor2) \
    YASSERT( ! mAttributesMap.contains( rawcolor1) ); \
    mAttributesMap[(rawcolor1)] = mAttributesMap[(rawcolor2)];

    MAP( 1, Qt::red, COLOR_RED, A_BOLD ); // red = 1, is used to display info on statusbar..
    YASSERT( ERR != init_pair( 2, COLOR_WHITE, COLOR_BLUE) ); \
    MAP( 3, Qt::yellow, COLOR_YELLOW, A_BOLD );
    MAP( 4, Qt::lightGray, COLOR_WHITE, A_NORMAL );
    MAP( 5, Qt::gray, COLOR_WHITE, A_NORMAL );
    MAP( 6, Qt::white, COLOR_WHITE, A_BOLD );
    MAP( 7, Qt::magenta, COLOR_MAGENTA, A_BOLD );
    MAP( 8, Qt::green, COLOR_GREEN, A_BOLD );
    MAP( 9, Qt::cyan, COLOR_CYAN, A_BOLD );
    MAP(10, Qt::black, COLOR_BLACK, A_NORMAL );
    MAP(11, Qt::blue, COLOR_BLUE, A_BOLD );
    MAP(12, Qt::darkGreen, COLOR_GREEN, A_NORMAL );
    MAP(13, Qt::darkMagenta, COLOR_MAGENTA, A_NORMAL );
    MAP(14, Qt::darkCyan, COLOR_CYAN, A_NORMAL );
    // ALIASMAP(0xc0c0c0); // already here (lightGray)
    ALIASMAP(0xDD0000, 0xff0000); // red


}

void NYView::refreshScreen()
{
    if ( marginLeft > 0 && !getLocalBooleanOption("number") )
        marginLeft = 0;
    YView::refreshScreen();
    refresh();
    updateCursor();
}

#if 0

// we once tried to use that, but failed (miserably) and as far as links/lynx/elinks don't
// use that neither...

// magenta = 1, is used to display info on statusbar..
//
if ( changecolorok )
{

#define COLOR_QT2CURSES(a) ((a)*1000/256)
#define COLOR_CURSES2QT(a) ((a)*256/1000)
#define MAP( nb, color )       \
    init_color( nb, COLOR_QT2CURSES(qRed(color.rgb())), COLOR_QT2CURSES( qGreen(color.rgb())), COLOR_QT2CURSES( qBlue(color.rgb()))) ; \
    YASSERT ( ERR != init_pair( nb, nb%COLORS, COLOR_BLACK ) );    \
    mColormap[color.rgb()] = nb;

    /*
    init_color( 1,
      (short)qRed(Qt::magenta.rgb()),
      (short)qGreen(Qt::magenta.rg),
      (short)qBlue(Qt::magenta));
    init_pair( 1, 1, COLOR_BLACK );
    mColormap[Qt::magenta] = 1;
      */

    MAP( 1, Qt::magenta );
    MAP( 2, Qt::white );
    MAP( 3, Qt::darkGray );
    MAP( 4, Qt::gray );
    MAP( 5, Qt::lightGray );
    MAP( 6, Qt::red );
    MAP( 7, Qt::green );
    MAP( 8, Qt::blue );
    MAP( 9, Qt::cyan );
    MAP( 10, Qt::black );
    MAP( 11, Qt::yellow );
    MAP( 12, Qt::darkRed );
    MAP( 13, Qt::darkGreen );
    MAP( 14, Qt::darkBlue );
    MAP( 15, Qt::darkCyan );
    MAP( 16, Qt::darkMagenta );
    MAP( 17, Qt::darkYellow );

} else

#endif

    bool NYView::guiPopupFileSaveAs()
{
    // TODO
    displayInfo ("Save as not implemented yet, use :w<filename>");
    return false;
}

void NYView::guiUpdateFileName()
{
    QString filename = myBuffer()->fileName();
    int lineCount = myBuffer()->lineCount();
    int wholeLength = myBuffer()->getWholeTextLength();
    displayInfo(QString("\"%1\" %2L, %3C" ).arg(filename).arg(lineCount).arg(wholeLength));
    restoreFocus();
}

void NYView::guiUpdateMode()
{
    restoreFocus();
}

void NYView::guiUpdateFileInfo()
{
    restoreFocus();
}

void NYView::guiUpdateCursor()
{
    restoreFocus();
}

void NYView::guiDisplayInfo(const QString&)
{
    restoreFocus();
}

void NYView::guiHighlightingChanged()
{}

