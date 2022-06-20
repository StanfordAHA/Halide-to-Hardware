import sys, json, re, os

fsource = open(sys.argv[1], "r")
fdest = open(sys.argv[2], "r")
flush_file = open(sys.argv[3], "r")
pond_file = open(sys.argv[4], "r")

source_lines = fsource.readlines()
dest_lines = source_lines

new_schedules = {}
new_pond_schedules = {}

for idx, line in enumerate(source_lines):
    if line.strip() == '"modref":"global.MEM",' or line.strip() == '"modref":"global.Pond",':
        new_schedules[source_lines[idx-1]] = source_lines[idx+1]
    if line.strip() == '"modref":"global.Pond",' and 'POND_PIPELINED' in os.environ and os.environ['POND_PIPELINED'] == '1':
        new_pond_schedules[source_lines[idx-1]] = source_lines[idx+1]

flush_latencies = json.load(flush_file)
pond_latencies = json.load(pond_file)

for idx, line in enumerate(dest_lines):
    if line in new_schedules:
        new_line = new_schedules[line]
        mem_name = line.split('"')[1]
        if mem_name in flush_latencies:
            def replace_func(matchobj):
                return str(int(matchobj.group(1)) - flush_latencies[mem_name])
            new_line = re.sub(r'(?<=cycle_starting_addr":\[)(\d*)(?=],"cycle_stride)', replace_func, new_line)
        dest_lines[idx+2] = new_line
    if line in new_pond_schedules:
        new_line = new_pond_schedules[line]
        pond_name = line.split('"')[1]
        if pond_name in pond_latencies:
            def replace_func(matchobj):
                return str(pond_latencies[pond_name])
            new_line = re.sub(r'(?<="regfile2out_0":{"cycle_starting_addr":\[)(\d*)(?=],"cycle_stride)', replace_func, new_line)
        dest_lines[idx+2] = new_line

fsource.close()
fdest.close()

fout = open(sys.argv[2], "w")
for line in dest_lines:
    fout.write(line)

fout.close()
