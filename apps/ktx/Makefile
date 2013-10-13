
CFLAGS   = -g -Wall -O2


all: ktx


ktx: ktx.c
	gcc $(CFLAGS) $^ -o $@ -lpthread


clean:
	rm -f ktx *~
