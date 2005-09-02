/*
    Copyright (c) 2003-2005 Mickael Marchand <mikmak@yzis.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* This file was copied from the KDE project (Kate)
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
*/

#include "konsole.moc"
#include <kde_terminal_interface.h>
#include <kurl.h>
#include <klibloader.h>
#include <klocale.h>
#include <kapplication.h>
#include <qlayout.h>
#include "debug.h"

Konsole::Konsole (QWidget* kvm, const char* name) : QWidget (kvm, name),part(0)
{
    lo = new QVBoxLayout(this);
    m_kvm=kvm;
}

Konsole::~Konsole ()
{
	delete part;
}

void Konsole::loadConsoleIfNeeded()
{
  yzDebug()<<"================================ loadConsoleIfNeeded()"<<endl;
  if (part!=0) return;
/*  if (!kapp->loopLevel()) {
	connect(kapp,SIGNAL(onEventLoopEnter()),this,SLOT(loadConsoleIfNeeded()));
	return;
  }
  if (!topLevelWidget() || !parentWidget()) return;
  if (!topLevelWidget() || !isVisibleTo(topLevelWidget())) return;
*/
  yzDebug()<<"CREATING A CONSOLE PART"<<endl;

    KLibFactory *factory = 0;
    factory = KLibLoader::self()->factory("libkonsolepart");
    part = 0L;
      if (factory)
        {
          part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,"libkonsolepart",	"KParts::ReadOnlyPart"));
	  if (part)
	    {
              KGlobal::locale()->insertCatalogue("konsole");
              part->widget()->show();
              lo->addWidget(part->widget());
              connect ( part, SIGNAL(destroyed()), this, SLOT(slotDestroyed()) );
/*	      if (m_kvm->activeView())
	      	if (m_kvm->activeView()->getDoc()->url().isValid())
			cd(KURL( m_kvm->activeView()->getDoc()->url().path() ));*/
            }
        }
}

void Konsole::showEvent(QShowEvent *)
{
  if (!part)
    loadConsoleIfNeeded();
}

void Konsole::cd (KURL url)
{
  if (!part)
    return;

  part->openURL (url);
}

void Konsole::sendInput( const QString& text )
{
  if (!part)
    return;

  TerminalInterface *t = dynamic_cast<TerminalInterface*>( part );

  if (!t)
    return;

  t->sendInput (text);
}

void Konsole::slotDestroyed ()
{
  part=0;

  // hide the dockwidget
  if (parentWidget())
    parentWidget()->hide ();
}

