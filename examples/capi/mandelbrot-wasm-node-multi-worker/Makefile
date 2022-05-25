
all: mandelbrot.wat

mandelbrot.o: mandelbrot.c
	clang-12 -O3 --no-standard-libraries --target=wasm32 -c -o $@ $^

mandelbrot.wasm: mandelbrot.o
	wasm-ld --no-entry $^ -o $@ --import-memory --export-all --shared-memory --features=mutable-globals,atomics,bulk-memory

mandelbrot.wat: mandelbrot.wasm
	wasm2wat --enable-all mandelbrot.wasm -o mandelbrot.wat

clean:
	rm -f mandelbrot.wasm mandelbrot.o output.png mandelbrot.wat

test:
	rm -f output.png
	node --experimental-wasm-threads --experimental-wasm-bulk-memory main.js