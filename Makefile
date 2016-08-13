CC = gcc
CXX = g++
CXXFLAGS = -O3 -Wall 
VPATH = src/

cadb090: Source.cpp Network.h
	$(CXX) $(CXXFLAGS) $< -o $@
test:
	./cadb090 10 7 < test_cases/case8
