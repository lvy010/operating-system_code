.PHONY:all
all: udpClient udpServer

udpClient:udpClient.cc
	g++ -o $@ $^ -pthread -std=c++11

udpServer:udpServer.cc
	g++ -o $@ $^ -std=c++11

.PHONY:clean
clean:
	rm -f udpClient udpServer
