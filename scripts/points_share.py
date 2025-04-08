import pandas as pd

folder = '../semantic-relaxation-dcbo/results/threads/'
num_threads = 2

def my_share(folder, thread_id):
    # Read the start times from the file
    puts = pd.read_csv(
        folder + f'thread_{thread_id}_puts.txt',
        sep='\s+',
        header=None,
        names=['timestamp_start', 'id']
    )

    # Read the end times from the file
    gets = pd.read_csv(
        folder + f'thread_{thread_id}_gets.txt',
        sep='\s+',
        header=None,
        names=['timestamp_end', 'id']
    )

    # Merge starts and ends on 'id'
    points = pd.merge(
        puts,
        gets,
        on='id',
        how='inner'  # Use left join to keep all gets
    )
    
    print(f"Thread {thread_id} got {len(points) / len(gets) * 100:.2f}% of its own points")


for i in range(num_threads):
    my_share(folder, i)
