#ifndef _CONVERSIONS_H
#define _CONVERSIONS_H


// - SECTION - prototypes

float reading_in_g(const uint32_t reading_in_twos_comp, const uint32_t full_scale, const uint32_t res_in_bits);

void integer_to_binary_string(const uint32_t integer, char* string, const uint32_t str_length);


// - SECTION - defines

#define APPR_ACCELERATION_OF_GRAVITY (9.80665)

//
// 2021-11-08 - Empirically chosen scaling factor which will give three
//  decimal places for most VRMS floating point values observed this
//  week:                                                         - TMH
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#define VRMS_POWER_OF_TEN_SCALING_FOR_JSON (10000)



// In following defines 'plus one' accounts for string terminating null character, zero value:
#define BINARY_REPRESENTATION_EIGHT_BITS_AS_STRING (8 + 1)
#define BINARY_REPRESENTATION_SIXTEEN_BITS_AS_STRING (16 + 1)
#define BINARY_REPRESENTATION_THIRTY_TWO_BITS_AS_STRING (32 + 1)



#endif // _CONVERSIONS_H
