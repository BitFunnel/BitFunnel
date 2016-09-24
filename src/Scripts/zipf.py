from collections import defaultdict
import numpy as np

num_increments = 6
outer_trials = 100
zipf_documents = 2
zipf_query = 1.2
running_sum = defaultdict(lambda: defaultdict(int))
num_entries = defaultdict(lambda: defaultdict(int))
max_query_terms = 10

def add_postings(postings, old_num, new_num):
    for _ in range (old_num, new_num+1):
        postings[np.random.zipf(zipf_documents)] += 1

def trial_with_n_terms(postings, running_sum, num_entries, zipf_query, terms):
    lengths = []
    for _ in range (terms):
        draw = np.random.zipf(zipf_query)
        lengths.append(postings[draw])
    running_sum[terms][num_postings] += min(lengths)
    num_entries[terms][num_postings] += num_postings
    # num_entries[terms][num_postings] += len(postings)

# TODO: we could use a ring buffer and re-use all of these draws.
# This is extremely wasteful.
def run_trial(postings, running_sum, num_entries, zipf_query):
    # print('{},{}'.format(num_postings, postings[draw]))
    for terms in range (1, max_query_terms+1):
        trial_with_n_terms(postings, running_sum, num_entries, zipf_query, terms)

# In each outer trials loop we create a dict of postings from scratch
# To do this, we quadruple the number of postings we have num_increments times.
# Within each "increment", we draw inner_trials random queries.
for _ in range (outer_trials):
    postings = defaultdict(int)
    num_postings_last = 0
    num_postings = 1024
    inner_trials = 10
    add_postings(postings, num_postings_last, num_postings)

    for _ in range (num_increments):
        num_postings_last = num_postings
        num_postings *= 4
        inner_trials *= 2
        add_postings(postings, num_postings_last, num_postings)

        for _ in range (inner_trials):
            run_trial(postings, running_sum, num_entries, zipf_query)

for terms in sorted(running_sum):
    for postings in sorted(running_sum[terms]):
        print('{}:{},{}'.format(terms, postings, running_sum[terms][postings]/num_entries[terms][postings]))

# We have the average ratio between a random list and the number of postings.
# 1. We want the ratio of random list to number of distinct terms.
# 2. We want the ratio of, for N random lists, the shortest of N random lists and the number of ???
