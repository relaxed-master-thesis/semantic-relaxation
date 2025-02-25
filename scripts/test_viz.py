import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter, MinuteLocator, SecondLocator
import numpy as np
# from StringIO import StringIO
#Plot function
def timelines(y, xstart, xstop, color='b'):
    """Plot timelines at y from xstart to xstop with given color."""   
    plt.hlines(y, xstart, xstop, color, lw=4)
    plt.vlines(xstart, y+0.03, y-0.03, color, lw=2)
    plt.vlines(xstop, y+0.03, y-0.03, color, lw=2)


#read start intervals from a file
# folder = 'data/timestamps/2dd-queue-opt-1ms/'
# folder = 'data/timestamps/q-k-1ms-8t/'
# folder = 'data/timestamps/2dd-q-opt-w50-l10-i1000-8t-1ms/'
folder = 'data/timestamps/FAKE/'

starts = pd.read_csv(
    folder + 'combined_put_stamps.txt',
    sep='\s+',
    header=None,
    names=['timestamp_start', 'id']
)

# Read the end times from the file
ends = pd.read_csv(
    folder + 'combined_get_stamps.txt',
    sep='\s+',
    header=None,
    names=['timestamp_end', 'id']
)

# Merge starts and ends on 'id'
intervals = pd.merge(
    starts,
    ends,
    on='id',
    how='left'  # Use left join to keep all starts
)

# intervals = intervals.head(500)
print(intervals)
#sort intervals on id   
intervals = intervals.sort_values(by='id')

cap = range(len(intervals))
captions = [str(i + 1) for i in cap]
unique_idx = len(cap) - 1

# start = np.array([i[0] for i in intervals])
# stop = np.array([i[1] for i in intervals])
#only take the values from the starts and ends
start = np.array(intervals['timestamp_start'])
stop = np.array(intervals['timestamp_end'])
#find the biggest non nan stop value
unique_idx = np.nanargmax(stop)
print(stop[unique_idx])
#replace all nan values in stop with the biggest non nan value + 1
stop[np.isnan(stop)] = stop[unique_idx] + 1
#make stop values range the same as start values, avd
intervals = [(x,y) for x,y in zip(start, stop)]

print(start)
print(stop)
y = np.array([i + 1 for i in range(len(intervals))])
print(y)

for i, (xstart, xstop) in enumerate(intervals):
    timelines(i + 1, xstart, xstop, 'b')



#Setup the plot
ax = plt.gca()
# ax.xaxis.set_major_locator(mdates.MinuteLocator(byminute=[0,30], interval=30))

# ax.xaxis_date()
# myFmt = DateFormatter('%H:%M:%S')
# ax.xaxis.set_major_formatter(myFmt)
# ax.xaxis.set_major_locator(SecondLocator(interval=20)) # used to be SecondLocator(0, interval=20)

#To adjust the xlimits a timedelta is needed.
delta = (stop.max() - start.min())/10

plt.yticks(y, captions)
plt.ylim(0, len(intervals) + 1)
plt.xlim(start.min()-delta, stop.max()+delta)
plt.xlabel('Time')
#make the window bigger
plt.gcf().set_size_inches(20, 10)
plt.show()