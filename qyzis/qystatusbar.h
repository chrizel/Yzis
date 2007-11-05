#ifndef QYZISSTATUSBAR_H
#define QYZISSTATUSBAR_H

#include "libyzis/statusbariface.h"
#include <QStatusBar>

class QLabel;
class QWidget;

/** QYzis status bar implementation */
class QYStatusBar : public QStatusBar, public YStatusBarIface
{
public:
    /** Initialize the status bar */
    QYStatusBar(QWidget *parent);
    virtual ~QYStatusBar();

    /** Displays current mode */
    virtual void setMode(const QString& mode);
    /** Displays current file name */
    virtual void setFileName(const QString& filename);
    /** Displays current file status information */
    virtual void setFileInfo(bool isNew, bool isModified);
    /** Displays current position within the buffer */
    virtual void setLineInfo(int bufferLine, int bufferColumn, int screenColumn, QString percentage);
    /** Displays an informational message */
    virtual void setMessage(const QString& message);

private:
    /** Area where current mode is shown */
    QLabel *m_mode;
    /** Area where file status information is shown */
    QLabel *m_fileinfo;
    /** Area where view's position withing the buffer is shown */
    QLabel *m_lineinfo;
    /** Area where informational messages are shown */
    QLabel *m_message;
};

#endif

