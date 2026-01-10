BUILD_DIR := build
BUILD_TYPE ?= Debug

.PHONY: all configure build clean

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -G Ninja -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

build: configure
	cmake --build $(BUILD_DIR)

clean:
	cmake --build build --target clean