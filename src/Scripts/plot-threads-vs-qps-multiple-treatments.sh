# Uses generate-queries-for-threads.py to generate input script.
python per-thread-qps.py /tmp/rank0.100.200/threads  /tmp/rank3rank0.100.200/threads /tmp/rankn.100.200/threads > /tmp/threads-vs-qps.csv
Rscript plot-qps.r /tmp/threads-vs-qps.csv threads-vs-qps.png
