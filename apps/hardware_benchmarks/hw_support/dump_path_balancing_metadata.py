import json

path_balancing_metadata = {
      "balance_lengths": {
        "p37": 4,
        "p35": 4,
        "p41": 4,
        "p39": 2,
        "p45": 2,
        "p43": 2,
        "p51": 2,
        "p47": 4,
        "p25": 2,
        "p3": 7,
        "p1": 4,
        "p55": 2,
        "p53": 2,
        "p59": 2,
        "p19": 7,
        "p17": 4,
        "p23": 4,
        "p21": 2,
        "p29": 2,
        "p27": 2,
        "p33": 2,
        "p63": 7,
        "p61": 4,
        "p7": 4,
        "p5": 2,
        "p11": 2,
        "p9": 2,
        "p15": 2
    },
    "total_stream_lengths": {
        "p37": 1568,
        "p35": 1568,
        "p41": 1568,
        "p39": 1568,
        "p45": 1568,
        "p43": 1568,
        "p51": 1568,
        "p47": 1568,
        "p25": 1568,
        "p3": 1568,
        "p1": 1568,
        "p55": 1568,
        "p53": 1568,
        "p59": 1568,
        "p19": 1568,
        "p17": 1568,
        "p23": 1568,
        "p21": 1568,
        "p29": 1568,
        "p27": 1568,
        "p33": 1568,
        "p63": 1568,
        "p61": 1568,
        "p7": 1568,
        "p5": 1568,
        "p11": 1568,
        "p9": 1568,
        "p15": 1568
    },
    "name_to_id": {
    }
}

# Read the name_to_id mapping from design.packed
packed_filepath = "/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/zircon_residual_relu_fp/bin/design.packed"
with open(packed_filepath, "r") as f:
    id_to_name_start = False
    for line in f:
        if line.startswith("ID to Names:"):
            id_to_name_start = True
            continue
        if id_to_name_start:
            if line.strip():
                parts = line.strip().split(": ")
                id = parts[0]
                name = parts[1]
                if id in path_balancing_metadata["balance_lengths"]:
                    path_balancing_metadata["name_to_id"][name] = id
            else:
                break


model = "resnet18"
layer = "submodule_11"
with open(f"/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/zircon_residual_relu_fp/path_balancing_configs/{model}-{layer}_path_balancing.json", "w") as f:
    json.dump(path_balancing_metadata, f, indent=4)