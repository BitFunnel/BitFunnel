import csv

with open("/tmp/int/QueryPipelineStatistics.csv") as f:
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
    sum = 0
    for row in reader:
        sum += int(row[3])
    print(sum)
