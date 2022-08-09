#ifndef _READINGS_SET_H
#define _READINGS_SET_H


#include <stdint.h>         // to provide define of uint32_t

#include "common.h"         // to provide acc_reading_triplet structure definition



#define FULL_READINGS_SET (2048) // (8192)


uint32_t readings_set__set_flag_readings_ready(const uint32_t val);

uint32_t readings_set__get_flag_readings_ready(uint32_t *val);

uint32_t readings_set__store_readings(
                                       const struct acc_reading_triplet *readings,
                                       const uint32_t reading_triplets_count,
                                       uint32_t *count_readings_stored
                                     );

uint32_t readings_set__share_latest_set(uint8_t *callers_array);

void readings_set__start_new_set(void);

uint32_t readings_set__show_latest_set(const uint32_t option);

uint32_t copy_x_axis_data_to(uint8_t *callers_x_axis_buffer);



#endif // _READINGS_SET_H
