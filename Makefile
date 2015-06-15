SER=server
CL=client
TIC=tictactoe
GEN=generate
TEST=test
CC=gcc

IDIR=include

CFLAGS=-std=gnu99 -Wall -I$(IDIR) -O0 -lc -g -pthread
DEPS=socket.h

all: | $(SER) $(CL) $(TIC)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TIC): $(TIC).c
	$(CC) $^ -o $@ $(CFLAGS)

$(SER): $(SER).c
	$(CC) $^ -o $@ $(CFLAGS)

$(CL): $(CL).c
	$(CC) $^ -o $@ $(CFLAGS)


.PHONY: clean
clean:
	rm $(SER) $(CL) $(TIC)
