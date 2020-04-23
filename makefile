build_server: cls
	g++ ./src/server.cpp -o server.exe

build_client: cls
	g++ ./src/client.cpp -o client.exe

run_server:cls
	./server.exe

run_client: cls
	./client.exe

cls:
	clear