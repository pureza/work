CC = icc
CFLAGS = -D_REENTRANT -Wall -Qtm_enabled
LDFLAGS = -lpthread -lstdc++
INCLUDE_PATH =
OBJ = main.o
EXE = main

all: main

main: main.o
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c $< -o $@

clean:
	rm -rf *o $(EXE)

