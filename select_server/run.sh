#!/bin/bash

LWIPLIBDIR=../contrib/ports/unix/proj/lib
IOELIBDIR=../ioengine/lib

export LD_LIBRARY_PATH=$LWIPLIBDIR:$IOELIBDIR:$LD_LIBRARY_PATH

echo
./select_server_lwip
echo

exit 0
