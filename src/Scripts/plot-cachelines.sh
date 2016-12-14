# Temporary script to run commands to plot cachlines.
python3 cachelines.py ~/dev/wikipedia.100.200/config/DocFreqTable-0.csv /tmp/QueryPipelineStatistics.csv /tmp/Memory.csv
Rscript plot-qwords.r /tmp/Memory.csv qwords.png qwords-divided.png
