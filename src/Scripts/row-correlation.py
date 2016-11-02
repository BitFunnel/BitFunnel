# Find unusual row correlations between terms

from collections import defaultdict

# term -> rows associated with term.
terms_to_rows = dict()
# list of terms in order they were read in.
# TODO: should really use DocFreqTable order.
terms = []

# create mapping of terms to rows.
with open("/tmp/show.results.psv") as f:
    for line in f:
        parts = line.rstrip().split('|')
        term = parts[0]
        terms.append(term)
        terms_to_rows[term] = set()
        for row in parts[1:]:
            terms_to_rows[term].add(row)

# for term in terms:
#     print(term + str(terms_to_rows[term]))

terms_to_correlation = defaultdict(list)

# for each pair of terms, find out how many overlap.
# note that this double counts everything.
for left_term in terms:
    print(left_term, end="")
    for right_term in terms:
        if left_term != right_term:
            both_len = len(terms_to_rows[left_term].intersection(
                terms_to_rows[right_term]))
            if both_len > 1:
                # terms_to_correlation[left_term].append(both_len)
                print("," + right_term + "," + str(both_len), end="")
    print()


# for term in terms:
#    print(terms_to_correlation[term])
