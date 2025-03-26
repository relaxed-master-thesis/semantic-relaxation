import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# folder = 'data/timestamps/FAKE/'
# folder = 'data/timestamps/2dd-queue-opt-1ms/'
folder = 'data/timestamps/dcbo-w32-n1-d1/'

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
    how='left'  # Use left join to keep all starts
)
# print(points)

# Create scatter plot
plt.figure(figsize=(8, 6))  # Set the figure size
plt.scatter(points['timestamp_start'], points['timestamp_end'], c='blue', alpha=0.6, edgecolors='black', s=50, label='Points')

# Customize plot
# if folder contains "2dd"
splits = folder.split('/')
name_split = splits[2].split('-')

if '2dd' in folder:
    w = name_split[1][1:]
    l = name_split[2][1:]
    i = name_split[3][1:]
    n = name_split[4][1:]
    d = name_split[5][1:]
    plt.title(f'2Dd-queue with params:\nwidth={w}, length={l}, initial size={i}, threads={n}, duration={d}ms', fontsize=14)
if 'dcbo' in folder:
    w = name_split[1][1:]
    n = name_split[2][1:]
    d = name_split[3][1:]
    plt.title(f'DCBO with params:\nwidth={w}, threads={n}, duration={d}ms', fontsize=14)
else:
    raise ValueError('Folder name not recognized')


plt.xlabel('Push time', fontsize=12)
plt.ylabel('Pop time', fontsize=12)
plt.legend()
plt.grid(True, linestyle='--', alpha=0.7)


# Show plot
plt.show()
