
#include <stdlib.h>
#include <stdio.h>

#include <libyzis/debug.h>
#include <libyzis/drawbuffer.h>
#include <libyzis/color.h>

void paintCell( const YZViewCell& cell, void* vfd ) {
	int* fd;
	fd = (int*) vfd;
	yzDebug() << "paintCell[" << *fd << "]: " << cell.fg.name() << " \"" << cell.c << "\"" << endl;
}

void test_drawbuffer() {
	int painter[1];

	*painter = 1;

	YZDrawBuffer buf( paintCell );

	buf.setCallbackArgument( painter );

	buf.setColor( YZColor( "red" ) );
	buf.push( "c" );
	buf.push( "l" );
	buf.push( "a" );
	buf.push( "s" );
	buf.push( "s" );
	buf.setColor( YZColor( "white" ) );
	buf.push( " " );
	buf.push( "t" );
	buf.push( "e" );
	buf.push( "s" );
	buf.push( "t" );
	buf.linebreak();
	buf.push( " " );
	buf.push( " " );
	buf.push( " " );
	buf.setColor( YZColor( "yellow" ) );
	buf.push( "d" );
	buf.push( "e" );
	buf.push( "f" );
	buf.flush();

	
}

int main() {
	YZDebugBackend::instance()->setDebugOutput( stdout );
	test_drawbuffer();
	return EXIT_SUCCESS;
}

