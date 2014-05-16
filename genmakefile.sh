#!/bin/bash

sources=$(find -name "*.c");
LIBDIR="../lib";
SRCMAKEFILE="Makefile.src";
MAKEFILE="Makefile";
COMPILESTR='$(CC) -c -o $@ $< $(CFLAGS) $(INCS)';

rm -f $MAKEFILE 2>/dev/null;
cp $SRCMAKEFILE $MAKEFILE | exit 1;

for src in $sources
do
    name=$(expr $src : '.*/\(.*\)\.' \| $src);
    file=$(expr $src : '[./]*\(.*\)' \| $src);
    echo -ne "\n$name.o: $LIBDIR/$file\n\t$COMPILESTR\n" >> $MAKEFILE;
done
