CC = gcc
CXX = g++
CXXFLAGS = -O3 -lpthread --std=c++11 -w
VPATH = src/
TARGET = cadb090

.PHONY: clean all test run_case1 run_case2 run_case3 run_case4 run_case5

all: $(TARGET)

$(TARGET): Source.cpp Network.o Gate.o
	$(CXX) $(CXXFLAGS) $^ -o $@

Network.o: Network.cpp Network.h

Gate.o: Gate.cpp Gate.h

test: $(TARGET)
	bash -c "time ./$(TARGET) 10 7 test_cases/case8 ans/case8.ans"

run_case1: $(TARGET)
	bash -c "time ./$(TARGET) 45 4 test_cases/case1 ans/case1.ans"

run_case2: $(TARGET)
	bash -c "time ./$(TARGET) 43 10 test_cases/case2 ans/case2.ans"

run_case3: $(TARGET)
	bash -c "time ./$(TARGET) 31 6 test_cases/case3 ans/case3.ans"

run_case4: $(TARGET)
	bash -c "time ./$(TARGET) 45 6 test_cases/case4 ans/case4.ans"

run_case5: $(TARGET)
	bash -c "time ./$(TARGET) 47 10 test_cases/case5 ans/case5.ans"

clean:
	$(RM) $(TARGET) *.o
