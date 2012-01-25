
LIBFILES = libhose.cpp Worker.cpp Controller.cpp util.cpp PowerHose.cpp 
LIBOFILES = $(LIBFILES:%.cpp=%.o)
LIBDIR = $(CURDIR)/powerhose/libhose

build:
	cd powerhose/libhose; rm -f libhose.a; rm -f *.o
	cd $(LIBDIR); g++ -g -c $(LIBFILES) -lpthread -lzmq -Wall -I . -Wextra -pedantic
	cd $(LIBDIR); ar cq libhose.a $(LIBOFILES)

build-example:
	cd examples; rm -f square
	cd examples; g++ -g -Wall -o square job.pb.cc square.cpp -lhose -lprotobuf -lzmq -Wextra -pedantic -I $(LIBDIR) -L $(LIBDIR)

proto:
	protoc examples/job.proto --python_out=examples -I=examples/
	protoc examples/job.proto --cpp_out=examples -I=examples/
