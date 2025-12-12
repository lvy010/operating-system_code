.PHONY:all
all:clint service

clint:clint.cc
	g++ -o $@ $^ -std=c++11

service:service.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f clint service