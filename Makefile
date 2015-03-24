all: anagrams

anagrams: anagrams.cpp
	g++ anagrams.cpp -o anagrams -O3 -std=c++11
