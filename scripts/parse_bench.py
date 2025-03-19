import re

class Benchmark:
    def __init__(self, name, data):
        self.name = name
        self.data = data
        self.mean = data["GeijerImp"]["mean"]
        self.max = data["GeijerImp"]["max"]
class BenchRun:
    def __init__(self, name, benches):
        self.name = name
        # self.
        # for bench in benches:

    
class Implementation:
    def __init__(self, name, benches):
        self.name = name
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
            ret += f"\tMean: {mean}\n"
            ret += f"\t\t Total: {self.tots[mean]} Speedup: {self.tot_speedup[mean]}\n"
            ret += f"\t\t Calc: {self.calc[mean]} Speedup: {self.calc_speedup[mean]}\n"
            ret += f"\t\t Prep: {self.prep[mean]} Speedup: {self.prep_speedup[mean]}\n"
        return ret

file = open("bench.txt", "r")
lines = file.readlines()
file.close()
#figure out how many implementations were run
imp_pattern = re.compile(r"Running bench::(.*)...")
imps = []
for line in lines:
    if imp_pattern.match(line):
        imps.append(imp_pattern.match(line).group(1))
    else:
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

time_pattern = r"(\w+)"

data_pattern = f"{col_pattern}{time_pattern} \({float_pattern}\){space_pattern}{time_pattern} \({float_pattern}\){space_pattern}{time_pattern} \({float_pattern}\)" 
line_pattern = re.compile(f"{name_pattern}{space_pattern}{mean_max_pattern}{space_pattern}{data_pattern}")
bench_name_pattern = re.compile(r"-+ ([\S]+) -+")


benches = []
i = 0
while i < len(lines):
    line = lines[i]

    if bench_name_pattern.match(line):
        name = bench_name_pattern.match(line).group(1)
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

        bench = Benchmark(name, data)
        benches.append(bench)
    i += 1

parsed_imps = []
for imp in imps:
    parsed_imp = Implementation(imp, benches)
    # print(parsed_imp)
    parsed_imps.append(parsed_imp)


#plot all benches, with mean on x axix and speedup on y axis
import matplotlib.pyplot as plt
fig, axs = plt.subplots(2, 3, figsize=(15, 5))  # 2 row, 3 columns
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
    axs[0, 0].plot(x, y[0], label=imp.name, linestyle = line_styles[0], marker = markers[0])
    axs[0, 1].plot(x, y[1], label=imp.name, linestyle = line_styles[0], marker = markers[0])
    axs[0, 2].plot(x, y[2], label=imp.name, linestyle = line_styles[0], marker = markers[0])
    axs[1, 0].plot(x, y[3], label=imp.name, linestyle = line_styles[0], marker = markers[0])
    axs[1, 1].plot(x, y[4], label=imp.name, linestyle = line_styles[0], marker = markers[0])
    markers = markers[1:]
    line_styles = line_styles[1:]
#log x axis
for i in range(3):
    axs[0, i].set_xscale("log")
    axs[0, i].set_yscale("log")
    axs[0, i].set_ylabel("Speedup")
    axs[0, i].set_xlabel("Mean relaxation error")
axs[0, 0].set_ylabel("Mean calculation error")
axs[0, 0].set_xlabel("Mean relaxation error ")
axs[0, 1].set_ylabel("Max calculation error")
axs[0, 1].set_xlabel("Mean relaxation error")

fig.legend(
    labels=imps,
    loc="center",               # Aligns the legend in the figure
    bbox_to_anchor=(0.87, 0.3), # Adjust this to position it exactly where the empty space is
    fontsize="large"
)
axs[0, 0].set_title("Total Speedup")
axs[0, 1].set_title("Calc Speedup")
axs[0, 2].set_title("Prep Speedup")
axs[1, 0].set_title("Mean Calculation Error (%)")
axs[1, 1].set_title("Max Calculation Error (%)")
plt.tight_layout()
fig.delaxes(axs[1, 2])
plt.show()