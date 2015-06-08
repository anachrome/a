.PHONY: all install clean

all: anagram

install: anagram
	mkdir -p /usr/local/bin/ /usr/local/share/man/man1/
	cp anagram /usr/local/bin/
	gzip -fk anagram.1
	cp anagram.1.gz /usr/local/share/man/man1/

uninstall:
	rm /usr/local/bin/anagram
	rm /usr/local/share/man/man1/anagram.1.gz

clean:
	rm -f anagram
	rm -f anagram.1.gz

anagram: anagram.cpp
	g++ anagram.cpp -o anagram -O3 -std=c++11
