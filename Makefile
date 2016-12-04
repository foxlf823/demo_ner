cc=g++
	
cflags = -O0 -g3 -w \
	-I./FoxUtil \
	-msse3 -I ./mshadow-master/mshadow -DMSHADOW_USE_CUDA=0 -DMSHADOW_USE_CBLAS=1 -DMSHADOW_USE_MKL=0 \
	-I./LibN3L-master -DUSE_CUDA=0 \

libs = -lm -lopenblas -Wl,-rpath,./ \
 
all: ner

ner: main.cpp NNentity.h
	$(cc) -o ner main.cpp $(cflags) $(libs)

	




clean:
	rm -rf *.o
	rm -rf ner

