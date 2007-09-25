/*
    Copyright (c) 2007 Lothar Braun <lothar@lobraun.de>

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

#include "main.h"

#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/editorchooser.h>

#include <kxmlguifactory.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kservice.h>


MainWindow::MainWindow( KTextEditor::Document* doc )
    : document( 0 ),
      view( 0 )
{
    if ( !doc ) {
        KTextEditor::Editor* editor = NULL;
        KService::Ptr serv = KService::serviceByDesktopName( "yzispart" );
        if ( !serv ) {
            KMessageBox::error( this, "Could not find yzispart!" );
            kapp->exit( -1 );
        } else {
            editor = KTextEditor::editor( serv->library().toLatin1() );
            if ( !editor ) {
                KMessageBox::error( this, "Could not create yziskpart editor component" );
                kapp->exit( -1 );
            }
        }

        document = editor->createDocument( 0 );
    } else {
        document = doc;
    }
    view = qobject_cast< KTextEditor::View* >( document->createView( this ) );

    setCentralWidget( view );
    guiFactory()->addClient( view );

    show();
}

MainWindow::~MainWindow()
{
    if ( document ) {
        guiFactory()->removeClient( view );
        delete document;
    }
}

int main( int argc, char** argv )
{

        KAboutData aboutData( "kyzis",
                              0,
                              ki18n( "KYZis" ),
                              "M3++",
                              ki18n( "KDE Frontend for YZis" ),
                              KAboutData::License_GPL,
                              ki18n( "(c) 2007" ),
                              ki18n( "http://www.yzis.org" ),
                              "http://yzis.org",
                              "yzis-dev@yzis.org"
                            );
 
    KCmdLineArgs::init( argc, argv, &aboutData );
    KApplication app;

    MainWindow* mainWindow = new MainWindow();
    mainWindow->show();

    return app.exec();
}

#include "main.moc"
