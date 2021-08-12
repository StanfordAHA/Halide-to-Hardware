import random
import sys
from PIL import Image, ImageDraw
from archipelago.io import load_routing_result
from pycyclone.io import load_placement
import math
import os

# Usage: python visualize_pnr.py <app_dir> <width> <height>
#  app_dir should contain design.place and design.route
#  app_dir should not end in /
# install:
#  pip3 install image

def arrowedLine(im, ptA, ptB, width=1, color=(0,255,0)):
    """Draw line from ptA to ptB with arrowhead at ptB"""
    # Get drawing context
    draw = ImageDraw.Draw(im)
    # Draw the line without arrows
    draw.line((ptA,ptB), width=width, fill=color)

    # Now work out the arrowhead
    # = it will be a triangle with one vertex at ptB
    # - it will start at 95% of the length of the line
    # - it will extend 8 pixels either side of the line
    x0, y0 = ptA
    x1, y1 = ptB
    # Now we can work out the x,y coordinates of the bottom of the arrowhead triangle
    xb = 0*(x1-x0)+x0
    yb = 0*(y1-y0)+y0

    # Work out the other two vertices of the triangle
    # Check if line is vertical
    if x0==x1:
       vtx0 = (xb-3, yb)
       vtx1 = (xb+3, yb)
    # Check if line is horizontal
    elif y0==y1:
       vtx0 = (xb, yb+3)
       vtx1 = (xb, yb-3)
    else:
       alpha = math.atan2(y1-y0,x1-x0)-90*math.pi/180
       a = 8*math.cos(alpha)
       b = 8*math.sin(alpha)
       vtx0 = (xb+a, yb+b)
       vtx1 = (xb-a, yb-b)

    #draw.point((xb,yb), fill=(255,0,0))    # DEBUG: draw point of base in red - comment out draw.polygon() below if using this line
    #im.save('DEBUG-base.png')              # DEBUG: save

    # Now draw the arrowhead triangle
    draw.polygon([vtx0, vtx1, ptB], fill=color)
    return im

if os.path.exists(f"{sys.argv[1]}/design.route"):
    raw_routing_result = load_routing_result(f"{sys.argv[1]}/design.route")
else:
    raw_routing_result = {}
raw_placement_result = load_placement(f"{sys.argv[1]}/design.place")

scale = 60

color_index = "imoprcdI"
color_palette = [(166, 206, 227),
            (31, 120, 180),
            (178, 223, 138),
            (51, 160, 44),
            (251, 154, 153),
            (227, 26, 28),
            (253, 191, 111),
            (255, 127, 0),
            (202, 178, 214),
            (106, 61, 154),
            (255, 255, 153),
            (177, 89, 40)]

height = int(sys.argv[3])
width = int(sys.argv[2])
img_width = width * scale
img_height = height * scale


port_map = {}
port_map["stencil_valid"] = "MEM"
port_map["data_out_0"] = "MEM"
port_map["data_out_1"] = "MEM"
port_map["data_in_0"] = "MEM"
port_map["data_in_1"] = "MEM"
port_map["flush"] = "MEM"
port_map["f2io_1"] = "IO1"
port_map["io2f_1"] = "IO1"
port_map["f2io_16"] = "IO16"
port_map["io2f_16"] = "IO16"

routes_list = []

for edge_id, routes in raw_routing_result.items():

    route = {}
    route["edge_id"] = edge_id
    segment_list = []
    route["segments"] = segment_list
    track1 = None
    route["end_list"] = []
    for segments in routes:
        # print(segments[0])
        if segments[0][1] in port_map:
            route["start"] = port_map[segments[0][1]]
        elif segments[0][0] != "SB":
            route["start"] = "PE"

        if segments[-1][1] in port_map:
            route["end"] = port_map[segments[-1][1]]
        elif segments[-1][0] != "SB":
            route["end"] = "PE"


        for seg1 in segments:
            if seg1[0] == "SB":
                track1, x1, y1, side, io_, bit_width = seg1[1:]
            elif seg1[0] == "PORT":
                port_name, x1, y1, bit_width = seg1[1:]
            elif seg1[0] == "REG":
                reg_name, track1, x1, y1, bit_width = seg1[1:]
            elif seg1[0] == "RMUX":
                rmux_name, x1, y1, bit_width = seg1[1:]

            if seg1 == segments[0]:
                route["start_point"] = (x1, y1)
            if seg1 == segments[-1]:
                route["end_list"].append((x1, y1))
            
            route["bit_width"] = bit_width
            
            if len(route["segments"]) > 0:
                last_x = route["segments"][-1]['x']
                last_y = route["segments"][-1]['y']

                if not (last_x == x1 and last_y == y1):
                    # if track1 == None:
                    #     breakpoint()
                    #     track1 = last_track
                    segment = {}
                    segment['x'] = x1
                    segment['y'] = y1
                    segment['track'] = track1
                    route["segments"].append(segment)
            else:
                segment = {}
                segment['x'] = x1
                segment['y'] = y1
                segment['track'] = track1
                route["segments"].append(segment)

        if route["segments"][0]['track'] == None:
            route["segments"][0]['track'] = route["segments"][1]['track']
    routes_list.append(route)

unordered_routes = routes_list.copy()
ordered_routes = []
ordered_routes.append([])
visited_points = []

for route in routes_list.copy():
    if route["start"] == "IO16":
        ordered_routes[0].append(route)
        routes_list.remove(route)
        for end_pts in route["end_list"]:
            visited_points.append(end_pts)

routes_idx = 1
routes_list_len = len(routes_list)
while True:
    ordered_routes.append([])
    visited_points_new = []
    for route in routes_list.copy():
        if route["start_point"] in visited_points:
            ordered_routes[routes_idx].append(route)
            routes_list.remove(route)
            
            for end_pts in route["end_list"]:
                visited_points_new.append(end_pts)
            
    
    if len(routes_list) == routes_list_len:
        for route in routes_list.copy():
            ordered_routes[routes_idx].append(route)
        break
    routes_idx += 1
    visited_points += visited_points_new
    routes_list_len = len(routes_list)


frames = []
for frame_idx, routes in enumerate(ordered_routes):
    used_tiles = set()
    im = Image.new("RGBA", (img_width, img_height), "BLACK")
    draw = ImageDraw.Draw(im)
    draw.rectangle((0, img_width, 0, img_height), fill = (0, 0, 0, 255))
    for i in range(0, height + 1):
        # horizontal lines
        draw.line((0, i * scale, img_width, i * scale),
                fill=(255, 255, 255), width=1)
                
    for i in range(0, width + 1):
        # vertical lines
        draw.line((i * scale, 0, i * scale, img_height),
                fill=(255, 255, 255), width=1)

    pos_set = set()
    board_pos = raw_placement_result
    blk_id_list = list(board_pos.keys())
    blk_id_list.sort(key=lambda x: 0 if x[0] == "r" or x[0] == "i" else 1)
    for blk_id in blk_id_list:
        pos = board_pos[blk_id]
        index = color_index.index(blk_id[0])
        color = color_palette[index]
        # if blk_id in changed_pe:
        #     color = color_palette[color_index.index("r")]
        # if blk_id[0] == "r":
        #     assert pos not in pos_set
        #     pos_set.add(pos)
        #     # pos = pos[0] + 0.25, pos[1]
        #     width_frac = 1
        # else:

        width_frac = 1
        size = scale - 1
        width = size * width_frac
        shrink = scale/5 
        x, y = pos

        # if (x,y) not in used_tiles:
        color = tuple([math.floor(x/3) for x in color])

        draw.rectangle((x * scale + 1 + shrink, y * scale + 1 + shrink, x * scale + width - shrink,
                        y * scale + size - shrink), fill=color)


    for route in routes:
        if not (route["bit_width"] == 16):
            continue
        # if not (route["end"] == "PE"):
        #     continue

        print(route)
        color = lambda : (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), 255)
        route_color = color()
        for idx, seg1 in enumerate(route["segments"]):
            if len(route["segments"]) > idx + 1:
                seg2 = route["segments"][idx + 1]
            else:
                continue
            x1 = seg1['x']
            y1 = seg1['y']
            x2 = seg2['x']
            y2 = seg2['y']
            track = seg2['track']
            track4 = seg1['track']

            if x1 != x2 and y1 != y2:
                continue
            end_offset = 0.5
            start_offset = 0.5

            if len(route["segments"]) > idx + 2:
                seg3 = route["segments"][idx + 2]
                track3 = seg3['track']

                change_dir = not ((x1 == x2 == seg3['x']) or (y1 == y2 == seg3['y']))

                # Assuming if route changes track its changing direction too
                if track != track3 or change_dir:
                    end_offset = (track3 / int(sys.argv[3])) * 0.6 + 0.2

            change_dir = False
            if idx > 0:
                change_dir = not ((x1 == x2 == route["segments"][idx - 1]['x']) or (y1 == y2 == route["segments"][idx - 1]['y']))


            # Assuming if route changes track its changing direction too
            if track != track4 or change_dir:
                start_offset = (track4 / int(sys.argv[3])) * 0.6 + 0.2


            track_offset = (track / int(sys.argv[3])) * 0.6

            if x1 == x2:
                # Vertical trace
                # draw.line(((x1 + 0.2 + track_offset) * scale, (y1 + start_offset) * scale, (x2 + 0.2 + track_offset) * scale, (y2 + end_offset) * scale), fill=route_color, width=2)
                im = arrowedLine(im, ((x1 + 0.2 + track_offset) * scale, (y1 + start_offset) * scale), ((x2 + 0.2 + track_offset) * scale, (y2 + end_offset) * scale), color=route_color, width=1)
            else:
                # Horizontal trace
                # draw.line(((x1 + start_offset) * scale, (y1 + 0.2 + track_offset) * scale, (x2 + end_offset) * scale, (y2 + 0.2 + track_offset) * scale), fill=route_color, width=2)
                im = arrowedLine(im,((x1 + start_offset) * scale, (y1 + 0.2 + track_offset) * scale), ((x2 + end_offset) * scale, (y2 + 0.2 + track_offset) * scale), color=route_color, width=1)

        used_tiles.add(route["start_point"])
        for end_pts in route["end_list"]:
            used_tiles.add(end_pts)


    pos_set = set()
    board_pos = raw_placement_result
    blk_id_list = list(board_pos.keys())
    blk_id_list.sort(key=lambda x: 0 if x[0] == "r" or x[0] == "i" else 1)
    for blk_id in blk_id_list:
        pos = board_pos[blk_id]
        index = color_index.index(blk_id[0])
        color = color_palette[index]
        # if blk_id in changed_pe:
        #     color = color_palette[color_index.index("r")]
        # if blk_id[0] == "r":
        #     assert pos not in pos_set
        #     pos_set.add(pos)
        #     # pos = pos[0] + 0.25, pos[1]
        #     width_frac = 1
        # else:

        width_frac = 1
        size = scale - 1
        width = size * width_frac
        shrink = scale/5 
        x, y = pos

        if (x,y) in used_tiles:
            draw.rectangle((x * scale + 1 + shrink, y * scale + 1 + shrink, x * scale + width - shrink,
                        y * scale + size - shrink), fill=color)


            

    frames.append(im)
    im.save(f'route_{frame_idx}.png', format='PNG')

used_tiles = set()
im = Image.new("RGBA", (img_width, img_height), "BLACK")
draw = ImageDraw.Draw(im)
draw.rectangle((0, img_width, 0, img_height), fill = (0, 0, 0, 255))
for i in range(0, height + 1):
    # horizontal lines
    draw.line((0, i * scale, img_width, i * scale),
            fill=(255, 255, 255), width=1)
            
for i in range(0, width + 1):
    # vertical lines
    draw.line((i * scale, 0, i * scale, img_height),
            fill=(255, 255, 255), width=1)


pos_set = set()
board_pos = raw_placement_result
blk_id_list = list(board_pos.keys())
blk_id_list.sort(key=lambda x: 0 if x[0] == "r" or x[0] == "i" else 1)
for blk_id in blk_id_list:
    pos = board_pos[blk_id]
    index = color_index.index(blk_id[0])
    color = color_palette[index]
    # if blk_id in changed_pe:
    #     color = color_palette[color_index.index("r")]
    # if blk_id[0] == "r":
    #     assert pos not in pos_set
    #     pos_set.add(pos)
    #     # pos = pos[0] + 0.25, pos[1]
    #     width_frac = 1
    # else:

    width_frac = 1
    size = scale - 1
    width = size * width_frac
    shrink = scale/5 
    x, y = pos

    # if (x,y) not in used_tiles:
    color = tuple([math.floor(x/3) for x in color])

    draw.rectangle((x * scale + 1 + shrink, y * scale + 1 + shrink, x * scale + width - shrink,
                    y * scale + size - shrink), fill=color)



num_switchboxes = 0
for route in unordered_routes:
    # if not (route["edge_id"] == 'e8'):
    #     continue
    # if not (route["start"] == "MEM" and route["end"] == "PE"):
    #     continue

    # print(route)
    color = lambda : (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255), 255)
    route_color = color()
    for idx, seg1 in enumerate(route["segments"]):
        if len(route["segments"]) > idx + 1:
            seg2 = route["segments"][idx + 1]
        else:
            continue
        x1 = seg1['x']
        y1 = seg1['y']
        x2 = seg2['x']
        y2 = seg2['y']
        track = seg2['track']
        track4 = seg1['track']

        if x1 != x2 and y1 != y2:
            continue

        num_switchboxes += 1
        end_offset = 0.5
        start_offset = 0.5

        if len(route["segments"]) > idx + 2:
            seg3 = route["segments"][idx + 2]
            track3 = seg3['track']

            change_dir = not ((x1 == x2 == seg3['x']) or (y1 == y2 == seg3['y']))

            # Assuming if route changes track its changing direction too
            if track != track3 or change_dir:
                end_offset = (track3 / int(sys.argv[3])) * 0.6 + 0.2

        change_dir = False
        if idx > 0:
            change_dir = not ((x1 == x2 == route["segments"][idx - 1]['x']) or (y1 == y2 == route["segments"][idx - 1]['y']))


        # Assuming if route changes track its changing direction too
        if track != track4 or change_dir:
            start_offset = (track4 / int(sys.argv[3])) * 0.6 + 0.2


        track_offset = (track / int(sys.argv[3])) * 0.6

        if x1 == x2:
            # Vertical trace
            # draw.line(((x1 + 0.2 + track_offset) * scale, (y1 + start_offset) * scale, (x2 + 0.2 + track_offset) * scale, (y2 + end_offset) * scale), fill=route_color, width=2)
            im = arrowedLine(im, ((x1 + 0.2 + track_offset) * scale, (y1 + start_offset) * scale), ((x2 + 0.2 + track_offset) * scale, (y2 + end_offset) * scale), color=route_color, width=1)
        else:
            # Horizontal trace
            # draw.line(((x1 + start_offset) * scale, (y1 + 0.2 + track_offset) * scale, (x2 + end_offset) * scale, (y2 + 0.2 + track_offset) * scale), fill=route_color, width=2)
            im = arrowedLine(im,((x1 + start_offset) * scale, (y1 + 0.2 + track_offset) * scale), ((x2 + end_offset) * scale, (y2 + 0.2 + track_offset) * scale), color=route_color, width=1)


    used_tiles.add(route["start_point"])
    for end_pts in route["end_list"]:
        used_tiles.add(end_pts)
print(f"This design uses {num_switchboxes} switchboxes")
    
pos_set = set()
board_pos = raw_placement_result
blk_id_list = list(board_pos.keys())
blk_id_list.sort(key=lambda x: 0 if x[0] == "r" or x[0] == "i" else 1)
for blk_id in blk_id_list:
    pos = board_pos[blk_id]
    index = color_index.index(blk_id[0])
    color = color_palette[index]
    # if blk_id in changed_pe:
    #     color = color_palette[color_index.index("r")]
    # if blk_id[0] == "r":
    #     assert pos not in pos_set
    #     pos_set.add(pos)
    #     # pos = pos[0] + 0.25, pos[1]
    #     width_frac = 1
    # else:

    width_frac = 1
    size = scale - 1
    width = size * width_frac
    shrink = scale/5 
    x, y = pos

    if (x,y) in used_tiles:
        draw.rectangle((x * scale + 1 + shrink, y * scale + 1 + shrink, x * scale + width - shrink,
                    y * scale + size - shrink), fill=color)
        draw.text((x * scale + 1 + shrink, y * scale + 1 + shrink), blk_id)

        

im.save(f'pnr_result.png', format='PNG')
# im.show()
# frames[0].save('route_animation.gif', format='GIF', append_images=frames[1:], save_all=True, duration=1000, loop=0)

# breakpoint()
