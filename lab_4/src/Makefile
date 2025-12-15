CC = gcc
CFLAGS = -fPIC

all: libmath_impl1.so libmath_impl2.so program1 program2

libmath_impl1.so: math_impl1.c math_functions.h
	$(CC) $(CFLAGS) -shared -o $@ $<

libmath_impl2.so: math_impl2.c math_functions.h
	$(CC) $(CFLAGS) -shared -o $@ $<

program1: program1.c
	$(CC) -o $@ $< -L. -lmath_impl1 -Wl,-rpath,.

program2: program2.c
	$(CC) -o $@ $< -ldl

clean:
	rm -f *.so program1 program2