# Tests
Directory containing unit tests. Tests are run with the help of
the [Check testing framework](https://libcheck.github.io/check/).

## Building Individual Tests
Use the makefile to build the test by its name:
```
make check_memtable
make check_segment
make check_hashDB
```

## Building and Running all Tests
To run all the tests run the following command:
```
$ ./testall.sh
```
