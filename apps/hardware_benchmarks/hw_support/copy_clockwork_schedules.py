import sys, json, re, os

fsource = open(sys.argv[1], "r")
flush_file = open(sys.argv[3], "r")
pond_file = open(sys.argv[4], "r")
stencil_valid_file = open(sys.argv[5], "r")

source_lines = fsource.readlines()

new_mem_schedules = {}
new_pond_schedules = {}
new_pond_pipe_schedules = {}

for idx, line in enumerate(source_lines):
    if line.strip() == '"genref":"cgralib.Mem",':
        new_mem_schedules[source_lines[idx-1]] = source_lines[idx+3]
    if line.strip() == '"genref":"cgralib.Pond",':
        new_pond_schedules[source_lines[idx-1]] = source_lines[idx+3]
    if line.strip() == '"modref":"global.Pond",' and 'POND_PIPELINED' in os.environ and os.environ['POND_PIPELINED'] == '1':
        new_pond_pipe_schedules[source_lines[idx-1]] = source_lines[idx+1]

flush_latencies = json.load(flush_file)
pond_latencies = json.load(pond_file)
stencil_valid_latencies = json.load(stencil_valid_file)

for idx, line in enumerate(source_lines):
    new_line = ""
    if line in new_mem_schedules:
        new_line = new_mem_schedules[line]
        mem_name = line.split('"')[1]
        if mem_name in flush_latencies:
            def replace_func(matchobj):
                return str(int(matchobj.group(1)) + flush_latencies[mem_name])
            new_line = re.sub(r'(?<=cycle_starting_addr":\[)(\d*)(?=],"cycle_stride)', replace_func, new_line)

        if mem_name in stencil_valid_latencies:
            def replace_func(matchobj):
                return str(int(matchobj.group(1)) + stencil_valid_latencies[mem_name])
            new_line = re.sub(r'(?<=cycle_starting_addr":\[)(\d*)(?=],"cycle_stride)', replace_func, new_line)
        source_lines[idx+4] = new_line

    if line in new_pond_schedules:
        new_line = new_pond_schedules[line]
        mem_name = line.split('"')[1]
        if mem_name in flush_latencies:
            def replace_func(matchobj):
                return str(int(matchobj.group(1)) + flush_latencies[mem_name])
            new_line = re.sub(r'(?<=cycle_starting_addr":\[)(\d*)(?=],"cycle_stride)', replace_func, new_line)
        source_lines[idx+4] = new_line
        
    if line in new_pond_pipe_schedules:
        new_line = new_pond_pipe_schedules[line]
        pond_name = line.split('"')[1]
        if pond_name in pond_latencies:
            def replace_func(matchobj):
                return str(pond_latencies[pond_name])
            new_line = re.sub(r'(?<="regfile2out_0":{"cycle_starting_addr":\[)(\d*)(?=],"cycle_stride)', replace_func, new_line)
        source_lines[idx+2] = new_line
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    if "-" in new_line:
        print(f"{FAIL}design_top.json has a schedule with a negative number:\n{new_line}\nThis application will probably fail{ENDC}")
        

fsource.close()

fout = open(sys.argv[2], "w")
for line in source_lines:
    fout.write(line)

fout.close()