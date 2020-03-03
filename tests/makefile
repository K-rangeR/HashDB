# Builds the unit tests
CC = gcc
CFLAGS = -Wall

# List of dependencies for compiling programs that 
# use the check testing framework
CHECKDEPENS = -lcheck -lm -lpthread

SRCDIR = ../src

# Build the unit tests for memtable.c
check_memtable: check_memtable.o memtable.o
	$(CC) check_memtable.o memtable.o $(CHECKDEPENS) -o check_memtable

check_memtable.o: check_memtable.c
	$(CC) -c check_memtable.c -o check_memtable.o

# Build the unit tests for segment.c
check_segment: check_segment.o segment.o memtable.o
	$(CC) check_segment.o segment.o  memtable.o $(CHECKDEPENS) -o check_segment

check_segment.o: check_segment.c
	$(CC) -c check_segment.c -o check_segment.o

# Build the unit tests for hashDB.c
check_hashDB: check_hashDB.o hashDB.o segment.o memtable.o
	$(CC) check_hashDB.o hashDB.o segment.o memtable.o $(CHECKDEPENS) -o check_hashDB

check_hashDB.o: check_hashDB.c
	$(CC) -c check_hashDB.c -o check_hashDB.o

# Build program to create testing data
write_perm: write_perm.o segment.o memtable.o
	$(CC) write_perm.o segment.o memtable.o -o write_perm

write_perm.o: write_perm.c data.h
	$(CC) -c write_perm.c -o write_perm.o

memtable.o: $(SRCDIR)/memtable.c $(SRCDIR)/memtable.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/memtable.c -o memtable.o

segment.o: $(SRCDIR)/segment.c $(SRCDIR)/segment.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/segment.c -o segment.o

hashDB.o: $(SRCDIR)/hashDB.c $(SRCDIR)/hashDB.h
	$(CC) $(CFLAGS) -c $(SRCDIR)/hashDB.c -o hashDB.o

clean:
	rm *.o
