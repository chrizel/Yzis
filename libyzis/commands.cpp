/**
 * $Id$
 */

#include "commands.h"
#include "view.h"
#include "debug.h"

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
	executor = new YZExExecutor();
}

YZCommandPool::~YZCommandPool() {
	globalCommands.clear();
	globalExCommands.clear();
	delete executor;
}

void YZCommandPool::initPool() {
	//normal stuff
	NEW_VIEW_COMMAND("j",&YZView::moveDown,true);
	NEW_VIEW_COMMAND("k",&YZView::moveUp,true);
	NEW_VIEW_COMMAND("h",&YZView::moveLeft,true);
	NEW_VIEW_COMMAND("l",&YZView::moveRight,true);
	NEW_VIEW_COMMAND("^",&YZView::moveToStartOfLine,true);
	NEW_VIEW_COMMAND("$",&YZView::moveToEndOfLine,true);
	NEW_VIEW_COMMAND("x",&YZView::deleteCharacter,true);
	NEW_VIEW_COMMAND("i",&YZView::gotoInsertMode,true);
	NEW_VIEW_COMMAND(":",&YZView::gotoExMode,true);
	NEW_VIEW_COMMAND("R",&YZView::gotoReplaceMode,true);
	NEW_VIEW_COMMAND("gg",&YZView::gotoLine,true);
	NEW_VIEW_COMMAND("G",&YZView::gotoLine,true);
	NEW_VIEW_COMMAND("dd",&YZView::deleteLine,true);
	NEW_VIEW_COMMAND("o",&YZView::openNewLineAfter,true);
	NEW_VIEW_COMMAND("O",&YZView::openNewLineBefore,true);
	NEW_VIEW_COMMAND("a",&YZView::append,true);
	NEW_VIEW_COMMAND("A",&YZView::appendAtEOL,true);
	NEW_SESS_COMMAND("ZZ",&YZSession::saveBufferExit,true);
}

void YZCommandPool::execCommand(YZView *view, const QString& inputs, int *error) {
	QString result,command;
	int i=0;

	//regexp ? //FIXME
	//try to find the command we are looking for
	//first remove any number at the beginning of command
	//XXX 0 is itself a command !
	while ( inputs[ i ].isDigit() )
		i++; //go on
	
	//now take the command until another number
	while ( !inputs[ i ].isDigit() && i<inputs.length() )
		command += inputs[ i++ ];
	//end FIXME
	QString realcommand = command;
	
	//printf( "%s %s\n", inputs.latin1(), command.latin1() );

	//try hard to find a correspondance
	QMap<QString, YZCommand>::Iterator it = globalCommands.end();
	while ( command.length() > 0 && it == globalCommands.end() ) {
		it = globalCommands.find(command);
		if ( it==globalCommands.end() ) command.truncate(command.length()-1);
	} // should not end here FIXME
	
	if ( it!=globalCommands.end() ) { //we got one match *ouf*
		switch ( globalCommands[ command ].obj ) {
			case VIEW :
				result = ( *view.*(globalCommands[ command ].viewFunc )) (inputs) ;
				break;
			case BUFF :
				result = ( *( view->myBuffer() ).*(globalCommands[ command ].buffFunc )) (inputs) ;
				break;
			case SESS :
				result = ( *( view->mySession() ).*(globalCommands[ command ].sessFunc )) (inputs) ;
				break;
			case POOL :
				result = ( *( view->mySession()->getPool() ).*(globalCommands[ command ].poolFunc )) (inputs) ;
				break;
				/**		case PLUG :
					result = ( *this.*(globalCommands[ command ].viewFunc )) (inputs) ;
					break;*/
			default:
				break;
		}
	} else {
		//if we are here then it's not a sub-command, so
		//is there a command starting with the input-ed characters ?  if yes then
		//wait for other characters otherwise return error to purge input buffer
		bool found=false;
		for ( it = globalCommands.begin(); !found && it != globalCommands.end(); ++it ) {
			yzDebug() << " No match : looking for possible future matches before returning error : " << realcommand << 
				" in command : " << it.key().latin1() << endl;
			QString command2check = it.key().latin1();
			if ( command2check.startsWith(realcommand) ) {
				yzDebug() << "FOUND" << endl;
				found = true;
			}
		}

		if ( !found )
			*error = 1;
	}
}

void YZCommandPool::initExPool() {
	//XXX should be ok now ? NEW_EX_COMMAND("w",&YZExExecutor::write,true);
	NEW_EX_COMMAND("write", &YZExExecutor::write,true);
	NEW_EX_COMMAND("bnext", &YZExExecutor::buffernext,true );
	NEW_EX_COMMAND("edit", &YZExExecutor::edit,true );
	NEW_EX_COMMAND("quit", &YZExExecutor::quit,true );
}

void YZCommandPool::execExCommand(YZView *view, const QString& inputs) {
	QString result,command;
	int i=0;

	//regexp ? //FIXME
	//try to find the command we are looking for
	//first remove any number at the beginning of command
	//XXX 0 is itself a command !
	while ( inputs[ i ].isDigit() )
		i++; //go on
	
	//now take the command until another number
	while ( !inputs[ i ].isDigit() && i<inputs.length() )
		command += inputs[ i++ ];
	//end FIXME
	
	//printf( "%s %s\n", inputs.latin1(), command.latin1() );

	//try hard to find a correspondance
	QMap<QString, YZCommand>::Iterator it = globalExCommands.end();
	while ( command.length() > 0 && it == globalExCommands.end() ) {
		it = globalExCommands.find(command);
		if ( it==globalExCommands.end() ) command.truncate(command.length()-1);
	} // should not end here FIXME
	
	if ( it!=globalExCommands.end() ) { //we got one match *ouf*
		switch ( globalExCommands[ command ].obj ) {
				case EX :
					( *executor.*(globalExCommands[ command ].exFunc )) (view,inputs) ;
					break;
			default:
				break;
		}
	}
}

