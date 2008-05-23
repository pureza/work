CC = g++
CFLAGS = -DX86 -DLINUX -Wno-non-template-friend -D_REENTRANT -DRSTM -ggdb -DTLS_PTHREAD -DALLOCATION_GCHEAP -DDEFAULT_CM=Polka -DTATAS -DPRIVATIZATION_TFENCE -Wall
LDFLAGS = -lpthread ../../stm/obj/rstm/libstm.a
INCLUDE_PATH = -I../../ -I../../stm
OBJ = main.o
EXE = main

all: main

main: main.o
	$(CC) $< -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDE_PATH) -c $< -o $@

clean:
	rm -rf *o $(EXE)

