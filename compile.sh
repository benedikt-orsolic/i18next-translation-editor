#!/bin/bash

if [ ! -d "./target" ]; then
	mkdir target;
fi

gcc $( pkg-config --cflags gtk4 ) -o target/main main.c $( pkg-config --libs gtk4 )
