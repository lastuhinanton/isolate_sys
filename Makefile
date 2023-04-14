
all: main

main:
	gcc *.c -o isolate && sudo ./isolate sh && rm -f isolate
