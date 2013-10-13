#!/bin/bash

LWIPLIBDIR=../../lwip_contrib/contrib/ports/unix/proj/lib
IOELIBDIR=../../lwip_contrib/ioengine/lib

export LD_LIBRARY_PATH=$LWIPLIBDIR:$IOELIBDIR:$LD_LIBRARY_PATH

echo
./select_server_lwip
echo

exit 0
