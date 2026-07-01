TARGET = katzi
GIT_HASH := $(shell git rev-parse --short HEAD)

CC = gcc
CCFLAGS = -Wall -Wextra -O2 -std=c++17

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@
