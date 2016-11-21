# Take the output of the BitFunnel correlate command and generate histograms.

from collections import defaultdict
import csv

term_term_correlation = defaultdict(lambda:defaultdict(int))
term_all_correlation = defaultdict(lambda:defaultdict(int))

def bf_correlate_to_dicts(term_term_correlation,
                          term_all_correlation,
                          basepath,
                          treatment):
    filename = basepath + "-" + treatment + ".csv"
    with open(filename) as f:
        reader = csv.reader(f)
        for row in reader:
            term_all = 0
            pos = 0
            for item in row:
                if pos > 0 and pos % 2 == 0:
                    correlation = int(item)
                    term_all += correlation
                    term_term_correlation[treatment][correlation] += 1
                pos += 1
            term_all_correlation[treatment][term_all] += 1

def dict_to_csv(dd, filename):
    with open(filename, 'w') as f:
        writer = csv.writer(f)
        writer.writerow(["bucket","y","treatment"])
        for treatment,subdict in dd.items():
            for k, v in subdict.items():
                writer.writerow([k,v,treatment])

bf_correlate_to_dicts(term_term_correlation,
                      term_all_correlation,
                      "/tmp/correlate-150k",
                      "rank3-rank0")
bf_correlate_to_dicts(term_term_correlation,
                      term_all_correlation,
                      "/tmp/correlate-150k",
                      "rank0")
dict_to_csv(term_term_correlation, "/tmp/term-term.csv")
dict_to_csv(term_all_correlation, "/tmp/term-all.csv")
