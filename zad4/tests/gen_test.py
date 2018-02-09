#!/usr/local/bin/python3.6
import random
import sys

if len(sys.argv) != 2:
	print("Usage: ./gen_test.py i")

i = sys.argv[1]
in_fname = i + ".in"
out_fname = i + ".out"

n = random.randint(4, 5)
l = [random.randint(0, 100000) for _ in range(2*n)]
ls = sorted(l)

with open(in_fname, 'w') as in_file:
	in_file.write(str(n) + "\n")
	for x in l:
		in_file.write(str(x) + "\n")

with open(out_fname, 'w') as out_file:
	for x in ls:
		out_file.write(str(x) + "\n")
