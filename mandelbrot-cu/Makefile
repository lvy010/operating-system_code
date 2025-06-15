mandelbrot: mandelbrot.cu
	nvcc -g -O2 $< -o $@
	cuobjdump --dump-ptx mandelbrot > mandelbrot.ptx

run:
	nsys profile --stats=true ./mandelbrot

clean:
	rm -f mandelbrot

