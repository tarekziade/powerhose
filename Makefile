
LIBFILES = libhose.cpp Worker.cpp Controller.cpp util.cpp 
LIBOFILES = $(LIBFILES:%.cpp=%.o)

build:
	cd powerhose/libhose; rm -f libhose.a; rm -f *.o
	cd examples; rm -f libhose.*
	cd powerhose/libhose; g++ -g -c $(LIBFILES) -lpthread -lzmq -Wall -I . -Wextra
	cd powerhose/libhose; ar cq libhose.a $(LIBOFILES)
	cp powerhose/libhose/libhose.a examples/
	cp powerhose/libhose/libhose.h examples/
	cd examples; rm -f square
	cd examples; g++ -g -Wall -o square job.pb.cc square.cpp libhose.a -lprotobuf -lzmq -Wextra

proto:
	protoc examples/job.proto --python_out=examples -I=examples/
	protoc examples/job.proto --cpp_out=examples -I=examples/
