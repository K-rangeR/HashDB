# Test pipeline
Simple pipeline that takes an input file containing testing stages and testing data.

## Stages
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
