from collections import defaultdict
import numpy as np

postings = defaultdict(int)
for _ in range (10000):
    postings[np.random.zipf(1.2)] += 1

for key, value in postings.items():
    print('{},{}'.format(key, value))
