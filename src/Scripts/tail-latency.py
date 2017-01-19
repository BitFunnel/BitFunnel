# Note that this is not a valid measurement of tail latency. This uses the execution times we measure because they're convenient, but this does not include queueing time inside BitFunnel nor does it include head-of-line blocking queue waiting time on the queue into BitFunnel.

import csv

filename = "/tmp/QueryPipelineStatistics.csv"

times = []

with open(filename) as f:
    reader = csv.reader(f)
    header = next(reader)
    assert header == ['query',
                      'rows',
                      'matches',
                      'quadwords',
                      'cachelines',
                      'parse',
                      'plan',
                      'match']
    for row in reader:
        total_time = float(row[-1]) + float(row[-2]) + float(row[-3])
        times.append(total_time)

times.sort(reverse=True)

idx_max = len(times) - 1
idx = [round(idx_max / 2),
       round(idx_max / 10),
       round(idx_max / 100),
       round(idx_max / 1000),
       0]

tails = [times[x] for x in idx]
print(tails)
