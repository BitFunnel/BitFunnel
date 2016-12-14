# Generate some random 64-bit values to use as contants for random array.

import numpy as np
import binascii

np.random.seed(0)

# TODO: check that hashes are optimal. This should be straightforward, but I'm
# checking this in early because even a potentially non-optimal version is a
# huge improvement.

num_entries = 64;
print("{")
for _ in range(num_entries):
    print("0x" + str(binascii.hexlify(np.random.bytes(8)).decode()) + ",")
print("}")
