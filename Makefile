CC = gcc
CXX = g++
CXXFLAGS = -g -Wall
VPATH = src/

cadb090: Source.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@
