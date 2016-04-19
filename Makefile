.PHONY: all tmp clean

#CC=icc -std=c++11 -fopenmp

#CC=g++ -std=c++11 -fopenmp -Wall -march=native -g
CC=g++ -std=c++11 -fopenmp -Wall -Ofast -march=native

all: tmp

tmp: include/* src/*
	rm -f tmp
	./sanitize.sh $^
	-cd build && $(CC) *.cpp -o ../tmp

clean:
	rm -f build/* tmp
