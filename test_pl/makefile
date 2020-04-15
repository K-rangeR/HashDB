# Builds the testing pipeline program
CC = gcc
CFLAGS = -Wall -DTESTING

SRCDIR = ../src

testpl: main.o pipeline.o stage.o
	$(CC) $(CFLAGS) main.o pipeline.o stage.o -o testpl

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

pipeline.o: pipeline.c pipeline.h stage.h
	$(CC) $(CFLAGS) -c pipeline.c -o pipeline.o

stage.o: stage.c stage.h
	$(CC) $(CFLAGS) -c stage.c -o stage.o

#test_stages.o: ts.o db.o
	#$(CC) $(CFLAGS) -c ts.o db.o -o test_stages.o

#test_stages.o: ts.o
	#$(CC) $(CFLAGS) -c ts.o -o test_stages.o

#ts.o: test_stages.c test_stages.h stage.h
	#$(CC) $(CFLAGS) -c test_stages.c -o ts.o

#db.o: memtable.o segment.o hashDB.o
	#$(CC) $(CFLAGS) -c memtable.o segment.o hashDB.o -o db.o

#memtable.o: $(SRCDIR)/memtable.c $(SRCDIR)/memtable.h
	#$(CC) $(CFLAGS) -c $(SRCDIR)/memtable.c -o memtable.o

#segment.o: $(SRCDIR)/segment.c $(SRCDIR)/segment.h
	#$(CC) $(CFLAGS) -c $(SRCDIR)/segment.c -o segment.o

#hashDB.o: $(SRCDIR)/hashDB.c $(SRCDIR)/hashDB.h
	#$(CC) $(CFLAGS) -c $(SRCDIR)/hashDB.c -o hashDB.o

clean:
	rm *.o
