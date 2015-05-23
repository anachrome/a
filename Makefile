all: anagram

anagram: anagram.cpp
	g++ anagram.cpp -o anagram -O3 -std=c++11 -g
	# temporarily until gcc 5 is in the arch repos
	#clang++ anagram.cpp -o anagram -O3 -std=c++11 -stdlib=libc++ -lc++abi -g
