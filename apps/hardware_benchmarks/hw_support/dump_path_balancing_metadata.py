import json


# path_balancing_metadata = {
#     "balance_lengths": {
#         "p37": 5,
#         "p35": 4,
#         "p41": 4,
#         "p39": 3,
#         "p45": 2,
#         "p43": 1,
#         "p51": 1,
#         "p36": 3,
#         "p40": 3,
#         "p44": 3,
#         "p50": 3,
#         "p47": 5,
#         "p25": 3,
#         "p3": 6,
#         "p1": 4,
#         "p55": 3,
#         "p53": 1,
#         "p59": 2,
#         "p0": 1,
#     },
#     "total_stream_lengths": {
#         "p37": 1024,
#         "p35": 1024,
#         "p41": 1024,
#         "p39": 1024,
#         "p45": 1024,
#         "p43": 1024,
#         "p51": 1024,
#         "p36": 1024,
#         "p40": 1024,
#         "p44": 1024,
#         "p50": 1024,
#         "p47": 1024,
#         "p25": 1024,
#         "p3": 1024,
#         "p1": 1024,
#         "p55": 1024,
#         "p53": 1024,
#         "p59": 1024,
#         "p0": 1024,
#     },
#     "name_to_id": {
#     }
# }


path_balancing_metadata = {
    "balance_lengths": {
        "p37": 5,
        "p35": 4,
        "p41": 4,
        "p39": 3,
        "p45": 2,
        "p43": 1,
        "p51": 1,
        "p47": 5,
        "p25": 3,
        "p3": 6,
        "p1": 4,
        "p55": 3,
        "p53": 1,
        "p59": 2,
        "p19": 6,
        "p17": 4,
        "p23": 5,
        "p21": 3,
        "p29": 3,
        "p27": 1,
        "p33": 2,
        "p63": 6,
        "p61": 4,
        "p7": 5,
        "p5": 3,
        "p11": 3,
        "p9": 1,
        "p15": 2,
    },
    "total_stream_lengths": {
        "p37": 1024,
        "p35": 1024,
        "p41": 1024,
        "p39": 1024,
        "p45": 1024,
        "p43": 1024,
        "p51": 1024,
        "p47": 1024,
        "p25": 1024,
        "p3": 1024,
        "p1": 1024,
        "p55": 1024,
        "p53": 1024,
        "p59": 1024,
        "p19": 1024,
        "p17": 1024,
        "p23": 1024,
        "p21": 1024,
        "p29": 1024,
        "p27": 1024,
        "p33": 1024,
        "p63": 1024,
        "p61": 1024,
        "p7": 1024,
        "p5": 1024,
        "p11": 1024,
        "p9": 1024,
        "p15": 1024,
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



with open(f"/aha/path_balancing.json", "w") as f:
    json.dump(path_balancing_metadata, f, indent=4)