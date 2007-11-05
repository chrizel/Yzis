#include "nyview.h"
#include "nystatusbar.h"

#include "libyzis/debug.h"
#define dbg()     yzDebug("NYStatusBar")
#define err()     yzError("NYStatusBar")
#define deepdbg() yzDeepDebug("NYStatusBar")

NYStatusBar::NYStatusBar(NYView *view)
{
    dbg().sprintf("NStatusBar( nyview=%s )\n", qp(view->toString()));
    m_view = view;
    m_bar = NULL;

    m_currentMode = QString();
    m_currentMessage = QString();
    m_currentFilename = QString();
    m_currentLineinfo = QString();
    m_isModified = false;
}

NYStatusBar::~NYStatusBar()
{
    dbg() << "~NYStatusBar()" << endl;
    YASSERT(m_bar); delwin(m_bar);
}

void NYStatusBar::setup( WINDOW* mainwin )
{
    dbg() << QString().sprintf("setup( mainwin = %p )", mainwin) << endl;
    if (m_bar)
        delwin(m_bar);

    m_bar = subwin(mainwin, 1, 0, m_view->getLinesVisible() + 0, 0); YASSERT(m_bar);
    wattrset(m_bar, A_REVERSE);
    wbkgd(m_bar, A_REVERSE );           // so that blank char are reversed, too
}

void NYStatusBar::refresh()
{
    dbg() << "refresh() Refreshing the status bar" << endl;
    // We want to the bar to expand and compress when there is less space.
    // In the most tight situation every element is separated by one cell.
    // If there is plenty of space we move away the lineinfo 5 cells from the right border.
    // If there isn't enough space we delete the path starting from the beginning.

    QString central;
    if (m_currentMessage.isEmpty()) {
        central = m_currentFilename + ' ';
        if (m_isModified)
            central += "[+] ";
    } else {
        central = m_currentMessage + ' ';
    }

    int spaceforlineinfo = m_currentLineinfo.size() + 1;

    int freespace = m_view->getColumnsVisible() - m_currentMode.size() - 1
                    - central.size() - spaceforlineinfo;

    if (freespace < 0)
        central.remove(0, -freespace); // make room for the lineinfo
    else
        spaceforlineinfo += (freespace > 10) ? 5 : freespace / 2;

    werase(m_bar);

    wmove(m_bar, 0, 0);
    wattron(m_bar, COLOR_PAIR(2));
    wprintw(m_bar, "%s", m_currentMode.toLocal8Bit().constData());
    wattroff(m_bar, COLOR_PAIR(2));

    waddch(m_bar, ' ');

    wprintw(m_bar, "%s", central.toLocal8Bit().constData());

    mvwprintw(m_bar, 0, m_view->getColumnsVisible() - spaceforlineinfo,
              m_currentLineinfo.toLocal8Bit().constData());

    wrefresh(m_bar);
}

void NYStatusBar::setMode(const QString& mode)
{
    deepdbg() << "setMode( " << mode << " )" << endl;

    m_currentMode = mode;
    refresh();
}

void NYStatusBar::setFileName(const QString& filename)
{
    deepdbg() << "setFileName( " << filename << " )" << endl;

    m_currentFilename = filename;
    refresh();
}

void NYStatusBar::setFileInfo(bool isNew, bool isModified)
{
    deepdbg() << QString("setFileInfo( isNew=%1, isModified=%2 )")
    .arg(isNew).arg(isModified) << endl;

    m_isModified = isModified;
    refresh();
}

void NYStatusBar::setLineInfo(int bufferLine, int bufferColumn,
                              int screenColumn, QString percentage)
{
    deepdbg() << QString("setLineInfo( %1, %2, %3, %4 )")
                 .arg(bufferLine)
                 .arg(bufferColumn)
                 .arg(screenColumn)
                 .arg(percentage)
    << endl;

    bool isNumber;
    percentage.toInt(&isNumber);
    if (isNumber)
        percentage += '%';

    if (bufferColumn != screenColumn) {
        m_currentLineinfo = QString("%1,%2-%3 (%4)")
                            .arg( bufferLine )
                            .arg( bufferColumn )
                            .arg( screenColumn )
                            .arg( percentage );
    } else {
        m_currentLineinfo = QString("%1,%2 (%3)")
                            .arg( bufferLine )
                            .arg( bufferColumn )
                            .arg( percentage );
    }

    refresh();
}

void NYStatusBar::setMessage(const QString& message)
{
    deepdbg() << "setMessage( " << message << " )" << endl;

    m_currentMessage = message;
    refresh();
}

