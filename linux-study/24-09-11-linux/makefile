.PHONY:all
all:tcpclient tcpserver

tcpclient:tcpClient.cc
	g++ -o $@ $^ -std=c++11

tcpserver:tcpServer.cc
	g++ -o $@ $^ -std=c++11 -lpthread

.PHONY:clean
clean:
	rm -f tcpserver tcpclient
