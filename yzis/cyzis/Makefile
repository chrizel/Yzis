



nyzis :  main.o lib
	g++ -o cyzis main.o  -lcurses ../libyzis/libyzis.a

lib:
	(cd ../libyzis; make)

main.o : main.c
	gcc -c -o main.o main.c -I../libyzis
display.o : display.c
	gcc -c -o display.o display.c -I../libyzis

clean :
	rm -f *.o cyzis display
