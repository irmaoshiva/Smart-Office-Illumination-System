FLAGS= -O2 -Ofast -Wall -std=c++11 -lboost_system -pthread
PGPIO= -lpigpio -lpthread -lrt

default: server

all: server client

server: server.cpp async_tcp_server.cpp luminaire.cpp deskDB.cpp
	g++ $(FLAGS) $(PGPIO) server.cpp async_tcp_server.cpp luminaire.cpp deskDB.cpp -o server

client: asynctcpclient1.cpp
	g++ $(FLAGS) asynctcpclient1.cpp -o client

clean:
	rm -f *.o server

clean_all:
	rm -f *.o server client