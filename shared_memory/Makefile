CC=clang
CFLAGS=-std=c11 -c

-all: writer.o reader.o
	$(CC) -o writer writer.o && $(CC) -o reader reader.o && rm reader.o writer.o instr.h.gch 

writer.o:
	$(CC) $(CFLAGS) writer.c instr.h

reader.o:
	$(CC) $(CFLAGS) reader.c instr.h