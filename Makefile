CC = gcc
CXX = g++
CXXFLAGS = -O3 -lpthread
VPATH = src/
TARGET = cadb090

.PHONY: clean all

all: $(TARGET) Makefile

$(TARGET): Source.cpp Network.o Gate.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Network.o: Network.cpp Network.h

Gate.o: Gate.cpp Gate.h

test: $(TARGET)
	./$(TARGET) 10 7 test_cases/case8

run_case1: $(TARGET)
	./$(TARGET) 45 4 test_cases/case1

run_case2: $(TARGET)
	./$(TARGET) 43 10 test_cases/case2

run_case3: $(TARGET)
	./$(TARGET) 31 6 test_cases/case3

run_case4: $(TARGET)
	./$(TARGET) 45 6 test_cases/case4

run_case5: $(TARGET)
	./$(TARGET) 47 10 test_cases/case5

clean:
	$(RM) $(TARGET) *.o
