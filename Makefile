all: anagram

anagram: anagram.cpp
	g++ anagram.cpp -o anagram -O3 -std=c++11 -g
