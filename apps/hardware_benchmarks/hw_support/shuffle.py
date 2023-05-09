import json, sys
import os
import os.path as path
import numpy as np
import scipy.io
import functools, operator

def index2vector(ind, extends)->list:
    ret = []
    for ext in extends:
        ret.append(ind % ext)
        ind = ind // ext
    return ret

#input_file = "../aha_garnet_design_new/resnet3_1/resnet3_1_garnet.json"
assert len(sys.argv) == 2, "python shuffle.py coreir.json"
input_file = sys.argv[1]
#input_file = "../aha_garnet_design_new/resnet_init_unroll_tile/resnet_init_unroll_tile_garnet.json"
with open(input_file) as f:
    data = json.load(f)
    for io_group, val in (data["IOs"]).items():
        for io_type in range(len(val)):
            io_unroll = len(val[io_type]["io_tiles"])
            data_shuffle_full = []
            for io in range(io_unroll):
                glb_config = val[io_type]["io_tiles"][io]["addr"]
                ins_name = val[io_type]["io_tiles"][io]["name"]
                ext = glb_config["extent"]
                stride_key = "_data_stride"
                if "outputs" == io_group:
                    stride_key = "write" + stride_key
                elif "inputs" == io_group:
                    stride_key = "read" + stride_key
                else:
                    assert(False)
                stride = glb_config[stride_key]
                compressed_ext = []
                compressed_stride = []
                compressed_new_stride= []

                #Find the zero stride dim
                zero_dim = -1
                for idx, s in enumerate(stride):
                    if (s == 0):
                        assert(zero_dim == -1)
                        zero_dim = idx
                #print (zero_dim)
                new_stride = [0 for _ in stride]
                accumulate_stride = 1
                dim_left = len(stride)
                if (zero_dim == -1):
                    zero_dim = 0
                else:
                    new_stride[zero_dim] = 0
                    zero_dim = zero_dim + 1
                    dim_left = dim_left - 1
                for dim in range(zero_dim, dim_left + zero_dim, 1):
                    wrap_dim = dim % len(stride)
                    new_stride[wrap_dim] = accumulate_stride
                    accumulate_stride = accumulate_stride * ext[wrap_dim]
                print ("\n\nFound IO instance name: ", ins_name)
                print("new stride: ", new_stride)

                # get the image size
                for idx, s in enumerate(stride):
                    if s != 0:
                        compressed_ext.append(ext[idx])
                        compressed_stride.append(s)
                        compressed_new_stride.append(new_stride[idx])
                img_size = functools.reduce(operator.mul, compressed_ext)
                compressed_ext = np.asarray(compressed_ext)
                compressed_stride = np.asarray(compressed_stride)
                compressed_new_stride= np.asarray(compressed_new_stride)
                print("ext: ", ext)
                print("compressed ext: ", compressed_ext)
                print ("image size: ", img_size)

                #create a dummy data now
                #TODO: should use a json file to save the input name
                def get_img_name(ins_name):
                  if "input" in ins_name:
                      return "input_host_stencil"
                      #return "input_padded"
                      #assert(path.exists("input_padded.mat"))
                      #data_org = scipy.io.loadmat("input_padded.mat")["input_padded"]
                      #data_org = np.random.randint(-128, 128, 58*58*16)
                  elif "kernel" in ins_name:
                      return "kernel"
                      #assert(path.exists("kernel.mat"))
                      #data_org = scipy.io.loadmat("kernel.mat")["kernel"]
                      #data_org = np.random.randint(-128, 128, 16*64*9)
                  elif "output" in ins_name:
                      return "hw_output"
                      #assert(path.exists("output_cpu.mat"))
                      #data_org = scipy.io.loadmat("output_cpu.mat")["output_cpu"]
                      #data_org = np.random.randint(-128, 128, 28*28*64)
                  else:
                      assert(False)

                img_file_name = get_img_name(ins_name)
                if path.exists(img_file_name+".mat"):
                    data_org = scipy.io.loadmat(img_file_name + ".mat")[img_file_name]
                elif path.exists(img_file_name+".raw"):
                    data_org = np.fromfile(img_file_name + ".raw", dtype=">i2")
                else:
                    raise Exception(f"{image_file_name}.raw or {image_file_name}.mat does not exist")
                print ("load halide generated image, with size =", data_org.shape)
                data_org = data_org.flatten('F')
                print ("After Flatten Fortran style size =", data_org.shape)
                
                #Split the image data based on unrolling (assumes innermost dimension is unrolled)
                data_org = data_org[io::io_unroll]

                #Create the reordered array
                data_shuffle = np.zeros(img_size, dtype='>i2')

                #sanity check set
                access_addr_set = set({})
                save_addr_set = set({})


                #copy the old data into new data
                for idx in range(img_size):
                    nd_idx = np.asarray(index2vector(idx, compressed_ext))
                    origin_addr = nd_idx.dot(compressed_stride)
                    new_addr = nd_idx.dot(compressed_new_stride)
                    access_addr_set.add(origin_addr)
                    assert(new_addr not in save_addr_set)
                    save_addr_set.add(new_addr)
                    #print("org addr:", origin_addr, "\tnew addr:", new_addr)
                    data_shuffle[new_addr] = data_org[origin_addr]


                def sanity_check_all_addr_visit(img_size, addr_set):
                    # print("length of image", img_size)
                    for addr in range(img_size):
                        # print("addr: ", addr)
                        assert(addr in addr_set)

                sanity_check_all_addr_visit(data_org.size, access_addr_set)
                sanity_check_all_addr_visit(data_shuffle.size, save_addr_set)

                # Append data_shuffle to the total image construction
                data_shuffle_full.append(data_shuffle)

                #optimize the level of iteration ?
                print ("\tnew stride: \t", new_stride)
                print ("\torigin stride: \t", stride)
                print ("\textend: \t", ext)
                print ("\tcycle stride: \t", glb_config["cycle_stride"])




                def merge_stride_and_add2json(ext, stride, glb_config):
                    cycle_stride = glb_config["cycle_stride"]
                    def get_possible_merge_dim(ext, stride):
                        merge_list = [] #list of dimension that can merge with its upper dimension
                        assert(len(stride) == len(ext))
                        for i in range(len(stride)-1):
                            if (stride[i]*ext[i] == stride[i+1]):
                                merge_list.append(i)
                        return merge_list

                    addr_stride_merge_dims = get_possible_merge_dim(ext, stride)
                    sched_stride_merge_dims = get_possible_merge_dim(ext, cycle_stride)

                    def merge_stride(ext, stride, dim):
                        new_stride = []
                        new_ext = []
                        for idx, (e, s) in enumerate(zip(ext, stride)):
                            if (idx == dim):
                                new_stride.append(s)
                                new_ext.append(e* ext[idx+1])
                            elif (idx == dim + 1):
                                continue
                            else:
                                new_stride.append(s)
                                new_ext.append(e)
                        return new_ext, new_stride
                    new_addr_ext = ext[:]
                    new_sched_ext = ext[:]
                    new_addr_stride = new_stride[:]
                    new_sched_stride = cycle_stride[:]
                    for dim in reversed(addr_stride_merge_dims):
                        new_addr_ext, new_addr_stride = merge_stride(new_addr_ext, new_addr_stride, dim)
                    for dim in reversed(sched_stride_merge_dims):
                        new_sched_ext, new_sched_stride = merge_stride(new_sched_ext, new_sched_stride, dim)

                    glb_config[stride_key] = new_stride

                    #print (addr_stride_merge_dims)
                    #print (sched_stride_merge_dims)
                
                    # Layer 1
                    if "ksize=7" in os.getenv('HALIDE_GEN_ARGS') and "input" in ins_name:
                        print("Layer 1 hack")
                        new_addr_stride = [2, 222, 0, 1,]
                        new_addr_ext = [112, 21, 4, 2]
                        new_sched_stride = [1, 8960]
                        new_sched_ext = [2352, 8]
                
                    print ("====Merge Access Pattern to Fit GLB Controller===")
                    print ("\tnew_addr_stride:\t", new_addr_stride)
                    print ("\tnew_addr_ext:\t", new_addr_ext)
                    print ("\tnew_sched_stride:\t", new_sched_stride)
                    print ("\tnew_sched_ext:\t", new_sched_ext)


                    glb_config["new_addr_stride"] = new_addr_stride
                    glb_config["new_addr_extent"] = new_addr_ext
                    glb_config["new_sched_stride"] = new_sched_stride
                    glb_config["new_sched_extent"] = new_sched_ext



                merge_stride_and_add2json(ext, new_stride, glb_config)

            
            data_shuffle_save = np.zeros(img_size*io_unroll, dtype='>i2')
            for i in range(img_size):
                for j in range(io_unroll):
                    data_shuffle_save[i*io_unroll+j] = data_shuffle_full[j][i]
            #save new file
            scipy.io.savemat(img_file_name + "_shuffle.mat", {img_file_name: data_shuffle_save})
            print ("SAVE reordered image into ",img_file_name + "_shuffle.mat!\n")
            data_shuffle_save.astype('>i2').tofile(img_file_name + '_shuffle.raw')
            print ("SAVE reordered image into ",img_file_name + "_shuffle.raw!\n")


with open('design_meta_reorder.json', 'w') as out_file:
    json_data = json.dumps(data, indent=4)
    out_file.write(json_data)

print ("\nSave new coreir json into design_top_reorder.json")


