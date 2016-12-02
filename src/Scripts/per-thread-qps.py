import re

print("Threads,QPS")
for i in range(1, 33):
    summaryName = "/tmp/threads/{}/QuerySummaryStatistics.txt".format(i)
    with open(summaryName) as f:
        lines = f.readlines()
        assert lines[5][0:3] == "QPS"
        qps = float(re.split(': ', lines[5])[1].rstrip())
        print("{},{}".format(i,qps))
