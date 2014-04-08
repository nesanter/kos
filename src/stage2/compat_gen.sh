#!/bin/bash

echo "#ifdef __MODE_KERNEL32
#error \"THIS HEADER IS NOT STAGE1 COMPATIBLE!\"
#endif

#ifndef _KERNEL32_COMPAT_H
#define _KERNEL32_COMPAT_H

" > src/$M/kernel32_compat.h

#search path is hardcoded, oh well...
for F in $(find src/stage1/ -name *\.h) ; do
    awk 'BEGIN {emit=0;} /^\/\/##KERNEL32_COMPAT_END$/ {emit=0;} {if (emit == 1) { print $0; }} /^\/\/##KERNEL32_COMPAT_START$/ {emit=1;}' $F >> src/$M/kernel32_compat.h
done

echo "#endif" >> src/$M/kernel32_compat.h
