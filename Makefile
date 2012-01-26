LIBFILES = libhose.cpp Worker.cpp Controller.cpp util.cpp PowerHose.cpp 
LIBOFILES = $(LIBFILES:%.cpp=%.o)
DIR = $(CURDIR)/powerhose/libhose
INCLUDEDIR = -I$(DIR) -I/opt/local/include
LIBDIR = -L$(DIR) -L/opt/local/lib
OPTS = -lpthread -lzmq -g -Wall -Wextra -lprotobuf

.phony: all build build-example proto


all:
	build

build:
	cd $(DIR); rm -f libhose.a; rm -f *.o
	cd $(DIR); g++ $(INCLUDEDIR) $(LIBDIR) -c $(LIBFILES) $(OPTS)
	cd $(DIR); ar cq libhose.a $(LIBOFILES)

build-example: proto
	cd examples; rm -f square
	cd examples; g++ -o square job.pb.cc square.cpp -lhose $(OPTS) $(INCLUDEDIR) $(LIBDIR)

proto:
	protoc examples/job.proto --python_out=examples -I=examples/
	protoc examples/job.proto --cpp_out=examples -I=examples/
