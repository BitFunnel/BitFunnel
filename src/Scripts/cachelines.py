# Takes a Document Frequency Table and caclulates the expected number of cache
# misses for each term.
#
# TODO: allow use of different TermTreatments.

# TODO: this assumes we subtract 1 from 'rows' because the DocumentActive row is counted as a row. Check that assumption!

from collections import defaultdict
import csv
import math
import sys

args = sys.argv[1:]
if len(args) != 3:
    print("Required args: [DocFreqTable filename], [QueryPipelineStatistics filename], [output filename]")
    sys.exit(1)
docfreq_filename = args[0]
querypipeline_filename = args[1]
output_filename = args[2]

frequency = defaultdict(float)

with open(docfreq_filename) as f:
    reader = csv.reader(f)
    header = next(reader)
    assert header == ['hash','gramSize', 'streamId', 'frequency', 'text']
    for dft_row in reader:
        frequency[dft_row[4]] = float(dft_row[3])

cachelines_per_row = 60 # TODO: don't hardcode this.
density = 0.1 # TODO: don't hardcode this.

with open(querypipeline_filename) as f:
    reader = csv.reader(f)
    qp_header = next(reader)
    assert qp_header == ['query',
                         'rows',
                         'matches',
                         'quadwords',
                         'cachelines',
                         'parse',
                         'plan',
                         'match']
    outf = open(output_filename, 'w', newline='')
    writer = csv.writer(outf)
    writer.writerow(['Query',
                     'TermPos',
                     'Quadwords',
                     'Cachelines',
                     'ExpectedCachelines'])

    pos = 0
    for qp_row in reader:
        term = qp_row[0]
        s = frequency[term]
        num_rows = int(qp_row[1]) - 1 # TODO: should we subtract 1?
        expected_misses = 0
        if num_rows == 1:
            # private row.
            expected_misses = cachelines_per_row
        else:
            # TODO: consider accounting for actual row density.
            for i in range(1, num_rows + 1):
                p = 1 - math.pow((1 - math.pow(density - s, i)) * (1 - s), 512)
                expected_misses += p * cachelines_per_row
        writer.writerow([term,
                         pos,
                         qp_row[3], # quadwords
                         qp_row[4], # cachlines
                         expected_misses])
        pos += 1
