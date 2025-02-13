# std::queue<Reorder> currIter{};
# 		std::queue<Reorder> nextIter{};
# 		std::unordered_set<size_t> visited{};

# 		currIter.push({0, nodes.size() - 1, 0});
# 		size_t numLevels = std::ceil(std::log2(nodes.size()));

# 		for (size_t i = 0; i < numLevels; ++i) {
# 			while (!currIter.empty()) {
# 				Reorder &r = currIter.front();
# 				if (r.index >= nodes.size()) {
# 					continue;
# 				}

# 				size_t mid = (r.start + r.end) / 2;
# 				visited.insert(mid);
# 				arr[r.index] = nodes[mid];

# 				nextIter.push({r.start, mid, 2 * r.index + 1});
# 				nextIter.push({mid + 1, r.end, 2 * r.index + 2});

# 				currIter.pop(); // do this last to ensure valid reference
# 			}
# 			currIter.swap(nextIter);
# 		}

from collections import deque

def build(nodes):
    arr = [None] * len(nodes)
    currIter = deque()
    nextIter = deque()
    visited = set()

    currIter.append((0, len(nodes) - 1, 0))
    numLevels = len(nodes).bit_length()

    print(f"NumLevels: {numLevels}")
    for i in range(numLevels):
        print(f"Level: {i}")
        while currIter:
            start, end, index = currIter.popleft()
            mid = (start + end) // 2
            # if ((end - start) % 2 == 1):
            #     mid += 1
            if (mid in visited):
                continue
            if start >= len(nodes) or index >= len(arr):
                continue

            visited.add(mid)
            arr[index] = nodes[mid]

            nextIter.append((start, mid, (2 * index) + 1))
            nextIter.append((mid + 1, end, (2 * index) + 2))

        currIter, nextIter = nextIter, currIter

    return arr

def build2(nodes):
    arr = [None] * len(nodes)
    currIter = deque()
    nextIter = deque()
    visited = set()

    currIter.append((0, len(nodes) - 1, 0))
    numLevels = len(nodes).bit_length()

    padding = 0
    print(f"NumLevels: {numLevels}")
    for i in range(numLevels):
        if (i > 0):
            padding += (2 ** (i - 1))
        for j in range(len(currIter)):
            start, end, _ = currIter.popleft()
            mid = (start + end) // 2
            if ((end - start) % 2 == 0) and end != start:
                mid += 1
            if mid in visited:
                continue
            if start >= len(nodes):
                print(f"Start: {start} >= {len(nodes)}")
                continue
            if mid >= len(nodes):
                print(f"Mid: {mid} >= {len(nodes)}")
                print(f"Start: {start}, End: {end}, Mid: {mid}")
                continue

            print(f"nodes[{mid}] -> arr[{padding} + {j}]  ({start}, {end}, {mid})")
            visited.add(mid)
            if (padding + j >= len(arr)):
                print(f"padding: {padding}, j: {j}")
                return []
            arr[padding + j] = nodes[mid]

            nextIter.append((start, mid, 0))
            nextIter.append((mid + 1, end, 0))
        currIter, nextIter = nextIter, currIter

    return arr

nodes = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
print(f"Original: {nodes}")
print(f"Expected: [6, 3, 8, 2, 5, 7, 9, 0, 1, 4]")
# print(f"Build: {build(nodes)}") # [4, 2, 6, 1, 3, 5, 7]
print(f"Build2: {build2(nodes)}") # [4, 2, 6, 1, 3, 5, 7]