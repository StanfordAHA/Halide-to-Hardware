#!/bin/python

unrolls = [1, 2, 4, 8, 14]
block_size = [2, 3, 4, 5, 6, 7]

for block_size in block_size:
    print(f"###block_size###: {block_size}")
    for output_size in range(20, 100):
        print(f"{output_size}: ",end='')
        for unroll in unrolls:
            input_size = (output_size + block_size - 1) / unroll
            if output_size % unroll == 0:# and input_size == (output_size + block_size - 1) // unroll:
                if input_size % 4 == 0 or input_size < 20:
                    print("x",end='')
                else:
                    print("/",end='')
            else:
                print(" ",end='')
        print("")
