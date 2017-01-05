import re
import sys

args = sys.argv[1:]
if len(args) == 0:
    print("Required args: [Base threads directory]+")
    sys.exit(1)

base_dir = args

print("Threads,QPS,Treatment")
for dir in base_dir:
    treatment = re.sub(r'/tmp/','',dir)
    treatment = re.sub(r'/threads','',treatment)
    treatment = re.sub(r'100.200','',treatment)
    for i in range(2, 17, 2):
        summary_name = "{}/{}/QuerySummaryStatistics.txt".format(dir, i)
        with open(summary_name) as f:
            lines = f.readlines()
            assert lines[5][0:3] == "QPS"
            qps = float(re.split(': ', lines[5])[1].rstrip())
            print("{},{},{}".format(i,qps, treatment))
