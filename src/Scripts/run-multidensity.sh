#!/bin/bash

set -e
set -x

echo "Bash version ${BASH_VERSION}..."

#outline="treatment,variant,density,bits,qps"
outline="Treatment,Density,Bits,QPS,TruePos,FalsePos"
echo $outline >> /tmp/multidensity.csv

for treatment in ClassicBitsliced PrivateSharedRank0 Optimal; do
    for dd in `seq 0.05 0.05 0.25`; do
    outline=$treatment
    outline+=","
    outline+=$dd
    # Build TermTable
    tools/BitFunnel/src/BitFunnel termtable ~/dev/wikipedia.100.150/config/ $dd $treatment
    outline+=","
    outline+=`python3 ../src/Scripts/get-memory-usage.py  /home/danluu/dev/wikipedia.100.150/config/TermTableStatistics-0.txt`

    # Run BitFunnel for output
    tools/BitFunnel/src/BitFunnel repl ~/dev/wikipedia.100.150/config/ -script ~/dev/wikipedia.100.150/script.output.docfreq
    # Check output against verifier
    python3 ../src/Scripts/verify.py ~/dev/wikipedia.100.150/d20.groundTruth.csv ~/dev/BitFunnel/build-ninja/verificationOutput.csv /tmp/verified.csv
    true_pos=`python3 ..//src/Scripts/sum-csv-column.py /tmp/verified.csv 2`
    false_pos=`python3 ..//src/Scripts/sum-csv-column.py /tmp/verified.csv 3`
    outline+=","
    outline+=`echo "scale=2; $true_pos/$false_pos" | bc`


    # Run BitFunnel for QPS
    tools/BitFunnel/src/BitFunnel repl ~/dev/wikipedia.100.150/config/ -script ~/dev/wikipedia.100.150/script.query.docfreq
    # 6th line, 2nd field should be QPS.
    outline+=","
    outline+=`awk 'FNR == 6 {print $2}' /tmp/QuerySummaryStatistics.txt`
    echo $outline >> /tmp/multidensity.csv
    done

done
