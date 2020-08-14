CC=gcc
BINARY=client server

LIB=-pthread
INLCUDE=

%: %.c
	$(CC) -o $@ $< $(LIB)

all: $(BINARY)

clean:
	rm -f *.o $(BINARY)
