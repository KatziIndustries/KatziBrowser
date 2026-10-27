
CC = gcc

all:
	make kb-linux

kb-linux:
	mkdir -p build
	make -C linux
	make -C webview

run:
	cd build && ../linux/bin/Release/net10.0/publish/katzi