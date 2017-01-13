#!/bin/bash

set -e
set -x

echo "Bash version ${BASH_VERSION}..."

outline="Query Log,Phi,Bytes,SNR,QPS"
echo $outline >> /tmp/multisnr.csv

for snr in 10.0 100.0 1200.0 20000.0; do
    # Build TermTable
    tools/BitFunnel/src/BitFunnel termtable ~/dev/wikipedia.100.150/config/ 0.15 Optimal -snr $snr

for query_log in docfreq generated; do
    outline=$query_log
    outline+=","
    outline+=$snr
    outline+=","
    outline+=`python3 ../src/Scripts/get-memory-usage.py  /home/danluu/dev/wikipedia.100.150/config/TermTableStatistics-0.txt`

    # Run BitFunnel for output
    tools/BitFunnel/src/BitFunnel repl ~/dev/wikipedia.100.150/config/ -script ~/dev/wikipedia.100.150/script.output.$query_log
    # Check output against verifier
    python3 ../src/Scripts/verify.py ~/dev/wikipedia.100.150/$query_log.d20.groundTruth.csv ~/dev/BitFunnel/build-ninja/verificationOutput.csv /tmp/verified.csv
    true_pos=`python3 ..//src/Scripts/sum-csv-column.py /tmp/verified.csv 2`
    false_pos=`python3 ..//src/Scripts/sum-csv-column.py /tmp/verified.csv 3`
    outline+=","
    outline+=`echo "scale=2; $true_pos/$false_pos" | bc`

    # Run BitFunnel for QPS
    tools/BitFunnel/src/BitFunnel repl ~/dev/wikipedia.100.150/config/ -script ~/dev/wikipedia.100.150/script.query.$query_log
    # 6th line, 2nd field should be QPS.
    outline+=","
    outline+=`awk 'FNR == 6 {print $2}' /tmp/QuerySummaryStatistics.txt`
    echo $outline >> /tmp/multisnr.csv
    done
done
