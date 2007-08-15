
#ifndef TEST_COLOR_H
#define TEST_COLOR_H

#include <QtTest/QtTest>

class TestColor : public QObject
{
    Q_OBJECT

private slots:
    void testColor1();

protected:
    void subTestColor( const QString& name, int red, int green, int blue,
                       const QString& hex, const QString& hex_short );
};

#endif // TEST_COLOR_H

