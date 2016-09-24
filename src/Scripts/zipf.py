from collections import defaultdict
import numpy as np

num_increments = 12
outer_trials = 10
inner_trials = outer_trials * 100
zipf_parameter = 2
running_sum = defaultdict(int)
num_entries = defaultdict(int)

def add_postings(postings, old_num, new_num):
    for _ in range (old_num, new_num):
        postings[np.random.zipf(zipf_parameter)] += 1

for _ in range (outer_trials):
    postings = defaultdict(int)
    num_postings_last = 0
    num_postings = 1024
    add_postings(postings, num_postings_last, num_postings)

    for _ in range (num_increments):
        num_postings_last = num_postings
        num_postings *= 2
        add_postings(postings, num_postings_last, num_postings)

        for _ in range (inner_trials):
            draw = np.random.zipf(zipf_parameter)
            local_num = postings[draw]
            # print('{},{}'.format(num_postings, postings[draw]))
            running_sum[num_postings] += local_num
            num_entries[num_postings] += num_postings

for key in sorted(running_sum):
    print('{},{}'.format(key, running_sum[key]/num_entries[key]))
