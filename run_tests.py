#!/usr/bin/env python3
import os
from subprocess import check_output
import re
from time import sleep

#
#  Feel free (a.k.a. you have to) to modify this to instrument your code
#
'''
# Step 1 Test
THREADS = [2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32]
LOOPS = [100000]
INPUTS = ["seq_64_test.txt", "1k.txt", "8k.txt", "16k.txt" ]

THREADS = [0]
LOOPS = [100000]
INPUTS = ["seq_64_test.txt", "1k.txt", "8k.txt", "16k.txt" ]
'''

# Step 2 Test
THREADS = [2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32]
LOOPS = [10, 100, 1000, 10000, 100000]
INPUTS = ["seq_64_test.txt", "1k.txt", "8k.txt", "16k.txt" ]
'''
THREADS = [0]
LOOPS = [10, 100, 1000, 10000, 100000]
INPUTS = ["seq_64_test.txt", "1k.txt", "8k.txt", "16k.txt" ]
'''

csvs = []
for inp in INPUTS:
    for loop in LOOPS:
        csv = ["{}/{}".format(inp, loop)]
        for thr in THREADS:
            cmd = "./bin/prefix_scan -o temp.txt -n {} -i tests/{} -l {}".format(
                thr, inp, loop)
            out = check_output(cmd, shell=True).decode("ascii")
            m = re.search("time: (.*)", out)
            if m is not None:
                time = m.group(1)
                csv.append(time)

        csvs.append(csv)
        sleep(0.5)

header = ["microseconds"] + [str(x) for x in THREADS]

print("\n")
print(", ".join(header))
for csv in csvs:
    print (", ".join(csv))
