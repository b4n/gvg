#!/bin/sh

mkdir -p build/auxf || exit
mkdir -p build/m4   || exit
autoreconf -vfi     || exit
