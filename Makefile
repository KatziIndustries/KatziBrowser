
CC = gcc

all:
	make kb-linux
	make test

kb-linux:
	mkdir -p build
	make -C linux
	make -C webview

test:
	cd build && ./katzi