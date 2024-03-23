CFLAGS=-std=c11 -g -fno-common

chibicc: chibicc.o
	$(CC) -o chibicc chibicc.o $(LDFLAGS)

test: chibicc
	./test.sh

clean:
	rm -f chibicc *.o *~ tmp*

.PHONY: test clean