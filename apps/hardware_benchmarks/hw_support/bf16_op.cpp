#include "bf16_op.h"

float bf16_add(float input1, float input2) {
    
    std::bitset<32> bf16_mask(0x0000FFFF);
    std::bitset<32> input1_bin(*reinterpret_cast<unsigned int*>(&input1));
    std::bitset<32> input2_bin(*reinterpret_cast<unsigned int*>(&input2));
    // first check the float is in bfloat16 format, i.e. the lower 16 bits are all zeros
    assert((input1_bin & bf16_mask) == std::bitset<32>(0x00000000));
    assert((input2_bin & bf16_mask) == std::bitset<32>(0x00000000));
    
    // add the float numbers
    float result = input1 + input2;

    // rounding to bfloat16 with IEEE 754 round to nearest even
    // convert the results float to binary
    std::bitset<32> result_bin(*reinterpret_cast<unsigned int*>(&result));

    // extract sign, exponent, and mantissa
    unsigned long exp = ((result_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();
    unsigned long lfrac = ((result_bin & std::bitset<32>(0x007F0000)) >> 16).to_ulong();
    unsigned long hfrac = (result_bin & std::bitset<32>(0x0000FFFF)).to_ulong();
    std::bitset<1> sign_bit = std::bitset<1>(result_bin[31]);
    std::bitset<8> exp_bit(exp);
    std::bitset<7> lfrac_bit(lfrac);
    std::bitset<16> hfrac_bit(hfrac);

    // rounding logic
    if (hfrac_bit.to_ulong() == 32768) {
        // tie, round to even 
        if (lfrac_bit[0] == 1) {
            if (lfrac_bit == std::bitset<7>(0x7F)) {
                // roll over mantissa and increase exponent
                exp = exp + 1;
            }
            lfrac = lfrac + 1;
        }
        
    } else if (hfrac_bit.to_ulong() > 32768) {
        // round up
        if (lfrac_bit == std::bitset<7>(0x7F)) {
                // roll over mantissa and increase exponent
                exp = exp + 1;
            }
        lfrac = lfrac + 1;
    }
    
    // recombine rounded sign, exponent, and mantissa
    std::bitset<8> exp_rounded_bit(exp);
    std::bitset<7> lfrac_rounded_bit(lfrac);
    std::bitset<32> result_bf16_bit(sign_bit.to_string() + exp_rounded_bit.to_string() + lfrac_rounded_bit.to_string() + "0000000000000000");

    // convert to float
    unsigned int tmp_result_bf16 =static_cast<unsigned int>(result_bf16_bit.to_ulong());
    float result_bf16 = *reinterpret_cast<float *>(&tmp_result_bf16);

    return result_bf16;
}

float bf16_mul(float input1, float input2) {

    std::bitset<32> bf16_mask(0x0000FFFF);
    std::bitset<32> input1_bin(*reinterpret_cast<unsigned int*>(&input1));
    std::bitset<32> input2_bin(*reinterpret_cast<unsigned int*>(&input2));
    // first check the float is in bfloat16 format, i.e. the lower 16 bits are all zeros
    assert((input1_bin & bf16_mask) == std::bitset<32>(0x00000000));
    assert((input2_bin & bf16_mask) == std::bitset<32>(0x00000000));

    // multiply the float numbers
    float result = input1 * input2;

    // rounding to bfloat16 with IEEE 754 round to nearest even
    // convert the results float to binary
    std::bitset<32> result_bin(*reinterpret_cast<unsigned int*>(&result));

    // extract sign, exponent, and mantissa
    unsigned long exp = ((result_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();
    unsigned long lfrac = ((result_bin & std::bitset<32>(0x007F0000)) >> 16).to_ulong();
    unsigned long hfrac = (result_bin & std::bitset<32>(0x0000FFFF)).to_ulong();
    std::bitset<1> sign_bit = std::bitset<1>(result_bin[31]);
    std::bitset<8> exp_bit(exp);
    std::bitset<7> lfrac_bit(lfrac);
    std::bitset<16> hfrac_bit(hfrac);

    // rounding logic (please refer to float2bfbin in lassen)
    if ((hfrac_bit[15] == 1 && (hfrac_bit[14] == 1 || hfrac_bit[13] == 1))
            || (lfrac_bit[0] == 1 && hfrac_bit[15] == 1)) {
        // round up
        if (lfrac_bit == std::bitset<7>(0x7F)) {
                // roll over mantissa and increase exponent
                exp = exp + 1;
            }
        lfrac = lfrac + 1;
    }

    // recombine rounded sign, exponent, and mantissa
    std::bitset<8> exp_rounded_bit(exp);
    std::bitset<7> lfrac_rounded_bit(lfrac);
    std::bitset<32> result_bf16_bit(sign_bit.to_string() + exp_rounded_bit.to_string() + lfrac_rounded_bit.to_string() + "0000000000000000");

    // convert to float
    unsigned int tmp_result_bf16 =static_cast<unsigned int>(result_bf16_bit.to_ulong());
    float result_bf16 = *reinterpret_cast<float *>(&tmp_result_bf16);

    return result_bf16;
}

float bf16_div(float in1, float in2) {

    // check inputs are in bfloat16 format
    std::bitset<32> in1_bin(*reinterpret_cast<unsigned int*>(&in1));
    std::bitset<32> in2_bin(*reinterpret_cast<unsigned int*>(&in2));
    assert((in1_bin & std::bitset<32>(0x0000FFFF)) == std::bitset<32>(0x00000000));
    assert((in2_bin & std::bitset<32>(0x0000FFFF)) == std::bitset<32>(0x00000000));

    // generate the lut for div
    std::vector<float> div_rom = gen_div_lut();
    unsigned int in2_mant = bf16_getman(in2);
    float lut_out = div_rom[in2_mant];
    float subexp_out = bf16_expsub(lut_out, in2);
    float result = bf16_mul(subexp_out, in1);

    return result;
}

int bf16_f2int(float input) {

    // check if input is in bfloat16 format
    std::bitset<32> input_bin(*reinterpret_cast<unsigned int*>(&input));
    assert((input_bin & std::bitset<32>(0x0000FFFF)) == std::bitset<32>(0x00000000));

    // extract sign, exponent, and fraction
    unsigned long sign = ((input_bin & std::bitset<32>(0x80000000)) >> 31).to_ulong();
    unsigned long exp = ((input_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();
    unsigned long frac = ((input_bin & std::bitset<32>(0x007F0000)) >> 16).to_ulong();
    std::bitset<23> frac_bin(frac);
    
    // add back the implicit 1
    frac_bin = frac_bin | std::bitset<23>(0x0080);
    
    // unbias of the exponent
    long exp_unbias = exp - 127;

    // shift the fraction according to the exponent
    if (exp_unbias >= 0) {
        frac_bin = frac_bin << exp_unbias;
    } else {
        frac_bin = frac_bin >> -exp_unbias;
    }

    // retreive the integer part by shifting out the fraction
    frac_bin = frac_bin >> 7;
    std::bitset<16> res(frac_bin.to_string());

    // sign conversion acoording to the sign bit
    if (sign == 1) {
        return -1 * frac_bin.to_ulong();
    } else {
        return frac_bin.to_ulong();
    }
}

int bf16_getfr(float input) {

    // check if input is in bfloat16 format
    std::bitset<32> input_bin(*reinterpret_cast<unsigned int*>(&input));
    assert((input_bin & std::bitset<32>(0x0000FFFF)) == std::bitset<32>(0x00000000));

    // extract sign, exponent, and fraction
    unsigned long sign = ((input_bin & std::bitset<32>(0x80000000)) >> 31).to_ulong();
    unsigned long exp = ((input_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();
    unsigned long frac = ((input_bin & std::bitset<32>(0x007F0000)) >> 16).to_ulong();
    
    // extract the fraction and add the implicit 1
    std::bitset<16> frac_bin(frac);

    // add back the implicit 1
    frac_bin = frac_bin | std::bitset<16>(0x0080);

    // unbias of the exponent
    long exp_unbias = exp - 127;

    // shift the fraction according to the exponent
    if (exp_unbias >= 0) {
        frac_bin = frac_bin << exp_unbias;
    } else {
        frac_bin = frac_bin >> -exp_unbias;
    }

    // mask out the unwanted bits
    frac_bin = frac_bin & std::bitset<16>(0x007F);

    // sign conversion acoording to the sign bit
    if (sign == 1) {
        return -1 * frac_bin.to_ulong();
    } else {
        return frac_bin.to_ulong();
    }
}

float bf16_faddiexp(float input1, int input2) { 

    // check if input is in bfloat16 format
    std::bitset<32> input_bin(*reinterpret_cast<unsigned int*>(&input1));
    assert((input_bin & std::bitset<32>(0x0000FFFF)) == std::bitset<32>(0x00000000));

    // extract sign, exponent, and mantissa
    std::bitset<1> sign_bit = std::bitset<1>(input_bin[31]);
    unsigned long exp = ((input_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();
    unsigned long lfrac = ((input_bin & std::bitset<32>(0x007F0000)) >> 16).to_ulong();

    // add the exponent
    exp = exp + input2;

    // recombine sign, exponent, and mantissa
    std::bitset<8> exp_bit(exp);
    std::bitset<7> lfrac_bit(lfrac);
    std::bitset<32> result_bin(sign_bit.to_string() + exp_bit.to_string() + lfrac_bit.to_string() + "0000000000000000");

    // convert to float
    unsigned int tmp_result_bf16 =static_cast<unsigned int>(result_bin.to_ulong());
    float result_bf16 = *reinterpret_cast<float *>(&tmp_result_bf16);

    return result_bf16;
}

// bf16_expsub substracts the exponent of in2 from the that of the in1
float bf16_expsub(float in1, float in2) {

    // ensuring both floating point inputs are of the bf16 format
    std::bitset<32> in1_bin(*reinterpret_cast<unsigned int*>(&in1));
    std::bitset<32> in2_bin(*reinterpret_cast<unsigned int*>(&in2));
    assert((in1_bin & std::bitset<32>(0x0000FFFF)) == std::bitset<32>(0x00000000));
    assert((in2_bin & std::bitset<32>(0x0000FFFF)) == std::bitset<32>(0x00000000));
    
    // extract sign, exponent, and mantissa of in1
    std::bitset<1> in1_sign_bit = std::bitset<1>(in1_bin[31]);
    unsigned long in1_exp = ((in1_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();
    unsigned long in1_lfrac = ((in1_bin & std::bitset<32>(0x007F0000)) >> 16).to_ulong();

    // extract exponent of in2
    std::bitset<1> in2_sign_bit = std::bitset<1>(in2_bin[31]);
    unsigned long in2_exp = ((in2_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();

    // substract the exponent, remember to re-bias the exponent!!
    unsigned long res_exp = in1_exp - in2_exp + 127;

    // reconstruct result with the new exponent
    std::bitset<8> res_exp_bit(res_exp);
    std::bitset<7> res_lfrac_bit(in1_lfrac);
    std::bitset<32> result_bin((in1_sign_bit | in2_sign_bit).to_string() + res_exp_bit.to_string() + res_lfrac_bit.to_string() + "0000000000000000");

    // convert to float
    unsigned int tmp_result_bf16 =static_cast<unsigned int>(result_bin.to_ulong());
    float result_bf16 = *reinterpret_cast<float *>(&tmp_result_bf16);

    return result_bf16;
}

unsigned long bf16_getman(float input) {

    // check if input is in bfloat16 format
    std::bitset<32> input_bin(*reinterpret_cast<unsigned int*>(&input));
    assert((input_bin & std::bitset<32>(0x0000FFFF)) == std::bitset<32>(0x00000000));

    // extract sign, exponent, and fraction
    unsigned long sign = ((input_bin & std::bitset<32>(0x80000000)) >> 31).to_ulong();
    unsigned long exp = ((input_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();
    unsigned long lfrac = ((input_bin & std::bitset<32>(0x007F0000)) >> 16).to_ulong();
    
    return lfrac;
}

std::string float2bfbin(float input, bool return_hex, bool return_bin_string) {
    // convert the input float to binary
    std::bitset<32> input_bin(*reinterpret_cast<unsigned int*>(&input));
    std::bitset<32> conversion_mask(0xFFFF0000);

    // convert to bf16 by masking the lower 16 bits and shifting right 
    unsigned long exp = ((input_bin & std::bitset<32>(0x7F800000)) >> 23).to_ulong();
    unsigned long lfrac = ((input_bin & std::bitset<32>(0x007F0000)) >> 16).to_ulong();
    unsigned long hfrac = (input_bin & std::bitset<32>(0x0000FFFF)).to_ulong();
    std::bitset<1> sign_bit = std::bitset<1>(input_bin[31]);
    std::bitset<8> exp_bit(exp);
    std::bitset<7> lfrac_bit(lfrac);
    std::bitset<16> hfrac_bit(hfrac);

    // rounding logic (please refer to float2bfbin in lassen)
    if ((hfrac_bit[15] == 1 && (hfrac_bit[14] == 1 || hfrac_bit[13] == 1))
            || (lfrac_bit[0] == 1 && hfrac_bit[15] == 1)) {
        // round up
        if (lfrac_bit == std::bitset<7>(0x7F)) {
                // roll over mantissa and increase exponent
                exp = exp + 1;
            }
        lfrac = lfrac + 1;
    }

    // recombine rounded sign, exponent, and mantissa
    std::bitset<8> exp_rounded_bit(exp);
    std::bitset<7> lfrac_rounded_bit(lfrac);
    std::bitset<16> bfbin(sign_bit.to_string() + exp_rounded_bit.to_string() + lfrac_rounded_bit.to_string());

    if (return_hex) {
        std::stringstream ss;
        ss << std::hex << bfbin.to_ulong();
        return ss.str();
    } else {
        if (return_bin_string) {
            return bfbin.to_string();
        } else {
            return std::to_string(bfbin.to_ulong());
        }
    }
}

float bfbin2float(std::string input) {

    // convert the input binary string to bitset
    std::bitset<32> bfbin(input + "0000000000000000");

    //convert to float
    unsigned int tmp_result =static_cast<unsigned int>(bfbin.to_ulong());
    float result = *reinterpret_cast<float *>(&tmp_result);

    return result;
}

unsigned int bfbin2uint(std::string input) {

    // convert the input binary string to bitset
    std::bitset<32> bfbin(input);

    return bfbin.to_ulong();
}