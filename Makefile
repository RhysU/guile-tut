# Basic Makefile for the tortoise package.

CFLAGS = -O0 -Wall `guile-config compile`
LIBS = `guile-config link` -lm

.PHONY: clean build run

build: tortoise

clean:
	rm -f tortoise tortoise.o

run: tortoise
	./tortoise

tortoise: tortoise.o
	gcc $< -o $@ $(LIBS)

tortoise.o: tortoise.c
	gcc -c $< -o $@ $(CFLAGS)
