import csv
import sys


filename = "/tmp/multidensity-temp.csv"
with open(filename) as f:
    reader = csv.reader(f)
    header = next(reader)
    assert header == ['treatment','variant', 'density', 'bits', 'qps']
    writer = csv.writer(sys.stdout, delimiter= '&')
    header[-1] += "\\\\"
    writer.writerow(header)
    print("\\midrule")
    for row in reader:
        row[-1] += "\\\\"
        writer.writerow(row)

