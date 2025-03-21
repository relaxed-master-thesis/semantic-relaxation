import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import re

# folder = 'data/timestamps/FAKE/'
# folder = 'data/timestamps/2dd-queue-opt-1ms/'
folder = 'data/benchData/2ddqopt-w512-l256-i1000000-n16-d100/'

# folder = 'data/timestamps/2dd-queue-opt-500ms/'
xs = pd.read_csv(
    folder + 'combined_put_stamps.txt',
    sep='\s+',
    header=None,
    names=['timestamp_start', 'id']
)

# Read the end times from the file
ys = pd.read_csv(
    folder + 'combined_get_stamps.txt',
    sep='\s+',
    header=None,
    names=['timestamp_end', 'id']
)

# Merge starts and ends on 'id'
points = pd.merge(
    xs,
    ys,
    on='id',
    how='inner'  # Use left join to keep all starts
)

# Create scatter plot
plt.figure(figsize=(8, 6))  # Set the figure size
plt.scatter(points['timestamp_start'], points['timestamp_end'], c='blue', alpha=0.6, edgecolors='black', s=50, label='Points')

# Customize plot
splits = folder.split('/')
name_split = splits[2].split('-')
w = name_split[1][1:]
l = name_split[2][1:]
i = name_split[3][1:]
n = name_split[4][1:]
d = name_split[5][1:]
plt.title(f'2Dd-queue with params:\nwidth={w}, length={l}, initial size={i}, threads={n}, duration={d}', fontsize=14)
plt.xlabel('Push time', fontsize=12)
plt.ylabel('Pop time', fontsize=12)
plt.legend()
plt.grid(True, linestyle='--', alpha=0.7)

xlines = []
ylines = []
i = 0
while i < len(points):
    # pushsum = sum(points.iloc[i:i + int(w) * int(l)]['timestamp_start'])
    # pushavg = pushsum / (int(w) * int(l))
    # popsum = sum(points.iloc[i:i + int(w) * int(l)]['timestamp_end'])
    # popavg = popsum / (int(w) * int(l))
    # xlines.append(pushavg)
    # ylines.append(popavg)
    xmin = min(points.iloc[i:i + int(w) * int(l)]['timestamp_start'])
    xmax = max(points.iloc[i:i + int(w) * int(l)]['timestamp_start'])
    ymin = min(points.iloc[i:i + int(w) * int(l)]['timestamp_end'])
    ymax = max(points.iloc[i:i + int(w) * int(l)]['timestamp_end'])
    # for j in range(int(w) * int(l)):
    #     x, y = points.iloc[i + j]['timestamp_start'], points.iloc[i + j]['timestamp_end']
    plt.axvline(x=xmin, color='r', linestyle='--')
    plt.axvline(x=xmax, color='g', linestyle='--')
    plt.axhline(y=ymin, color='r', linestyle='--')
    plt.axhline(y=ymax, color='g', linestyle='--')
    i += int(w) * int(l)

# for xline in xlines:
#     plt.axvline(x=xline, color='r', linestyle='--')
# for yline in ylines:
#     plt.axhline(y=yline, color='r', linestyle='--')

# min_x = min(points['timestamp_start'])
# max_x = max(points['timestamp_start'])
# i=0
# line = i*int(w)*int(l)*2+min_x
# while line < max_x:
#     plt.axvline(x=line, color='r', linestyle='--')
#     i+=1
#     line = i*int(w)*int(l)*2+min_x

# min_y = min(points['timestamp_end'])
# max_y = max(points['timestamp_end'])

# i=0
# line = i*int(w)*int(l)*2+min_y
# while line < max_y:
#     plt.axhline(y=line, color='r', linestyle='--')
#     i+=1
#     line = i*int(w)*int(l)*2+min_y

# Show plot
plt.show()
