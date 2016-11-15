# Take a ground truth file produced by the verifier and a match file and compare them.
# Output is in fully normalized format, the same as VerifyCommand.cpp produces.
#

# TODO: remove hardcoded paths.

# file format:
# term,docId,[0-3]
# 0: true positive
# 1: false postive
# 2: false negative
# 3: unverified

from collections import defaultdict
import csv

true_matches = defaultdict(set)

with open("/tmp/groundTruth.csv") as f:
    reader = csv.reader(f)
    for row in reader:
        if (len(row) == 3 and (row[2] == '0' or row[2] == '2')):
            true_matches[row[0]].add(row[1])

with open("/tmp/unknowns.csv") as f:
    reader = csv.reader(f)
    for row in reader:
        # TODO: assert that value is '3'
        #
        # TODO: handle false negatives. Could keep a counter of how many matches
        # we've seen and compare, then iterate over the set in the rare instance
        # we see a false negative.
        if (len(row) == 3):
            if row[1] in true_matches[row[0]]:
                print(row[0] + "," + row[1] + ",0")
            else:
                print(row[0] + "," + row[1] + ",1")
