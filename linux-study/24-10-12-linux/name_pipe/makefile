.PHONY:all
all:send receive

send:send.cc
	g++ -o $@ $^ -std=c++11

receive:receive.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f send receive