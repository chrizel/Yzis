
#include "testColor.h"

#include <libyzis/debug.h>
#include <libyzis/color.h>

void TestColor::testColor1()
{
    subTestColor( "white", 255, 255, 255, "#ffffff", "#fff" );
    subTestColor( "red", 255, 0, 0, "#ff0000", "#f00" );
    subTestColor( "lime", 0, 255, 0, "#00ff00", "#0f0" );
    subTestColor( "blue", 0, 0, 255, "#0000ff", "#00f" );
    subTestColor("#ffcccc", 255, 204, 204, "#ffcccc", "#fcc" );
}


void TestColor::subTestColor( const QString& name,
                              int red, int green, int blue,
                              const QString& hex, const QString& hex_short )
{
    YColor c;
    QRgb rgb = YzqRgb( red, green, blue );

#define CHECK_VALIDITY() \
    QCOMPARE( c.name(), hex ); \
    QCOMPARE( c.rgb(), rgb ); \
    QCOMPARE( c.red(), red ); \
    QCOMPARE( c.green(), green ); \
    QCOMPARE( c.blue(), blue ); \

    c.setNamedColor( name );
    CHECK_VALIDITY()

    c.setNamedColor( hex );
    CHECK_VALIDITY()

    if ( hex_short != NULL ) {
        c.setNamedColor( hex_short );
        CHECK_VALIDITY();
    }

    c.setRgb( rgb );
    CHECK_VALIDITY()
}

