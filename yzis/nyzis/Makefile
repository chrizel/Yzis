



nyzis : display.o main.o lib
	g++ -o nyzis main.o display.o -lcurses ../libyzis/libyzis.a

lib:
	(cd ../libyzis; make)

main.o : ../libyzis/yz_interface.h main.cpp
	g++ -c -o main.o main.cpp -I../libyzis
display.o : ../libyzis/yz_interface.h display.cpp
	g++ -c -o display.o display.cpp -I../libyzis

clean :
	rm -f *.o nyzis display
