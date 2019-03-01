all: clean mytar

mytar: mytar.o tarcreate.o tarextract.o tarlist.o
	gcc -ggdb -o mytar mytar.o tarcreate.o tarextract.o tarlist.o

mytar.o: mytar.c tarheader.h tarutil.h
	gcc -ggdb -c mytar.c

tarcreate.o: tarcreate.c tarheader.h tarutil.h
	gcc -ggdb -c tarcreate.c

tarextract.o: tarextract.c tarheader.h tarutil.h
	gcc -ggdb -c tarextract.c

tarlist.o: tarlist.c tarheader.h tarutil.h
	gcc -ggdb -c tarlist.c

clean:
	rm -f mytar
	rm -f *.o
