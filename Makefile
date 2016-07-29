CC = gcc
CXX = g++
CXXFLAGS = -g -O3 -Wall
VPATH = src/

cadb090: Source.cpp Network.h
	$(CXX) $(CXXFLAGS) $< -o $@
