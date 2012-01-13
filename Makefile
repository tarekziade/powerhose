
build:
	cd powerhose; g++ workers.cpp -lpthread -lzmq -o server -Wall
