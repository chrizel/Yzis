#ifndef NYSTATUSBAR_H
#define NYSTATUSBAR_H

#include <ncurses.h>
#include <QString>
#include "libyzis/statusbariface.h"

class NYView;

/** ncurses status bar implementation */
class NYStatusBar : public YStatusBarIface
{
public:
    /** Initialize the status bar */
    NYStatusBar(NYView *view);
    virtual ~NYStatusBar();

    /** Display current mode */
    virtual void setMode(const QString& mode);
    /** Display current file name */
    virtual void setFileName(const QString& filename);
    /** Display current file status information */
    virtual void setFileInfo(bool isNew, bool isModified);
    /** Display current position within the buffer */
    virtual void setLineInfo(int bufferLine, int bufferColumn, int screenColumn, QString percentage);
    /** Display an informational message */
    virtual void setMessage(const QString& message);

    /** Initialize the status bar subwindow.
     * If the subwindow already exists we delete it beforehand.
     * @param mainwin view's window, parent of m_bar
     */
    void setup(WINDOW *mainwin);

private:
    /** Paint current information */
    void refresh();

    /** current mode */
    QString m_currentMode;
    /** current file name */
    QString m_currentFilename;
    /** whether the file has been modified */
    bool m_isModified;
    /** current line info */
    QString m_currentLineinfo;
    /** current informational message */
    QString m_currentMessage;

    /** NCurses window */
    WINDOW *m_bar;
    /** Status bar owner */
    NYView *m_view;
};

#endif

