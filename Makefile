#
#
#

LJPG = /usr/local/opt/libjpeg-turbo/lib/libturbojpeg.a
LPNG = -lpng
INC = -I/usr/local/opt/jpeg-turbo/include/
OP  = -g
CC  = gcc $(OP) $(INC)
CO  = $(CC) -c

main: main.c imgio.o ffimg.o
	$(CC) ffimg.o main.c imgio.o -o main $(LPNG) $(LJPG) -lfftw3

ffimg.o: ffimg.c 
	$(CO) ffimg.c -o ffimg.o

imgio.o: imgio.c imgio.h openip_globals.h
	$(CO) imgio.c -o imgio.o

clean:
	rm -f  *.o
	rm -rf *.dSYM
	rm -f  main
