import csv
import sys

# As a hack to get sorted output, sort input with:
# (head -n 1 multidensity.csv && tail -n +2 multidensity.csv | sort) > multidensity-temp.csv

# TODO: take this from BitFunnel instead of hardcoding.
input_size = 1297.97

filename = "/tmp/multisnr-temp.csv"
with open(filename) as f:
    reader = csv.reader(f)
    header = next(reader)
    # assert header == ['treatment','variant', 'density', 'bits', 'qps']
    # assert header == ['Treatment','Density', 'Bytes', 'QPS']
    assert header == ['Query Log','Phi', 'Bytes', 'SNR', 'QPS']
    # header[1] = 'target'
    header[-3] = 'Size (MB)'
    header[-1] = 'kQPS'
    header[0] = 'System'
    header[1] = 'Query Log'
    header.append('DQ')
    writer = csv.writer(sys.stdout, delimiter= '&')

    header[-1] += "\\\\"
    writer.writerow(header)

    last_querylog = ""
    for row in reader:
    #         print("Error: expected Rank0 or RankN treatment")
    #         assert False
        if row[0] != last_querylog:
            print("\\midrule")
            last_querylog = row[0]

        # Overwrite Phi with QueryLog
        row[1] = row[0]
        if row[1] == "docfreq":
            row[1] = "Unigram"
        elif row[1] == "generated":
            row[1] = "Multiterm"
        else:
            print("Error: expected docfreq or generated query log")
            assert False

        row[0] = "BitFunnel"

        # row[-4] = str(float(row[-4]) / 1297.97)
        row[-3] = str(float(row[-3]) * 651587 / 1000000)

        dq = "{0:.0f}".format(float(row[-1]) / float(row[-3]))
        row.append(dq)

        # convert from qps to kqps.
        row[-2] = "{0:.0f}".format(float(row[-2])/1000)

        row[-4] = "{0:.0f}".format(float(row[-4]))

        row[-1] += "\\\\"
        writer.writerow(row)
    print("\\midrule")
