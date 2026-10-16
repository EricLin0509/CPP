CC=g++
LDFLAGS=`pkg-config --static --libs glfw3` -lvulkan

all: shader build

shader:
	sh compile.sh

build: *.cpp
	$(CC) $(LDFLAGS) -o main $^