
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <libyzis/debug.h>
#include <libyzis/color.h>

#define qRgb YzqRgb

#define check_string( name, s1, s2 ) \
	yzDebug(__FILE__) << name << "( " << s1 << " =? " << s2 << " ) " << (QString(s1) == QString(s2)? "OK" : "KO") << endl
#define check_int( name, i1, i2 ) \
	yzDebug(__FILE__) << name << "( " << i1 << " =? " << i2 << " ) " << (i1 == i2 ? "OK" : "KO") << endl

void test_color( const char* name, int red, int green, int blue, const char* hex, const char* hex_short ) {
	YZColor c;
	QRgb rgb = qRgb( red, green, blue );

	printf( "test_color: %s, %s, %s, (%d,%d,%d) = %u\n", name, hex, hex_short, red, green, blue, rgb );

#define CHECK_VALIDITY \
	check_string( "name", hex, c.name() ); \
	check_int( "rgb", rgb, c.rgb() ); \
	check_int( "red", red, c.red() ); \
	check_int( "green", green, c.green() ); \
	check_int( "blue", blue, c.blue() );
	
	printf( "setNamedColor(%s)\n", name );
	c.setNamedColor( name );
	CHECK_VALIDITY;
	printf( "setNamedColor(%s)\n", hex );
	c.setNamedColor( hex );
	CHECK_VALIDITY;
	if ( hex_short != NULL ) {
		printf( "setNamedColor(%s)\n", hex_short );
		c.setNamedColor( hex_short );
		CHECK_VALIDITY;
	}
	printf( "setRgb(%u)\n", rgb );
	c.setRgb( rgb );
	CHECK_VALIDITY;
}


int main() {
	YZDebugBackend::instance()->setDebugOutput( stdout );

	//test_color( "white", 255, 255, 255, "#ffffff", "#fff" );
	//test_color( "red", 255, 0, 0, "#ff0000", "#f00" );
	test_color( "lime", 0, 255, 0, "#00ff00", "#0f0" );
	//test_color( "blue", 0, 0, 255, "#0000ff", "#00f" );
	test_color( "#ffcccc", 255, 204, 204, "#ffcccc", "#fcc" );

	return EXIT_SUCCESS;
}

