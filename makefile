hashDB: main.o db.o
	g++ --std=c++11 -Wall main.o db.o -o hashDB

main.o: main.cpp hashDB.h
	g++ --std=c++11 -Wall -c main.cpp -o main.o

db.o: hashDB.o segment.o
	ld -r hashDB.o segment.o -o db.o

hashDB.o: hashDB.cpp hashDB.h segment.h
	g++ --std=c++11 -Wall -c hashDB.cpp -o hashDB.o

segment.o: segment.cpp segment.h
	g++ --std=c++11 -Wall -c segment.cpp -o segment.o

clean:
	rm *.o hashDB
