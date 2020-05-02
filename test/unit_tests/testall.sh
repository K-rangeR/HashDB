#!/usr/bin/env bash

# Builds and runs all tests, if a build or test fails any subsequent builds
# or tests are not run

echo "Building tests..."
make check_memtable || { echo "ERROR: make check_memtable failed" ; exit 1; }
make check_segment  || { echo "ERROR: make check_segment failed"  ; exit 1; }
make check_hashDB   || { echo "ERROR: make check_hashDB failed"   ; exit 1; }
echo

# Run in a bottom up order
echo "Running tests..."
./check_memtable || { exit 1; }
echo 
./check_segment  || { exit 1; }
echo 
./check_hashDB   || { exit 1; }
echo

echo "Cleaning up after tests"
make clean &> /dev/null
rm check_memtable check_segment check_hashDB
