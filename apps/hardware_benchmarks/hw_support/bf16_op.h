#ifndef BF16_OP_H
#define BF16_OP_H

#include <string>
#include <stdint.h> 
#include <cassert>
#include <iostream>
#include <bitset>
#include <sstream>
#include <cmath>
#include "gen_lut.h"

float bf16_add(float input1, float input2);
float bf16_mul(float input1, float input2);
float bf16_div(float input1, float input2);
int bf16_f2int(float input);
int bf16_getfr(float input);
float bf16_faddiexp(float input1, int input2);
float bf16_expsub(float in1, float in2);
unsigned long bf16_getman(float input);
std::string float2bfbin(float input, bool return_hex, bool return_bin_string);
float bfbin2float(std::string input);
unsigned int bfbin2uint(std::string input);

#endif