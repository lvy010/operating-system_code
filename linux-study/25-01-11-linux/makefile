libmymath.a:my_add.o my_sub.o
	ar -rc $@ $^

my_add.o:my_add.c
	gcc -c my_add.c -o my_add.o

my_sub.o:my_sub.c
	gcc -c my_sub.c -o my_sub.o

.PHONY:output
output:
	mkdir -p mylib/include
	mkdir -p mylib/lib
	cp -f *.h mylib/include
	cp -f *.a mylib/lib


.PHONY:clean
clean:
	rm -f my_add.o my_sub.o libmymath
