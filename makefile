build:
	@mkdir -p build
	@cd build && cmake -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ .. && make

run: build
	WINEDEBUG=-all wine explorer /desktop=Lab4,800x600 build/src/receiver.exe

test: build
	@WINEDEBUG=-all wine build/tests/lab4_tests.exe

clean:
	@rm -rf build
