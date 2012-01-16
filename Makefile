
build:
	cd powerhose/libhose; g++ -c libhose.cpp -I . -lpthread -lzmq -o libhose.o -Wall
	cd powerhose/libhose; ar r libhose.a libhose.o
