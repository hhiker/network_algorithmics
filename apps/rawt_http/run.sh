#!/bin/bash

LWIPLIBDIR=../../lwip_contrib/contrib/ports/unix/proj/lib
IOELIBDIR=../ioengine/lib

export LD_LIBRARY_PATH=$LWIPLIBDIR:$IOELIBDIR:$LD_LIBRARY_PATH

perf record -e cycles:u -g ./rawt_http

exit 0
