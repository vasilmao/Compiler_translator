options = --std=c++17

translator: main.o parser.o file_reader.o DynamicArray.o assembly.o
	g++ main.o parser.o file_reader.o DynamicArray.o assembly.o

assembly.o: parser.h assembly.h assembly.cpp
	g++ -g -c assembly.cpp $(options) $(debug)

parser.o: parser.cpp parser.h
	g++ -g -c parser.cpp $(options) $(debug)

main.o: main.cpp
	g++ -g -c main.cpp $(options) $(debug)

file_reader.o: file_reader.cpp
	g++ -g -c file_reader.cpp $(options) $(debug)

DynamicArray.o: DynamicArray.cpp
	g++ -g -c DynamicArray.cpp $(options) $(debug)
