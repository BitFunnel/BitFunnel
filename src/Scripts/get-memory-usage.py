# This is a hack -- it's possible to get this direct from BitFunnel, but this gets it from the stats file that we're printing out by summing up the sizes for each rank.

import re
import sys

args = sys.argv[1:]
if len(args) != 1:
    print("Required arg: [filename]")
filename = args[0]

bpd = re.compile("\s+Bytes per document:\s+(\d+\.*\d*)")

# TODO: take a column name instead of a column number as an argument.

with open(filename) as f:
    sum = 0.0
    for line in f:
        match = re.match(bpd, line)
        if match is not None:
            sum += float(match.group(1))
    print(sum)
        
