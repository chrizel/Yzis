/*
    Copyright (c) 2003-2004 Mickael Marchand <mikmak@yzis.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* This file was copied from the KDE project (Kate)
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
*/

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <qwidget.h>
#include <kparts/part.h>
#include <qlayout.h>

class Konsole : public QWidget
{
  Q_OBJECT

  public:
    Konsole (QWidget* parent, const char* name);
    ~Konsole ();

    void cd (KURL url=KURL());

    void sendInput( const QString& text );

  protected:
    void focusInEvent( QFocusEvent * ) { if (part) part->widget()->setFocus(); };
    virtual void showEvent(QShowEvent *);


  private:
    KParts::ReadOnlyPart *part;
    QVBoxLayout* lo;
	QWidget *m_kvm;

  public slots:
    void loadConsoleIfNeeded();

  // Only needed for Konsole
  private slots:
    void notifySize (int,int) {};
    void changeColumns (int) {};
    void changeTitle(int,const QString&) {};

    void slotDestroyed ();
};

#endif
