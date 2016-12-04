cc=g++

cflags = -O0 -g3 -w -msse3 -funroll-loops -std=c++11  \
			

libs = 

all: test

test: test.cpp 
	$(cc) -o test test.cpp $(cflags) $(libs)

	




clean:
	rm -rf *.o
	rm -rf test

