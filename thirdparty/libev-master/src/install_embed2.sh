#!/bin/sh
path=$(pwd)
path=${path%/*}
path=${path%/*}
path=${path%/*}
path=${path%/*}
incpath=$path/inc/libev
libpath=$path/lib/libev

echo $incpath
echo $libpath

./configure --libdir=$libpath --includedir=$incpath 
&& sed -i 's/^install_embed:.*$//' Makefile
&& echo "install_embed: install-libLTLIBRARIES" >> Makefile
&& make 
&& make install 
