
CFLAGS   = -g -Wall -O2

INCLUDES = -I ../../lwip_contrib/contrib/ports/unix/include \
           -I ../../lwip_contrib/lwip/src/include \
           -I ../../lwip_contrib/lwip/src/include/ipv4 \
           -I ../../lwip_contrib/contrib/ports/unix/proj/lib

LIBS     = -L ../../lwip_contrib/contrib/ports/unix/proj/lib -llwip \
           -lpthread


all: rawt_http


rawt_http: rawt_http.c
	(cd ../../lwip_contrib/contrib/ports/unix/proj/lib; $(MAKE))
	gcc $(CFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)


clean:
	(cd ../../lwip_contrib/contrib/ports/unix/proj/lib; $(MAKE) clean)
	rm -rf rawt_http *~ annotated oprofile_out.log  output.png  perf.data  perf.data.old
