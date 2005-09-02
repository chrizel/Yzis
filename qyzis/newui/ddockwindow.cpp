/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "ddockwindow.h"

#include <qtoolbutton.h>
#include <qlayout.h>
#include <qstyle.h>
#include <q3widgetstack.h>
#include <qimage.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

#include <kglobal.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>

#include "buttonbar.h"
#include "button.h"
#include "debug.h"

DDockWindow::DDockWindow(QWidget *parent, Position position)
    :Q3DockWindow(Q3DockWindow::InDock, parent), m_position(position), m_expanded(false),
    m_toggledButton(0)
{
    setMovingEnabled(false);
    setResizeEnabled(true);

    Ideal::Place place = Ideal::Left;
    switch (position) {
        case DDockWindow::Bottom:
            m_name = "BottomToolWindow";
            place = Ideal::Bottom;
            m_internalLayout = new QVBoxLayout(boxLayout(), 0);
            m_internalLayout->setDirection(QBoxLayout::BottomToTop);
            break;
        case DDockWindow::Left:
            m_name = "LeftToolWindow";
            place = Ideal::Left;
            m_internalLayout = new QHBoxLayout(boxLayout(), 0);
            m_internalLayout->setDirection(QBoxLayout::LeftToRight);
            break;
        case DDockWindow::Right:
            m_name = "RightToolWindow";
            place = Ideal::Right;
            m_internalLayout = new QHBoxLayout(boxLayout(), 0);
            m_internalLayout->setDirection(QBoxLayout::RightToLeft);
            break;
    }

    KConfig *config = kapp->config();
    config->setGroup("UI");
    int mode = config->readNumEntry("MDIStyle", 3);
    Ideal::ButtonMode buttonMode = Ideal::Text;
    if (mode == 0)
        buttonMode = Ideal::Icons;
    else if (mode == 1)
        buttonMode = Ideal::Text;
    else if (mode == 3)
        buttonMode = Ideal::IconsAndText;

    m_bar = new Ideal::ButtonBar(place, buttonMode, this);
    m_internalLayout->addWidget(m_bar);

    m_widgetStack = new QStackedWidget(this);
    m_internalLayout->addWidget(m_widgetStack);

    setExpanded(m_expanded);

    loadSettings();
}

DDockWindow::~DDockWindow()
{
    saveSettings();
}

void DDockWindow::setExpanded(bool v)
{
    //write dock width to the config file
    KConfig *config = kapp->config();
    QString group = QString("%1").arg(m_name);
    config->setGroup(group);

    if (m_expanded)
        config->writeEntry("ViewWidth", m_position == DDockWindow::Bottom ? height() : width() );

    m_widgetStack->setVisible(v);
    m_expanded = v;

    m_internalLayout->invalidate();
    if (!m_expanded)
        if (m_position == DDockWindow::Bottom)
            setFixedExtentHeight(m_internalLayout->sizeHint().height());
        else
            setFixedExtentWidth(m_internalLayout->sizeHint().width());
    else
    {
        //restore widget size from the config
        int size = 0;
        if (m_position == DDockWindow::Bottom)
        {
            size = config->readNumEntry("ViewWidth", m_internalLayout->sizeHint().height());
            setFixedExtentHeight(size);
        }
        else
        {
            size = config->readNumEntry("ViewWidth", m_internalLayout->sizeHint().width());
            setFixedExtentWidth(size);
        }
    }
    /***** Qt 3 to 4 porting hack (harryF) *****/
    QWidget *topMost = this->window();
    if (topMost && topMost->layout()) {
        topMost->layout()->invalidate();
        topMost->layout()->activate();
    }
}

void DDockWindow::loadSettings()
{
}

void DDockWindow::saveSettings()
{
    KConfig *config = kapp->config();
    QString group = QString("%1").arg(m_name);
    int invisibleWidth = 0;
    config->setGroup(group);
    if (config->hasKey("ViewWidth"))
        invisibleWidth = config->readNumEntry("ViewWidth");
    config->deleteEntry("ViewWidth");
    config->deleteEntry("ViewLastWidget");
    if (m_toggledButton && m_expanded)
    {
        config->writeEntry("ViewWidth", m_position == DDockWindow::Bottom ? height() : width());
        config->writeEntry("ViewLastWidget", m_toggledButton->realText());
    }
    else if (invisibleWidth != 0)
        config->writeEntry("ViewWidth", invisibleWidth);
}

QWidget *DDockWindow::currentWidget() const
{
    return m_widgetStack->currentWidget();
}

void DDockWindow::addWidget(const QString &title, QWidget *widget)
{
    QPixmap *pm = const_cast<QPixmap*>(widget->icon());
    Ideal::Button *button;
    if (pm != 0)
    {
        //force 16pt for now
        if (pm->height() > 16)
        {
            QImage img = pm->convertToImage();
            img = img.smoothScale(16, 16);
            pm->convertFromImage(img);
        }
        button = new Ideal::Button(m_bar, title, *pm);
    }
    else
        button = new Ideal::Button(m_bar, title);
    m_widgets[button] = widget;
    m_buttons[widget] = button;
    m_bar->addButton(button);

    m_widgetStack->addWidget(widget);
    connect(button, SIGNAL(clicked()), this, SLOT(selectWidget()));

    //if the widget was selected last time the dock is deleted
    //we need to show it
    KConfig *config = kapp->config();
    QString group = QString("%1").arg(m_name);
    config->setGroup(group);
    if (config->readEntry("ViewLastWidget") == title)
    {
        kdDebug() << k_funcinfo << " : activating last widget " << title << endl;
        button->setOn(true);
        selectWidget(button);
    }
}

void DDockWindow::raiseWidget(QWidget *widget)
{
    kdDebug() << k_funcinfo << endl;
    Ideal::Button *button = m_buttons[widget];
    if ((button != 0) && (!button->isOn()))
    {
        button->setOn(true);
        selectWidget(button);
    }
}

void DDockWindow::removeWidget(QWidget *widget)
{
    kdDebug() << k_funcinfo << endl;
    if (m_widgetStack->indexOf(widget) == -1)
        return; //not in dock

    bool changeVisibility = false;
    if (m_widgetStack->currentWidget() == widget)
        changeVisibility = true;

    Ideal::Button *button = m_buttons[widget];
    if (button)
        m_bar->removeButton(button);
    m_widgets.remove(button);
    m_buttons.remove(widget);
    m_widgetStack->removeWidget(widget);

    if (changeVisibility)
    {
        m_toggledButton = 0;
        setExpanded(false);
    }
}

void DDockWindow::selectWidget(Ideal::Button *button)
{
    kdDebug() << k_funcinfo << endl;
    if (m_toggledButton == button)
    {
        setExpanded(!m_expanded);
        return;
    }

    if (m_toggledButton)
        m_toggledButton->setOn(false);
    m_toggledButton = button;
    setExpanded(true);
    m_widgetStack->setCurrentWidget(m_widgets[button]);
}

void DDockWindow::selectWidget()
{
    selectWidget((Ideal::Button*)sender());
}

void DDockWindow::hideWidget(QWidget *widget)
{
    Ideal::Button *button = m_buttons[widget];
    if (button != 0)
    {
        button->setOn(false);
        button->hide();
    }
    widget->hide();
    if (button == m_toggledButton)
        setExpanded(false);
}

void DDockWindow::showWidget(QWidget *widget)
{
    Ideal::Button *button = m_buttons[widget];
    if (button != 0)
        button->show();
    widget->show();
}

void DDockWindow::setMovingEnabled(bool b)
{
    //some operations on KMainWindow cause moving to be enabled
    //but we always don't want DDockWindow instances to be movable
    Q3DockWindow::setMovingEnabled(false);
}

#include "ddockwindow.moc"
