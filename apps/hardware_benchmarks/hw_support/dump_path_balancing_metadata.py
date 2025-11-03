import json

# zircon_deq_ResReLU_fp
# path_balancing_metadata = {
#       "balance_lengths": {
#         "p71": 8,
#         "p38": 8,
#         "p2": 4,
#         "p5": 7,
#         "p80": 8,
#         "p86": 4,
#         "p89": 8,
#         "p92": 2,
#         "p8": 7,
#         "p11": 4,
#         "p17": 14,
#         "p14": 4,
#         "p20": 7,
#         "p23": 14,
#         "p29": 4,
#         "p26": 8,
#         "p35": 4,
#         "p44": 7,
#         "p41": 4,
#         "p47": 4,
#         "p50": 7,
#         "p56": 14,
#         "p53": 4,
#         "p59": 4,
#         "p62": 14,
#         "p74": 8,
#         "p65": 7,
#         "p68": 4,
#         "p90": 4,
#         "p9": 2,
#         "p93": 2,
#         "p6": 2,
#         "p15": 7,
#         "p18": 4,
#         "p21": 7,
#         "p24": 2,
#         "p30": 2,
#         "p48": 4,
#         "p54": 4,
#         "p57": 4,
#         "p60": 4,
#         "p66": 4,
#         "p63": 4,
#         "p72": 4,
#         "p75": 2,
#         "p10": 4,
#         "p13": 7,
#         "p19": 4,
#     },
#     "total_stream_lengths": {
#         "p71": 1568,
#         "p38": 1568,
#         "p2": 1568,
#         "p5": 1568,
#         "p80": 1568,
#         "p86": 1568,
#         "p89": 1568,
#         "p92": 1568,
#         "p8": 1568,
#         "p11": 1568,
#         "p17": 1568,
#         "p14": 1568,
#         "p20": 1568,
#         "p23": 1568,
#         "p29": 1568,
#         "p26": 1568,
#         "p35": 1568,
#         "p44": 1568,
#         "p41": 1568,
#         "p47": 1568,
#         "p50": 1568,
#         "p56": 1568,
#         "p53": 1568,
#         "p59": 1568,
#         "p62": 1568,
#         "p74": 1568,
#         "p65": 1568,
#         "p68": 1568,
#         "p90": 1568,
#         "p9": 1568,
#         "p93": 1568,
#         "p6": 1568,
#         "p15": 1568,
#         "p18": 1568,
#         "p21": 1568,
#         "p24": 1568,
#         "p30": 1568,
#         "p48": 1568,
#         "p54": 1568,
#         "p57": 1568,
#         "p60": 1568,
#         "p66": 1568,
#         "p63": 1568,
#         "p72": 1568,
#         "p75": 1568,
#         "p10": 1568,
#         "p13": 1568,
#         "p19": 1568,
#     },
#     "name_to_id": {
#     },
#     "pe_to_pond": {
#         "p71": True,
#         "p38": True,
#         "p2": True,
#         "p5": True,
#         "p80": True,
#         "p86": True,
#         "p89": True,
#         "p92": True,
#         "p8": True,
#         "p11": True,
#         "p17": True,
#         "p14": True,
#         "p20": True,
#         "p23": True,
#         "p29": True,
#         "p26": True,
#         "p35": True,
#         "p44": True,
#         "p41": True,
#         "p47": True,
#         "p50": True,
#         "p56": True,
#         "p53": True,
#         "p59": True,
#         "p62": True,
#         "p74": True,
#         "p65": True,
#         "p68": True,
#         "p90": True,
#         "p9": True,
#         "p93": True,
#         "p6": True,
#         "p15": True,
#         "p18": True,
#         "p21": True,
#         "p24": True,
#         "p30": True,
#         "p48": True,
#         "p54": True,
#         "p57": True,
#         "p60": True,
#         "p66": True,
#         "p63": True,
#         "p72": True,
#         "p75": True,
#         "p10": True,
#         "p13": True,
#         "p19": True,
#     }
# }

# zircon_deq_ResReLU_quant_fp
# path_balancing_metadata = {
#       "balance_lengths": {
#         # Lane 0
#          "p15": 2,
#          "p21": 4,
#          "p19": 4,
#          "p11": 2,

#         # Lane 1
#         "p16": 4,

#         # Lane 2
#         "p5": 4,
#         "p3": 4,
#         "p9": 4,
#         "p6": 4,
#         "p1": 4,
#         "p0": 7,

#         # Lane 3
#         "p4": 4,
#         "p10": 7,
#         "p8": 7,
#         "p2": 4,

#         # Lane 4
#         "p88": 8,
#         "p95": 4,
#         "p92": 4,
#         "p98": 4,
#         "p96": 4,
#         "p90": 4,

#         # Lane 5
#         "p93": 2,
#         "p91": 4,
#         "p97": 4,
#         "p94": 4,
#         "p89": 2,

#         # Lane 6
#         "p99": 8,
#         "p102": 7,
#         "p108": 4,
#         "p105": 4,
#         "p100": 4,

#         # Lane 7
#         "p103": 7,
#         "p109": 4,
#         "p107": 7,
#         "p101": 4,

#         # Lane 8
#         "p110": 2,
#         "p114": 4,
#         "p120": 4,
#         "p112": 4,

#         # Lane 9
#         "p115": 4,
#         "p113": 2,
#         "p119": 2,
#         "p116": 2,
#         "p111": 4,

#         # Lane 10
#         "p121": 2,
#         "p126": 2,
#         "p124": 2,
#         "p127": 2,
#         "p122": 2,

#         # Lane 11
#         "p128": 2,
#         "p125": 2,
#         "p131": 2,
#         "p129": 4,
#         "p123": 4,

#         # Lane 12
#         "p139": 2,
#         "p142": 2,
#         "p140": 4,
#         "p134": 4,

#         # Lane 13
#         "p135": 4,
#         "p141": 2,
#         "p138": 4,

#         # Lane 14
#         "p143": 8,
#         "p146": 4,
#         "p152": 7,
#         "p149": 4,
#         "p144": 4,

#         # Lane 15
#         "p147": 4,
#         "p153": 7,
#         "p151": 7,
#         "p145": 4,

#         # Lane 16
#         "p154": 8,
#         "p162": 8,

#         # Lane 17
#         "p155": 8,

#         # Lane 18
#         "p165": 14,
#         "p173": 14,

#         # Lane 19
#         "p174": 8,
#         "p166": 2,

#         # Lane 20
#         "p33": 8,
#         "p41": 8,

#         # Lane 21
#         "p36": 8,
#         "p42": 2,

#         # Lane 22
#         "p30": 7,

#         # Lane 23
#         "p31": 8,
#         "p23": 2,

#         # Lane 24
#         "p44": 7,
#         "p54": 8,

#         # Lane 25
#         "p53": 7,

#         # Lane 26
#         "p55": 8,
#         "p58": 7,

#         # Lane 27
#         "p59": 7,
#         "p63": 4,

#         # Lane 28
#         "p66": 14,
#         "p74": 7,

#         # Lane 29
#         "p72": 4,

#         # Lane 30
#         "p77": 14,
#         "p86": 4,

#         # Lane 31
#         "p81": 7,
#     },
#     "total_stream_lengths": {
#         # Lane 0
#         "p15": 1568,
#         "p21": 1568,
#         "p19": 1568,
#         "p11": 1568,

#         # Lane 1
#         "p16": 1568,

#         # Lane 2
#         "p5": 1568,
#         "p3": 1568,
#         "p9": 1568,
#         "p6": 1568,
#         "p1": 1568,
#         "p0": 1568,

#         # Lane 3
#         "p4": 1568,
#         "p10": 1568,
#         "p8": 1568,
#         "p2": 1568,

#         # Lane 4
#         "p88": 1568,
#         "p95": 1568,
#         "p92": 1568,
#         "p98": 1568,
#         "p96": 1568,
#         "p90": 1568,

#         # Lane 5
#         "p93": 1568,
#         "p91": 1568,
#         "p97": 1568,
#         "p94": 1568,
#         "p89": 1568,

#         # Lane 6
#         "p99": 1568,
#         "p102": 1568,
#         "p108": 1568,
#         "p105": 1568,
#         "p100": 1568,

#         # Lane 7
#         "p103": 1568,
#         "p109": 1568,
#         "p107": 1568,
#         "p101": 1568,

#         # Lane 8
#         "p110": 1568,
#         "p114": 1568,
#         "p120": 1568,
#         "p112": 1568,

#         # Lane 9
#         "p115": 1568,
#         "p113": 1568,
#         "p119": 1568,
#         "p116": 1568,
#         "p111": 1568,

#         # Lane 10
#         "p121": 1568,
#         "p126": 1568,
#         "p124": 1568,
#         "p127": 1568,
#         "p122": 1568,

#         # Lane 11
#         "p128": 1568,
#         "p125": 1568,
#         "p131": 1568,
#         "p129": 1568,
#         "p123": 1568,

#         # Lane 12
#         "p139": 1568,
#         "p142": 1568,
#         "p140": 1568,
#         "p134": 1568,

#         # Lane 13
#         "p135": 1568,
#         "p141": 1568,
#         "p138": 1568,

#         # Lane 14
#         "p143": 1568,
#         "p146": 1568,
#         "p152": 1568,
#         "p149": 1568,
#         "p144": 1568,

#         # Lane 15
#         "p147": 1568,
#         "p153": 1568,
#         "p151": 1568,
#         "p145": 1568,

#         # Lane 16
#         "p154": 1568,
#         "p162": 1568,

#         # Lane 17
#         "p155": 1568,

#         # Lane 18
#         "p165": 1568,
#         "p173": 1568,

#         # Lane 19
#         "p174": 1568,
#         "p166": 1568,

#         # Lane 20
#         "p33": 1568,
#         "p41": 1568,

#         # Lane 21
#         "p36": 1568,
#         "p42": 1568,

#         # Lane 22
#         "p30": 1568,

#         # Lane 23
#         "p31": 1568,
#         "p23": 1568,

#         # Lane 24
#         "p44": 1568,
#         "p54": 1568,

#         # Lane 25
#         "p53": 1568,

#         # Lane 26
#         "p55": 1568,
#         "p58": 1568,

#         # Lane 27
#         "p59": 1568,
#         "p63": 1568,

#         # Lane 28
#         "p66": 1568,
#         "p74": 1568,

#         # Lane 29
#         "p72": 1568,

#         # Lane 30
#         "p77": 1568,
#         "p86": 1568,

#         # Lane 31
#         "p81": 1568,
#     },
#     "name_to_id": {
#     },
#     "pe_to_pond": {
#         # Lane 0
#         "p15": True,
#         "p21": True,
#         "p19": True,
#         "p11": True,

#         # Lane 1
#         # "p16": False,
#         "p16": True,

#         # Lane 2
#         # "p5": False,
#         "p5": True,
#         "p3": True,
#         "p9": True,
#         "p6": True,
#         "p1": True,
#         "p0": True,

#         # Lane 3
#         "p4": True,
#         "p10": True,
#         "p8": True,
#         "p2": True,

#         # Lane 4
#         "p88": True,
#         "p95": True,
#         "p92": True,
#         "p98": True,
#         "p96": True,
#         "p90": True,

#         # Lane 5
#         "p93": True,
#         "p91": True,
#         "p97": True,
#         "p94": True,
#         "p89": True,

#         # Lane 6
#         "p99": True,
#         "p102": True,
#         "p108": True,
#         "p105": True,
#         "p100": True,

#         # Lane 7
#         "p103": True,
#         "p109": True,
#         "p107": True,
#         "p101": True,

#         # Lane 8
#         "p110": True,
#         "p114": True,
#         "p120": True,
#         "p112": True,

#         # Lane 9
#         "p115": True,
#         "p113": True,
#         "p119": True,
#         "p116": True,
#         "p111": True,

#         # Lane 10
#         "p121": True,
#         "p126": True,
#         "p124": True,
#         "p127": True,
#         "p122": True,

#         # Lane 11
#         # "p128": False,
#         "p128": True,
#         "p125": True,
#         "p131": True,
#         "p129": True,
#         "p123": True,

#         # Lane 12
#         "p139": True,
#         "p142": True,
#         "p140": True,
#         "p134": True,

#         # Lane 13
#         "p135": True,
#         "p141": True,
#         "p138": True,

#         # Lane 14
#         "p143": True,
#         "p146": True,
#         "p152": True,
#         "p149": True,
#         "p144": True,

#         # Lane 15
#         "p147": True,
#         "p153": True,
#         "p151": True,
#         "p145": True,

#         # Lane 16
#         "p154": True,
#         "p162": True,

#         # Lane 17
#         "p155": True,

#         # Lane 18
#         "p165": True,
#         "p173": True,

#         # Lane 19
#         "p174": True,
#         "p166": True,

#         # Lane 20
#         "p33": True,
#         "p41": True,

#         # Lane 21
#         "p36": True,
#         "p42": True,

#         # Lane 22
#         "p30": True,

#         # Lane 23
#         "p31": True,
#         "p23": True,

#         # Lane 24
#         "p44": True,
#         "p54": True,

#         # Lane 25
#         "p53": True,

#         # Lane 26
#         "p55": True,
#         "p58": True,

#         # Lane 27
#         "p59": True,
#         "p63": True,

#         # Lane 28
#         "p66": True,
#         "p74": True,

#         # Lane 29
#         "p72": True,

#         # Lane 30
#         "p77": True,
#         "p86": True,

#         # Lane 31
#         "p81": True,
#     }
# }


path_balancing_metadata = {
      "balance_lengths": {
         "p3": 8,
    },
    "total_stream_lengths": {
        "p3": 1024,
    },
    "name_to_id": {
    },
    "pe_to_pond": {
        "p3": False,
    }
}


# # zircon_deq_resReLU_quant_fp_V1
# path_balancing_metadata = {
#     "balance_lengths": {
#         "p11": 2,
#         "p0": 8,
#         "p88": 14,
#         "p99": 8,
#         "p110": 2,
#         "p121": 2,
#         "p143": 8,
#         "p154": 8,
#         "p165": 14,
#         "p33": 8,
#         "p44": 7,
#         "p55": 8,
#         "p66": 14,
#         "p77": 14,
#         "p111": 2,

#         "p16": 4,
#         "p5": 4,
#     },
#     "total_stream_lengths": {
#         "p11": 1568,
#         "p0": 1568,
#         "p88": 1568,
#         "p99": 1568,
#         "p110": 1568,
#         "p121": 1568,
#         "p143": 1568,
#         "p154": 1568,
#         "p165": 1568,
#         "p33": 1568,
#         "p44": 1568,
#         "p55": 1568,
#         "p66": 1568,
#         "p77": 1568,
#         "p111": 1568,

#         "p16": 1568,
#         "p5": 1568,
#     },
#     "name_to_id": {

#     },
#     "pe_to_pond": {
#         "p11": True,
#         "p0": True,
#         "p88": True,
#         "p99": True,
#         "p110": True,
#         "p121": True,
#         "p143": True,
#         "p154": True,
#         "p165": True,
#         "p33": True,
#         "p44": True,
#         "p55": True,
#         "p66": True,
#         "p77": True,
#         "p111": True,

#         "p16": True,
#         "p5": True,

#     }
# }

# # zircon_deq_q_res_relu_fp
# path_balancing_metadata = {
#     "balance_lengths": {
#         # Lane 0
#         "p61": 14,

#         # Lane 1
#         "p62": 14,

#         # Lane 2
#         "p68": 8,

#         # Lane 3
#         "p69": 8,

#         # Lane 4
#         "p5": 4,

#         # Lane 5
#         "p6": 4,
#         "p4": 4,

#         # Lane 6
#         "p12": 8,

#         # Lane 7
#         "p13": 8,

#         # Lane 8
#         "p75": 8,

#         # Lane 9
#         "p76": 8,

#         # Lane 10
#         "p82": 7,

#         # Lane 11
#         "p83": 4,

#         # Lane 14
#         "p96": 7,

#         # Lane 15
#         "p97": 7,

#         # Lane 16
#         "p103": 8,

#         # Lane 17
#         "p104": 8,

#         # Lane 18
#         "p110": 7,

#         # Lane 19
#         "p111": 7,

#         # Lane 20
#         "p19": 4,

#         # Lane 23
#         "p27": 4,

#         # Lane 27
#         "p41": 4,

#         # Lane 29
#         "p48": 4,

#         # Lane 30
#         "p54": 2,
#     },
#     "total_stream_lengths": {
#           # Lane 0
#         "p61": 6272,

#         # Lane 1
#         "p62": 6272,

#         # Lane 2
#         "p68": 6272,

#         # Lane 3
#         "p69": 6272,

#         # Lane 4
#         "p5": 6272,

#         # Lane 5
#         "p6": 6272,
#         "p4": 6272,

#         # Lane 6
#         "p12": 6272,

#         # Lane 7
#         "p13": 6272,

#         # Lane 8
#         "p75": 6272,

#         # Lane 9
#         "p76": 6272,

#         # Lane 10
#         "p82": 6272,

#         # Lane 11
#         "p83": 6272,

#         # Lane 14
#         "p96": 6272,

#         # Lane 15
#         "p97": 6272,

#         # Lane 16
#         "p103": 6272,

#         # Lane 17
#         "p104": 6272,

#         # Lane 18
#         "p110": 6272,

#         # Lane 19
#         "p111": 6272,

#         # Lane 20
#         "p19": 6272,

#         # Lane 23
#         "p27": 6272,

#         # Lane 27
#         "p41": 6272,

#         # Lane 29
#         "p48": 6272,

#         # Lane 30
#         "p54": 6272,

#     },
#     "name_to_id": {
#     },
#     "pe_to_pond": {
#         # Lane 0
#         "p61": True,

#         # Lane 1
#         "p62": True,

#         # Lane 2
#         "p68": True,

#         # Lane 3
#         "p69": True,

#         # Lane 4
#         "p5": True,

#         # Lane 5
#         "p6": True,
#         "p4": True,

#         # Lane 6
#         "p12": True,

#         # Lane 7
#         "p13": True,

#         # Lane 8
#         "p75": True,

#         # Lane 9
#         "p76": True,

#         # Lane 10
#         "p82": True,

#         # Lane 11
#         "p83": True,

#         # Lane 14
#         "p96": True,

#         # Lane 15
#         "p97": True,

#         # Lane 16
#         "p103": True,

#         # Lane 17
#         "p104": True,

#         # Lane 18
#         "p110": True,

#         # Lane 19
#         "p111": True,

#         # Lane 20
#         "p19": True,

#         # Lane 23
#         "p27": True,

#         # Lane 27
#         "p41": True,

#         # Lane 29
#         "p48": True,

#         # Lane 30
#         "p54": True,
#     }
# }

# Read the name_to_id mapping from design.packed
packed_filepath = "/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/pointwise_mu_io/bin/design.packed"
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
layer = "submodule_15"
# output_filepath = f"/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/pointwise_mu_io/path_balancing_configs/{model}-{layer}_path_balancing.json"
output_filepath = f"/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/pointwise_mu_io/path_balancing_configs/path_balancing.json"
with open(output_filepath, "w") as f:
    json.dump(path_balancing_metadata, f, indent=4)