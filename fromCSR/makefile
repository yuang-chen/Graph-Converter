CC      = g++
CPPFLAGS = -O3 -c -std=c++11 -fopenmp -mavx -w -march=native -D_GLIBCXX_PARALLEL 
#CPPFLAGS = -g -O0 -Wall -c -std=c++11 -fopenmp -mavx -w  
LDFLAGS = -fopenmp -m64 -lpthread   -lboost_system#-fopenmp-simd 

SOURCES = main.cpp 
OBJECTS = $(SOURCES:.cpp=.o)

all: $(SOURCES) convert

convert : $(OBJECTS)  

	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o : 
	$(CC) $(CPPFLAGS) $(OMP) $< -o $@

clean:
	rm -f *.o pr dump*

