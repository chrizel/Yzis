#include "qystatusbar.h"

#include "libyzis/debug.h"
#define dbg()     yzDebug("QYStatusBar")
#define err()     yzError("QYStatusBar")
#define deepdbg() yzDeepDebug("QYStatusBar")

#include <QStatusBar>
#include <QLabel>


QYStatusBar::QYStatusBar(QWidget *parent)
        : QStatusBar(parent)
{
    dbg() << QString("QYStatusBar( %1 )").arg(qp(parent->objectName())) << endl;
    m_mode = new QLabel(this);
    m_message = new QLabel(this);
    m_fileinfo = new QLabel(this);
    m_lineinfo = new QLabel(this);

    addWidget(m_mode, 1);
    addWidget(m_message, 100);
    addWidget(m_fileinfo, 1);
    addWidget(m_lineinfo, 1);

}

QYStatusBar::~QYStatusBar()
{
    dbg() << "~QYStatusBar()" << endl;
}

void QYStatusBar::setMode(const QString& mode)
{
    deepdbg() << "setMode( " << mode << " )" << endl;
    m_mode->setText(mode);
}

void QYStatusBar::setFileName(const QString& filename)
{}

void QYStatusBar::setFileInfo(bool isNew, bool isModified)
{
    deepdbg() << QString("setFileInfo( isNew=%1, isModified=%2 )").arg(isNew).arg(isModified) << endl;
    QString fileinfo;

    fileinfo += isNew ? 'N' : ' ';
    fileinfo += isModified ? 'M' : ' ';

    m_fileinfo->setText(fileinfo);
}

void QYStatusBar::setLineInfo( int bufferLine, int bufferColumn, int screenColumn, QString percentage )
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

    if (bufferColumn != screenColumn)
        m_lineinfo->setText( QString("%1,%2-%3 (%4)")
                             .arg(bufferLine)
                             .arg(bufferColumn)
                             .arg(screenColumn)
                             .arg(percentage) );
    else
        m_lineinfo->setText( QString("%1,%2 (%3)")
                             .arg(bufferLine)
                             .arg(bufferColumn)
                             .arg(percentage) );
}

void QYStatusBar::setMessage(const QString& message)
{
    deepdbg() << "setMessage( " << message << " )" << endl;
    m_message->setText(message);
}

