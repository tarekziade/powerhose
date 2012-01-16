
build:
	cd powerhose/libhose; g++ -c libhose.cpp -I . -lpthread -lzmq -o libhose.o -Wall
	cd powerhose/libhose; ar r libhose.a libhose.o
	cp powerhose/libhose/libhose.a examples/
	cp powerhose/libhose/libhose.h examples/
	cd examples; g++ -o square job.pb.cc square.cpp -L . -lprotobuf 
