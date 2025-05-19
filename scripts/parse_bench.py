import re
import sys
import matplotlib.pyplot as plt
from matplotlib.ticker import NullLocator, FixedFormatter, FixedLocator
import os

class Benchmark:
    def __init__(self, name, data, baseline="GeijerImp"):
        self.name = name
        self.data = data
        self.mean = data[baseline]["mean"]
        self.max = data[baseline]["max"]
        self.gets = data[baseline]["gets"]
        self.queue_size = data[baseline]["queue_size"]

    def getTotTimeInSec(self, imp):
        total_time = self.data[imp]["tot"]
        total_time = self.timeToSec(total_time)
        return total_time
    def getCalcTimeInSec(self, imp):
        calc_time = self.data[imp]["calc"]
        calc_time = self.timeToSec(calc_time)
        return calc_time
    def getPrepTimeInSec(self, imp):
        prep_time = self.data[imp]["prep"]
        prep_time = self.timeToSec(prep_time)
        return prep_time 
    def timeToSec(self, time):
        #find the time in seconds
        if "us" in time:
            time = float(time.split("us")[0]) / 1000000
        elif "ms" in time:
            time = float(time.split("ms")[0]) / 1000
        elif "m" in time:
            sec_min = float(time.split("m")[0]) * 60
            sec = float(time.split("m")[1].split("s")[0])
            time = sec_min + sec
        elif "s" in time:
            time = float(time.split("s")[0])
        return time
    
class BenchRun:
    def __init__(self, name, benches):
        self.name = name
        # self.
        # for bench in benches:

    
class Implementation:
    def __init__(self, name, benches):
        self.name = name
        self.gets = {}
        self.means = {}
        self.maxs = {}
        self.tots = {}
        self.calc = {}
        self.prep = {}
        self.tot_speedup = {}
        self.calc_speedup = {}
        self.prep_speedup = {}
        self.mean_error = {}
        self.max_error = {}
        
        for bench in benches:
            self.gets[bench.mean] = bench.data[name]["gets"]
            self.means[bench.mean] = bench.data[name]["mean"]
            self.mean_error[bench.mean] = (abs(bench.data[name]["mean"] - bench.mean) / bench.mean) * 100
            self.maxs[bench.max] = bench.data[name]["max"]
            self.max_error[bench.max] = (abs(bench.data[name]["max"] - bench.max) / bench.max) * 100
            self.tots[bench.mean] = bench.data[name]["tot"] 
            self.calc[bench.mean] = bench.data[name]["calc"]
            self.prep[bench.mean] = bench.data[name]["prep"]
            self.tot_speedup[bench.mean] = bench.data[name]["tot_speedup"]
            self.calc_speedup[bench.mean] = bench.data[name]["calc_speedup"]
            self.prep_speedup[bench.mean] = bench.data[name]["prep_speedup"]

    def __str__(self):
        ret = f"Implementation: {self.name}\n"
        for mean in self.means:
            ret += f"\tGets: {self.gets[mean]}\n"
            ret += f"\tMean: {mean}\n"
            ret += f"\t\t Total: {self.tots[mean]} Speedup: {self.tot_speedup[mean]}\n"
            ret += f"\t\t Calc: {self.calc[mean]} Speedup: {self.calc_speedup[mean]}\n"
            ret += f"\t\t Prep: {self.prep[mean]} Speedup: {self.prep_speedup[mean]}\n"
        return ret

# open the file which is the first argument to this script

def parseFile(file):
    log_file_name = os.path.basename(file.name).split("/")[-1].split(".")[0]
    lines = file.readlines()
    file.close()
    #figure out how many implementations were run
    imp_pattern = re.compile(r"Running bench::(.*)...")
    bench_name_pattern = re.compile(r"-+ ([\S]+) gets: ([\d]+) -+")
    queue_size_pattern = re.compile(r"-i([\d]+)-")
    imps = []
    for line in lines:
        if imp_pattern.match(line) and imp_pattern.match(line).group(1) not in imps:
            imps.append(imp_pattern.match(line).group(1))
        elif bench_name_pattern.match(line):
            break 

    #parse all the benchmarks
    # Define ANSI color codes
    red_pattern = r"\033\[91m"
    green_pattern = r"\033\[92m"
    yellow_pattern = r"\033\[93m"
    white_pattern = r"\033\[0m"
    col_pattern = rf"({red_pattern}|{green_pattern}|{yellow_pattern}|{white_pattern})"
    space_pattern = r"\s+"
    name_pattern = r"bench::(\w+)"
    float_pattern = r"([\d]+\.[\d]+)"
    int_pattern = r"([\d]+)"
    mean_max_pattern = f"{col_pattern}{float_pattern}{space_pattern}{col_pattern}{int_pattern}"

    time_pattern = fr"([\w.]+)"

    data_pattern = f"{col_pattern}{time_pattern} \({float_pattern}\){space_pattern}{time_pattern} \({float_pattern}\){space_pattern}{time_pattern} \({float_pattern}\)" 
    line_pattern = re.compile(f"{name_pattern}{space_pattern}{mean_max_pattern}{space_pattern}{data_pattern}")

    print(f"Parsing {log_file_name} with {len(imps)} implementations")
    benches = []
    i = 0
    while i < len(lines):
        line = lines[i]

        if bench_name_pattern.match(line):
            name = bench_name_pattern.match(line).group(1)
            gets = bench_name_pattern.match(line).group(2)
            queue_size = queue_size_pattern.search(line)
            data = {}
            i += 1
            for j in range(i+1, i + len(imps) + 1):
                line = lines[j]
                if line_pattern.match(line):
                    match = line_pattern.match(line)
                    imp = match.group(1)
                    mean = float(match.group(3))
                    max_val = float(match.group(5))
                    tot = match.group(7)
                    tot_speedup = float(match.group(8))
                    calc = match.group(9)
                    calc_speedup = float(match.group(10))
                    prep = match.group(11)
                    prep_speedup = float(match.group(12))
                    data[imp] = {"imp": imp,
                            "gets": float(gets),
                            "queue_size": int(queue_size.group(1)) if queue_size else 0,
                            "mean": mean, 
                            "max": max_val, 
                            "tot": tot, 
                            "tot_speedup": tot_speedup, 
                            "calc": calc, 
                            "calc_speedup": calc_speedup, 
                            "prep": prep, 
                            "prep_speedup": prep_speedup
                            }
                i = j
            bench = Benchmark(name, data, imps[0])
            benches.append(bench)
        i += 1

    parsed_imps = []
    for imp in imps:
        parsed_imp = Implementation(imp, benches)
        # print(parsed_imp)
        parsed_imps.append(parsed_imp)

    return parsed_imps, benches, log_file_name


def plotSpeedupBenchmarks(parsed_imps, benches, log_file_name, dest_dir):

    print(f"Plotting {log_file_name} with {len(parsed_imps)} implementations")
    #plot all benches, with mean on x axix and speedup on y axis
    speed_fig, speed_axs = plt.subplots(1, 3, figsize=(15, 5))  # 2 row, 3 columns
    error_fig, error_axs = plt.subplots(1, 2, figsize=(15, 5))  # 2 row, 3 columns
    markers = ["o", "x", "s", "D", "^", "v", "<", ">", "p", "P", "*", "h", "H", "+", "X"]
    line_styles = ["-", "--", "-.", ":", "-", "--", "-.", ":", "-", "--", "-.", ":", "-", "--", "-.", ":"]
    for imp in parsed_imps:
        x = []
        y = [[] for i in range(5)]
        for bench in benches:
            x.append(bench.mean)
            y[0].append(bench.data[imp.name]["tot_speedup"])
            y[1].append(bench.data[imp.name]["calc_speedup"])
            y[2].append(bench.data[imp.name]["prep_speedup"])
            y[3].append(imp.mean_error[bench.mean])
            y[4].append(imp.max_error[bench.max])
        speed_axs[0].plot(x, y[0], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        speed_axs[1].plot(x, y[1], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        speed_axs[2].plot(x, y[2], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        error_axs[0].plot(x, y[3], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        error_axs[1].plot(x, y[4], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        markers = markers[1:]
        line_styles = line_styles[1:]
    #log x axis

    for i in range(3):
        # speed_axs[i].set_xscale("log")
        # speed_axs[i].set_yscale("log")
        speed_axs[i].set_ylabel("Speedup")
        speed_axs[i].set_xlabel("Mean relaxation error")

    speed_axs[0].set_title("Total Speedup")
    speed_axs[2].legend()
    speed_axs[1].set_title("Calc Speedup")
    speed_axs[2].set_title("Prep Speedup")
    error_axs[0].set_title("Mean Calculation Error (%)")
    error_axs[1].set_title("Max Calculation Error (%)")
    error_axs[1].legend()
    error_axs[0].set_ylabel("Error %")
    error_axs[0].set_xlabel("Mean relaxation error")
    error_axs[0].set_xscale("log")
    error_axs[1].set_ylabel("Error %")
    error_axs[1].set_xlabel("Mean relaxation error")
    error_axs[1].set_xscale("log")
    ticks = [b.mean for b in benches]
    rounded_labels = [f"{int(tick)}" if tick.is_integer() else f"{tick:.2f}" for tick in ticks]
    for ax in [*speed_axs.flat, *error_axs.flat]:
        ax.set_xticks(ticks)
        ax.set_xticklabels(rounded_labels, rotation=45, fontsize=10)
        ax.tick_params(axis='x', which='minor', bottom=False, labelbottom=False)
    #move legend outside of plot
    for ax in [speed_axs[2], error_axs[1]]:
       
        handles, labels = ax.get_legend_handles_labels()
        # sort both labels and handles by labels
        labels, handles = zip(*sorted(zip(labels, handles), key=lambda t: sort_key(t[0])))
        ax.legend(handles, labels, loc='upper left', bbox_to_anchor=(1, 1))
    #set names on plots
    error_fig.suptitle(f"{log_file_name} Error")
    error_fig.subplots_adjust(bottom=0.2)
    speed_fig.suptitle(f"{log_file_name} Speedup")
    speed_fig.subplots_adjust(bottom=0.2)
    if(dest_dir != ""):
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)
        speed_fig.savefig(f"{dest_dir}{log_file_name}_tp.svg", format='svg')
        error_fig.savefig(f"{dest_dir}{log_file_name}_error.svg", format='svg')

def sort_key(s):
    match = re.search(r'(\D*)(\d*)$', s)  # Splits into text and optional number
    text_part = match.group(1)
    num_part = int(match.group(2)) if match.group(2) else float('-inf')  # Treat non-numeric as smallest
    return (text_part, num_part)
def plotBenchmarks(parsed_imps, benches, log_file_name, dest_dir, queueSize):

    print(f"Plotting {log_file_name} with {len(parsed_imps)} implementations")
    #plot all benches, with mean on x axix and speedup on y axis
    #bigger space between plots
    plt.rcParams.update({'font.size': 15})
    plt.rcParams['axes.titlesize'] = 15
    plt.rcParams['axes.labelsize'] = 15
    plt.rcParams['xtick.labelsize'] = 15
    plt.rcParams['ytick.labelsize'] = 15
    plt.rcParams['legend.fontsize'] = 15
    speed_fig, speed_axs = plt.subplots(1, 3, figsize=(20, 5))  # 2 row, 3 columns
    error_fig, error_axs = plt.subplots(1, 2, figsize=(15, 5))  # 2 row, 3 columns
    markers = ["o", "x", "s", "D", "^", "v", "<", ">", "p", "P", "*", "h", "H", "+", "X"]
    line_styles = ["-", "--", "-.", ":", "-", "--", "-.", ":", "-", "--", "-.", ":", "-", "--", "-.", ":"]
    gets = parsed_imps[0].gets[benches[0].mean]
    for imp in parsed_imps:
        x = []
        y = [[] for i in range(5)]
        for bench in benches:
            if(queueSize):
                x.append(bench.data[imp.name]["queue_size"])
            else:
                x.append(bench.mean)
            y[0].append(gets / float(bench.getTotTimeInSec(imp.name)))
            y[1].append(gets / float(bench.getCalcTimeInSec(imp.name)))
            y[2].append(gets / float(bench.getPrepTimeInSec(imp.name)))
            y[3].append(imp.mean_error[bench.mean])
            y[4].append(imp.max_error[bench.max])
        speed_axs[0].plot(x, y[0], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        speed_axs[1].plot(x, y[1], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        speed_axs[2].plot(x, y[2], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        error_axs[0].plot(x, y[3], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        error_axs[1].plot(x, y[4], label=imp.name, linestyle = line_styles[0], marker = markers[0])
        markers = markers[1:]
        line_styles = line_styles[1:]
    #log x axis

    for i in range(3):
        speed_axs[i].set_xscale("log")
        speed_axs[i].set_yscale("log")
        speed_axs[i].set_ylabel("Dequeues per second")
        if(plotQs):
            speed_axs[i].set_xlabel("Queue size")
        else:
            speed_axs[i].set_xlabel("Mean relaxation error")

    speed_axs[0].set_title("Total Dequeues per second")
    speed_axs[2].legend()
    speed_axs[1].set_title("Calc Dequeues per second")
    speed_axs[2].set_title("Prep Dequeues per second")
    error_axs[0].set_title("Mean Calculation Error (%)")
    error_axs[1].set_title("Max Calculation Error (%)")
    error_axs[1].legend()
    error_axs[0].set_ylabel("Error %")
    error_axs[0].set_xlabel("Mean relaxation error")
    error_axs[0].set_xscale("log")
    error_axs[1].set_ylabel("Error %")
    error_axs[1].set_xlabel("Mean relaxation error")
    error_axs[1].set_xscale("log")
    ticks = [b.mean for b in benches] if not queueSize else [float(b.queue_size) for b in benches]
    rounded_labels = [f"{int(tick)}" if tick.is_integer() else f"{tick:.2f}" for tick in ticks]
    for ax in [*speed_axs.flat, *error_axs.flat]:
        ax.set_xticks(ticks)
        ax.set_xticklabels(rounded_labels, rotation=45, fontsize=15)
        ax.tick_params(axis='x', which='minor', bottom=False, labelbottom=False)
    
    #move legend outside of plot
    for ax in [speed_axs[2], error_axs[1]]:
        handles, labels = ax.get_legend_handles_labels()
        # sort both labels and handles by labels
        labels, handles = zip(*sorted(zip(labels, handles), key=lambda t: sort_key(t[0])))
        ax.legend(handles, labels, loc='upper left', bbox_to_anchor=(1, 1))
    #set names on plots
    # error_fig.suptitle(f"{log_file_name} Error")
    error_fig.subplots_adjust(bottom=0.2)
    error_fig.tight_layout(rect=[0, 0, 1, 1])
    # speed_fig.suptitle(f"{log_file_name} Dequeues per second, {int(gets)} gets")
    speed_fig.subplots_adjust(bottom=0.2)
    speed_fig.tight_layout(rect=[0, 0, 1, 1])

    #save the figures to the given directory
    if(dest_dir != ""):
        if not os.path.exists(dest_dir):
            os.makedirs(dest_dir)
        speed_fig.savefig(f"{dest_dir}{log_file_name}-tp.svg", format='svg')
        error_fig.savefig(f"{dest_dir}{log_file_name}-error.svg", format='svg')


if __name__ == "__main__":
    #if sys.argv contains -d, then parse all files in the given directory
    dest_dir = sys.argv[sys.argv.index("-dest") + 1] if sys.argv.count("-dest") > 0 else ""
    if dest_dir != "":
        dest_idx = sys.argv.index("-dest")
        sys.argv = sys.argv[:dest_idx] + sys.argv[dest_idx + 2:]

    queueSize = False
    if sys.argv.count("-qs") > 0:
        queueSize = True
        sys.argv = [arg for arg in sys.argv if arg != "-qs"]
    

    show = sys.argv.count("-show") > 0
    sys.argv = [arg for arg in sys.argv if arg != "-show"]
    if sys.argv.count("-d") > 0:
        dir = sys.argv[sys.argv.index("-d") + 1]
        files = os.listdir(dir)
        for file in files:
            if file.endswith(".log"):
                file = open(os.path.join(dir, file), "r")
                parsed_imps, benches, log_file_name = parseFile(file)
                plotQs = queueSize or log_file_name.count("size") > 0
                print(f"Plotting {log_file_name} with {len(parsed_imps)} implementations")

                plotBenchmarks(parsed_imps, benches, log_file_name, dest_dir, plotQs)
                # plotSpeedupBenchmarks(parsed_imps, benches, log_file_name)
    else: #assumes that sys.argv are files
        for i in range(1, len(sys.argv)):
            file = open(sys.argv[i], "r")
            parsed_imps, benches, log_file_name = parseFile(file)
            plotQs = queueSize or log_file_name.count("size") > 0
            print(f"Plotting {log_file_name} with {len(parsed_imps)} implementations")


            plotBenchmarks(parsed_imps, benches, log_file_name, dest_dir, plotQs)
            # plotSpeedupBenchmarks(parsed_imps, benches, log_file_name)
    if show:
        plt.show()