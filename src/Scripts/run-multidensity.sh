#!/bin/bash

set -e
set -x

echo "Bash version ${BASH_VERSION}..."

for dd in `seq 0.1 0.05 0.35`
do
    # for treatment in PrivateSharedRank0 Optimal; do
    outline=$treatment
    outline+=","
    outline+=$dd
    # Build TermTable
    tools/BitFunnel/src/BitFunnel termtable ~/dev/wikipedia.100.150/config/ $dd Optimal
    outline+=","
    outline+=`python3 ../src/Scripts/get-memory-usage.py  /home/danluu/dev/wikipedia.100.150/config/TermTableStatistics-0.txt`

    # # Run BitFunnel for output
    # tools/BitFunnel/src/BitFunnel repl ~/dev/wikipedia.100.200/config/ -script ~/dev/wikipedia.100.200/script.output
    # # Check output against verifier
    # python3 ../src/Scripts/verify.py ~/dev/wikipedia.100.200/d20.groundTruth.csv ~/dev/BitFunnel/build-ninja/verificationOutput.csv /tmp/verified.csv    
    # outline+=","
    # outline+=`python3 ..//src/Scripts/sum-csv-column.py /tmp/verified.csv 3`

    # Run BitFunnel for QPS
    tools/BitFunnel/src/BitFunnel repl ~/dev/wikipedia.100.150/config/ -script ~/dev/wikipedia.100.150/script.query.docfreq
    # 6th line, 2nd field should be QPS.
    outline+=","
    outline+=`awk 'FNR == 6 {print $2}' /tmp/QuerySummaryStatistics.txt`
    echo $outline >> /tmp/multidensity.csv
    # done
done    
