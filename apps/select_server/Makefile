
CFLAGS   = -g -Wall -O2


INCLUDES = -I ../../lwip_contrib/contrib/ports/unix/include \
           -I ../../lwip_contrib/lwip/src/include \
           -I ../../lwip_contrib/lwip/src/include/ipv4 \
           -I ../../lwip_contrib/contrib/ports/unix/proj/lib \
           -I ../../lwip_contrib/lwip/src/include/posix

LIBS     = -L ../../lwip_contrib/contrib/ports/unix/proj/lib -llwip \
           -lpthread


all: select_server select_server_lwip


select_server: select_server.c
	gcc $(CFLAGS) $^ -o $@


select_server_lwip: select_server.c
	# (cd ../../lwip_contrib/contrib/ports/unix/proj/lib; $(MAKE))
	gcc $(CFLAGS) -DLWIP $(INCLUDES) $^ -o $@ $(LIBS)


clean:
	# (cd ../../lwip_contrib/contrib/ports/unix/proj/lib; $(MAKE) clean)
	rm -f select_server select_server_lwip *~
