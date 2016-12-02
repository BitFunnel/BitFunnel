# Uses generate-queries-for-threads.py to generate input script.
python per-thread-qps.py > /tmp/threads-vs-qps.csv
Rscript plot-qps.r /tmp/threads-vs-qps.csv threads-vs-qps.png
