import json
import os
import sys

config_path = sys.argv[1] # "cgra_config.json"
output_path = sys.argv[2] # "input_cgra.pgm"

with open(config_path, "r") as read_file:
    data = json.load(read_file)
    inputs = data["IOs"]["inputs"]
    default_byte = b'\x11'

    input_files = []
    input_length = 0
    input_bw = 0
    bitwidth = inputs[0]["bitwidth"]
    for input in inputs:
        file_name = input["name"]
        file_path = input["datafile"]
        os.system(f"make {file_path}")
        assert (input["bitwidth"]==8) or (input["bitwidth"]==16)
        assert input["bitwidth"] == bitwidth
        assert os.path.exists(file_path), f"Could not find {file_path}"
        input_files.append(file_path)
        input_size = 1
        for num in input["shape"]:
            input_size = input_size * num

        file_size = os.path.getsize(file_path)
        computed_size = input_size * bitwidth//8
        #assert file_size == computed_size, f"{file_name}: filesize={file_size}  !=  computedsize={computed_size}"
        input_length = max(input_length, input_size // input["pixels_per_cycle"])
        input_bw = input_bw + input["pixels_per_cycle"]

    print(f"Writing file length {input_length} with bw={bitwidth} and {input_bw} bytes/cycles")

    with open(output_path, "w") as write_file:
        write_file.write("P5\n")
        write_file.write(str(input_bw) + " " + str(input_length) + "\n")
        max_value = 2**bitwidth - 1
        write_file.write(str(max_value) + "\n")
        
    filedata = {input["name"]: open(input["datafile"], 'rb') for input in inputs}

    with open(output_path, "ab") as write_file:
        for i in range(input_length):
            for input in inputs:
                name = input["name"]
                datafile = filedata[input["name"]]
                read_size = input["pixels_per_cycle"] * input["bitwidth"] // 8
                data = datafile.read(read_size)

                if data:
                    #print(f"Writing {read_size} bytes from {name}")
                    write_file.write(data)
                else:
                    #print(f"Writing token bytes for {name}")
                    write_file.write(default_byte * read_size)
                #print(f"Read {read_size} bytes from {name}")

    for file in filedata.values():
        file.close()

    print(f"Wrote to input_cgra.pgm")
    
