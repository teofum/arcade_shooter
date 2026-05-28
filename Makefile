all: build
	cmake --build build

build: CMakeLists.txt
	cmake -B build -DCMAKE_BUILD_TYPE="Debug"

clean:
	rm -rf build bin

run: all
	bin/arcade_shooter

.PHONY: clean run
