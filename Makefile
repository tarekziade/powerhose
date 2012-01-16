
build:
	cd powerhose/libhose; g++ -c libhose.cpp -I . -lpthread -lzmq -o libhose.o -Wall
	cd powerhose/libhose; ar r libhose.a libhose.o
	cp powerhose/libhose/libhose.a examples/
	cp powerhose/libhose/libhose.h examples/
	cp powerhose/libhose/libhose.o examples/
	cd examples; g++ -Wall -o square job.pb.cc square.cpp libhose.o -lprotobuf -lzmq

proto:
	protoc examples/job.proto --python_out=examples -I=examples/
	protoc examples/job.proto --cpp_out=examples -I=examples/
