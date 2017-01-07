import csv
import sys


filename = "/tmp/multidensity-temp.csv"
with open(filename) as f:
    reader = csv.reader(f)
    header = next(reader)
    assert header == ['treatment','variant', 'density', 'bits', 'qps']
    header[-2] = 'bits/doc'
    header[-1] = 'kqps'
    header.append('dq')
    writer = csv.writer(sys.stdout, delimiter= '&')

    header[-1] += "\\\\"
    writer.writerow(header)

    print("\\midrule")
    for row in reader:
        dq = "{0:.0f}".format(float(row[-1]) / float(row[-2]))
        row.append(dq)

        # convert from qps to kqps.
        row[-2] = "{0:.0f}".format(float(row[-2])/1000)

        row[-3] = "{0:.0f}".format(float(row[-3]))

        row[-1] += "\\\\"
        writer.writerow(row)

