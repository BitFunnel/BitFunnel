import csv
import sys

args = sys.argv[1:]
if len(args) != 2:
    print("Required arg: [filename], [column number]")
filename = args[0]
col = int(args[1])
    
# TODO: take a column name instead of a column number as an argument.

with open(filename) as f:
    reader = csv.reader(f)
    header = next(reader)
    sum = 0
    for row in reader:
        sum += int(row[col])
    print(sum)
