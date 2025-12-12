.PHONY:all
all: mytest myexe

mytest:test.c
	gcc -o $@ $^ -std=c99

myexe:myexe.c
	gcc -o $@ $^ -std=c99

.PHONY:clean
clean:
	rm -f mytest myexe
