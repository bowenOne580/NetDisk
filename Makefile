# Build and run from project root.
# Usage:
#   make run        # build in source/, move binary to root, then run
#   make run PORT=2055
#   make            # build only
#   make only-build # build only (explicit target)
#   make clean      # remove built files

CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra

ROOT_DIR := .
SRC_DIR := source
TARGET := server
PORT ?= 8000

SRCS := $(wildcard $(SRC_DIR)/*.cpp)

.PHONY: all build only-build run clean

all: build

build: $(ROOT_DIR)/$(TARGET)

# Compile inside source/, then move binary to root/
$(ROOT_DIR)/$(TARGET): $(SRCS)
	cd $(SRC_DIR) && $(CXX) $(CXXFLAGS) -o $(TARGET) *.cpp
	mv -f $(SRC_DIR)/$(TARGET) $(ROOT_DIR)/$(TARGET)

run: build
	./$(TARGET) port=$(PORT)

clean:
	rm -f $(ROOT_DIR)/$(TARGET)
	rm -f $(SRC_DIR)/$(TARGET)
