/**
 * $id$
 */

#include "yz_commands.h"
#include "yz_view.h"

/**
 * Here is a simple example to add a new command and how to use it :
 * This binds the keystroke "test", to the function YZCommandPool::test()
 * Note that you have to precise on which kind of objects you intend to operate ( here
 * it's the pool)
 * The third parameter specify whether your command will be overwritable or not ( not working
 * yet )
 * NEW_POOL_COMMAND("test", &YZCommandPool::test(),true);
 *
 * This calls the test function with an empty arg list using the 'this' instance :
 *	QString result = ( *this.*( globalCommands[ "test" ].function ) )( QStringList() );
 *
 * Thanks to this you can access commands on any view/buffer.
 * This should also allow us to 'remap' commands, and dynamically add new ones :)
 */
YZCommandPool::YZCommandPool() {
	//this creates the default commands
	initPool();
}

YZCommandPool::~YZCommandPool() {
}

void YZCommandPool::initPool() {
	//our basic example
	NEW_POOL_COMMAND("test",&YZCommandPool::test,true);

	//normal stuff
	NEW_VIEW_COMMAND("j",&YZView::moveDown,true);
}

QString YZCommandPool::test(QStringList) {
	return QString("testing");
}

