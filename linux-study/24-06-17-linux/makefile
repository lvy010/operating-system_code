LD=-DMYSELF

.PHONY:all
all:calclient calserver

calclient:CalClient.cc
	g++ -o $@ $^ -std=c++11 -ljsoncpp ${LD}

calserver:CalServer.cc
	g++ -o $@ $^ -std=c++11 -ljsoncpp ${LD}

.PHONY:clean
clean:
	rm -f calserver calclient
