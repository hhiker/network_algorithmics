
CFLAGS   = -g -Wall -O2

INCLUDES = -I ../contrib/ports/unix/include \
           -I ../lwip/src/include \
           -I ../lwip/src/include/ipv4 \
           -I ../contrib/ports/unix/proj/lib

LIBS     = -L ../contrib/ports/unix/proj/lib -llwip \
           -lpthread


all: rawt_http


rawt_http: rawt_http.c
	(cd ../contrib/ports/unix/proj/lib; $(MAKE))
	gcc $(CFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)


clean:
	(cd ../contrib/ports/unix/proj/lib; $(MAKE) clean)
	rm -f rawt_http *~
