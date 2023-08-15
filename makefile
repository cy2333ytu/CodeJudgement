CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

simpleoj: SimpleOJ.o main.o
	$(CXX) $(CXXFLAGS) -o simpleoj SimpleOJ.o main.o -lrt

SimpleOJ.o: SimpleOJ.cpp SimpleOJ.h
	$(CXX) $(CXXFLAGS) -c SimpleOJ.cpp

main.o: main.cpp SimpleOJ.h
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f *.o simpleoj
