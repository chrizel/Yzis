/**
 * $Id$
 */
#include "kyzis.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char *description =
    I18N_NOOP("KDE Frontend for the YZis Editor");

static const char *version = "v0.0.1";

static KCmdLineOptions options[] = {
    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv) {
    KAboutData about("kyzis", I18N_NOOP("Kyzis"), version, description, KAboutData::License_GPL, "(C) 2003 Yzis Team", 0, 0, "yzis-dev@yzis.org");
    about.addAuthor( "Yzis Team", 0, "yzis-dev@yzis.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;

    // see if we are starting with session management
    if (app.isRestored())
        RESTORE(Kyzis)
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if ( args->count() == 0 ) {
        Kyzis *widget = new Kyzis;
        widget->show();
        } else {
            int i = 0;
            for (; i < args->count(); i++ ) {
                Kyzis *widget = new Kyzis;
                widget->show();
                widget->load( args->url( i ) );
            }
        }
        args->clear();
    }

    return app.exec();
}
