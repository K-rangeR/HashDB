# Test pipeline
Simple pipeline that takes an input file containing testing stages and testing data.

## Stages and Syntax
segf_put \[segment file ID] \[number of key value pairs]<br>
\[key] \[value]<br>
...<br>

segf_delete \[segment file ID] \[number of keys\]<br>
\[key]<br>
...<br>

segf_update \[segment file ID] \[number of key value pairs]<br>
\[key] \[value]<br>
...<br>

hashdb_put \[number of key value pairs]<br>
\[key] \[value]<br>
...<br>

hashdb_delete \[number of keys]<br>
\[key]<br>
...<br>

hashdb_update \[number of key value pairs]<br>
\[key] \[value]<br>
...<br>

hashdb_compact \[segment file ID]<br>

hashdb_merge \[segment file ID] \[segment file ID]<br>

## Example Stage File
Note, stages are separated by newlines.
```
segf_put 1 2
1 one
2 two

segf_put 2 1
3 three

segf_delete 1
2

segf_update 3 1
3 THREE

hashdb_put 2
4 four
5 five

hashdb_compact 1

hashdb_merge 1 2
```

## Building and running pipeline
```
$ make
$ ./testpl stage_file.txt
```

## Why is there no ```*_get``` stage?
Get is implied with put, delete, and update. That is, key value pairs that are inserted, deleted, or updated in the database are verified correct by reading them back from the database.

## Things to note
* The stage file parsing code is a little fragile so not all syntax errors will be caught
* All segment files are deleted after the pipeline has completed regardless of passing or failing test stages
