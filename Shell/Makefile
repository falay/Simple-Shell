CXX = g++
CFLAGS = -O3 -std=c++14 


all: Main.o SimpleShell.o PipePool.o RegularExp.o
	${CXX} $^ ${CFLAGS} -o Shell
	
%.o: %.cpp
	${CXX} $< ${CFLAGS} -c 	
	
clean:
	rm -rf *.o Shell	