all: clean mytar

mytar: mytar.o tarcreate.o tarextract2.o
	gcc -ggdb -o mytar mytar.o tarcreate.o tarextract2.o

mytar.o: mytar.c tarheader.h tarutil.h
	gcc -ggdb -c mytar.c

tarcreate.o: tarcreate.c tarheader.h tarutil.h
	gcc -ggdb -c tarcreate.c

tarextract2.o: tarextract2.c tarheader.h tarutil.h
	gcc -ggdb -c tarextract2.c

clean:
	rm -f mytar
	rm -f *.o
