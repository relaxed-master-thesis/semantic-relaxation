import sys

file = sys.argv[1]
ints = []
with open(file, 'r') as f:
    lines = f.readlines()
    lines = [line.strip().split(' ')[1] for line in lines]
    ints = [int(line) for line in lines]

s = set()
for i in range(5):
    print(ints[i])
    if ints[i] in s:
        print(f"Duplicate found: {ints[i]}")
    else:
        s.add(ints[i])
        # print(f"Unique: {ints[i]}")