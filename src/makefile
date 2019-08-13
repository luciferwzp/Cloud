server:server.cpp
	g++ -std=c++11 $^ -o $@ -lpthread -lboost_filesystem -lboost_system -lz 
test_server:test_server.cpp
		g++ -std=c++11 $^ -o $@ -lpthread
.PHONY:clean
clean:
	rm -f server
