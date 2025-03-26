geijer = 'dcbo_geijer.txt'
fenwick = 'dcbo_fenwick.txt'

gerrs = {}
ferrs = {}

with open(geijer, 'r') as f:
    for line in f:
        if line.startswith('Running'):
            continue
        if line.startswith('(max'):
            break
        parts = line.split(' ')
        gerrs[parts[0]] = parts[1].strip('\n')

with open(fenwick, 'r') as f:
    for line in f:
        if line.startswith('Running'):
            continue
        if line.startswith('(max'):
            break
        parts = line.split(' ')
        ferrs[parts[0]] = parts[1].strip('\n')

fenwick_missing = 0
err_mismatch = 0
err_match = 0
for k in gerrs:
    if k not in ferrs:
        print(f'{k} not in fenwick')
        fenwick_missing += 1
        continue
    if gerrs[k] != ferrs[k]:
        err_mismatch += 1
        print(f'{k} has diff: gerr={gerrs[k]}, ferr={ferrs[k]}')
    else:
        err_match += 1

print(f"Match: {err_match}, Mismatch: {err_mismatch}, Missing: {fenwick_missing}")