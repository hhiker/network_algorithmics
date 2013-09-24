
CFLAGS   = -g -Wall -O2


INCLUDES = -I ../contrib/ports/unix/include \
           -I ../lwip/src/include \
           -I ../lwip/src/include/ipv4 \
           -I ../contrib/ports/unix/proj/lib \
           -I ../lwip/src/include/posix

LIBS     = -L ../contrib/ports/unix/proj/lib -llwip \
           -lpthread


all: select_server select_server_lwip


select_server: select_server.c
	gcc $(CFLAGS) $^ -o $@


select_server_lwip: select_server.c
	(cd ../contrib/ports/unix/proj/lib; $(MAKE))
	gcc $(CFLAGS) -DLWIP $(INCLUDES) $^ -o $@ $(LIBS)


clean:
	(cd ../contrib/ports/unix/proj/lib; $(MAKE) clean)
	rm -f select_server select_server_lwip *~
