#
#
#

LJPG = /usr/local/opt/libjpeg-turbo/lib/libturbojpeg.a
LPNG = -lpng
INC = -I/usr/local/opt/jpeg-turbo/include/
OP  = -g
CC  = gcc $(OP) $(INC)
CO  = $(CC) -c

main: main.c libopenip.a
	$(CC) main.c libopenip.a -o main $(LPNG) $(LJPG) -lfftw3

libopenip.a: ffimg.o imgio.o
	ar rcs libopenip.a ffimg.o imgio.o

ffimg.o: ffimg.c 
	$(CO) ffimg.c -o ffimg.o

imgio.o: imgio.c imgio.h openip_globals.h
	$(CO) imgio.c -o imgio.o

clean:
	rm -f  *.o
	rm -f  *.a
	rm -rf *.dSYM
	rm -f  main
