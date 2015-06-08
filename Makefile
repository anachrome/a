.PHONY: all install clean

all: anagram

install: anagram
	mkdir -p /usr/local/bin/ /usr/local/share/man/man1/
	cp anagram /usr/local/bin/
	ln -sf /usr/local/bin/anagram /usr/local/bin/a
	gzip -fk anagram.1 a.1
	mv anagram.1.gz a.1.gz /usr/local/share/man/man1/

uninstall:
	rm /usr/local/bin/anagram
	rm /usr/local/bin/a
	rm /usr/local/share/man/man1/anagram.1.gz
	rm /usr/local/share/man/man1/a.1.gz

clean:
	rm anagram

anagram: anagram.cpp
	g++ anagram.cpp -o anagram -O3 -std=c++11
