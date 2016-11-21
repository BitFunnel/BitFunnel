# Take the output of the BitFunnel correlate command and generate histograms.

from collections import defaultdict
import csv

term_term_correlation = defaultdict(int)
term_all_correlation = defaultdict(int)

# TODO: don't hardcode name.
with open ("/tmp/Correlate-0.csv") as f:
    reader = csv.reader(f)
    for row in reader:
        term_all = 0
        pos = 0
        for item in row:
            if pos > 0 and pos % 2 == 0:
                correlation = int(item)
                term_all += correlation
                term_term_correlation[correlation] += 1
            pos += 1
        term_all_correlation[term_all] += 1

print(term_term_correlation)
print(term_all_correlation)
