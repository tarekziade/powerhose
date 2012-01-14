
build:
	cd powerhose; g++ workers.cpp job.pb.cc -lpthread -lzmq -lprotobuf -lstdc++ -o server -Wall
