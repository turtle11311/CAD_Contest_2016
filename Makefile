CC = gcc
CXX = g++
CXXFLAGS = -O3 -Wall 
VPATH = src/
TARGET = cadb090

$(TARGET): Source.cpp Network.h
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(TARGET)
	./$(TARGET) 10 7 < test_cases/case8

run_case1: $(TARGET)
	./$(TARGET) 45 4 < test_cases/case1

run_case2: $(TARGET)
	./$(TARGET) 43 10 < test_cases/case2

run_case3: $(TARGET)
	./$(TARGET) 31 6 < test_cases/case3

run_case4: $(TARGET)
	./$(TARGET) 45 6 < test_cases/case3

run_case5: $(TARGET)
	./$(TARGET) 47 10 < test_cases/case3
