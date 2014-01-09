#!/bin/sh

rm -rf ds1302.so build/ *.pyc
python setup.py build || exit 1
cp build/lib.linux-*/ds1302.so ./
