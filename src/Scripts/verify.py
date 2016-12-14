# Take a ground truth file produced by the verifier and a match file and compare them.
# Output is in fully normalized format, the same as VerifyCommand.cpp produces.
#

# Note: requires Python3.

# file format:
# term,docId,[0-3]
# 0: true positive
# 1: false postive
# 2: false negative
# 3: unverified

from collections import defaultdict
import csv
import sys

args = sys.argv[1:]
if len(args) != 3:
    print("Required args: [groundTruth filename], [unknown filename], [output filename]")

groundtruth_filename = args[0]
unknown_filename = args[1]
output_filename = args[2]

def get_true_matches(filename):
    true_matches = defaultdict(set)
    with open(filename) as f:
        reader = csv.reader(f)
        for row in reader:
            if (len(row) == 3 and (row[2] == '0' or row[2] == '2')):
                true_matches[row[0]].add(row[1])
    return true_matches

def process_unknowns(unknown_filename, output_filename, true_matches):
    with open(unknown_filename) as f:
        cf = open(output_filename, 'w', newline='')
        writer = csv.writer(cf)
        writer.writerow(["Query","TermPos","TruePositives","FalsePositives","FalseNegatives","FalseRate"])
        reader = csv.reader(f)
        last_term = ""
        num_true_pos = 0
        num_false_pos = 0
        num_false_neg = 0 # TODO: handle.
        term_pos = -1
        for row in reader:
            # print(row)
            # TODO: assert that value is '3'
            #
            # TODO: handle false negatives. Could keep a counter of how many matches
            # we've seen and compare, then iterate over the set in the rare instance
            # we see a false negative.
            #
            # TODO: handle last term.
            if row[0] != last_term:
                if (term_pos > 0):
                    writer.writerow([last_term,
                                     term_pos,
                                     num_true_pos,
                                     num_false_pos,
                                     num_false_neg,
                                     num_false_pos / (num_true_pos + num_false_pos)])
                num_true_pos = 0
                num_false_pos = 0
                num_false_neg = 0 # TODO: handle.
                term_pos += 1

            if (len(row) == 3):
                if row[1] in true_matches[row[0]]:
                    num_true_pos += 1
                else:
                    num_false_pos += 1
            last_term = row[0]

def run():
    true_matches = get_true_matches(groundtruth_filename)
    process_unknowns(unknown_filename, output_filename, true_matches)

run()
