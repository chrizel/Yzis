

all : nyzis


nyzis : lib display.o main.o nyz_view.o
	g++ -o nyzis main.o display.o nyz_view.o  -lcurses ../libyzis/libyzis.a

lib:
	(cd ../libyzis; make)

main.o : main.cpp
	g++ -c -o main.o main.cpp -I../libyzis
display.o : display.cpp
	g++ -c -o display.o display.cpp -I../libyzis
nyz_view.o : display.cpp
	g++ -c -o nyz_view.o nyz_view.cpp  -I../libyzis
clean :
	rm -f *.o nyzis display
re : clean all
