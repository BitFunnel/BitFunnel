import csv
import sys

# As a hack to get sorted output, sort input with:
# (head -n 1 multidensity.csv && tail -n +2 multidensity.csv | sort) > multidensity-temp.csv

filename = "/tmp/multidensity-temp.csv"
with open(filename) as f:
    reader = csv.reader(f)
    header = next(reader)
    assert header == ['treatment','variant', 'density', 'bits', 'qps']
    header[1] = 'target'
    header[-2] = 'bytes/doc'
    header[-1] = 'kqps'
    header.append('dq')
    writer = csv.writer(sys.stdout, delimiter= '&')

    header[-1] += "\\\\"
    writer.writerow(header)

    print("\\midrule")
    for row in reader:
        if row[0] == "Rank0":
            row[1] = "N/A"
        elif row[0] == "RankN":
            if row[1] == "0":
                row[1] = "DQ"
            elif row[1] == "1":
                row[1] = "Q"
            elif row[1] == "2":
                row[1] = "D"
        else:
            print("Error: expected Rank0 or RankN treatment")
            assert False


        dq = "{0:.0f}".format(float(row[-1]) / float(row[-2]))
        row.append(dq)

        # convert from qps to kqps.
        row[-2] = "{0:.0f}".format(float(row[-2])/1000)

        row[-3] = "{0:.0f}".format(float(row[-3]))

        row[-1] += "\\\\"
        writer.writerow(row)

