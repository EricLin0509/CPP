CC=g++
LDFLAGS=`pkg-config --static --libs glfw3` -lvulkan -ltinyobjloader

all: shader build

shader:
	sh compile.sh

build: *.cpp
	$(CC) $(LDFLAGS) -o main $^