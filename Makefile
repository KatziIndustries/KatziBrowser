TARGET = katzi
GIT_HASH := $(shell git rev-parse --short HEAD)

CC = gcc
CCFLAGS = -Wall -Wextra -O2 -std=c++17

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

.PHONY: all run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(VKATZI_DIR)/$(VKATZI_O) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
