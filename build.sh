#!/usr/bin/env bash

mkdir -p build

gcc main.c -o build/main -Wall -Werror $(pkg-config --cflags --libs sdl3)