#!/bin/bash

LWIPLIBDIR=../contrib/ports/unix/proj/lib
IOELIBDIR=../ioengine/lib

export LD_LIBRARY_PATH=$LWIPLIBDIR:$IOELIBDIR:$LD_LIBRARY_PATH


./rawt_http


exit 0
