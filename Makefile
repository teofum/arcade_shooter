all: build
	cmake --build build

build:
	cmake -B build

clean:
	rm -rf build bin

run: all
	bin/arcade_shooter

.PHONY: clean run
