CC = gcc
CXX = g++
CXXFLAGS = -g -Wall
VPATH = src/

cadb090: Source.cpp Network.h
	$(CXX) $(CXXFLAGS) $< -o $@
