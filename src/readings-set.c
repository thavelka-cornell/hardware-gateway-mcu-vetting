/*
 * =====================================================================
 *
 *  @Project   Pulse Stage 1 firmware
 *
 *  @File:     readings-set.c
 *
 *  @Brief     . . .
 *
 * =====================================================================
 */

//
// TO-DO:  much code clean up to do in this file - TMH
//



//----------------------------------------------------------------------
// - SECTION - pound includes
//----------------------------------------------------------------------

#include <string.h>       // to provide memset(), memcpy() family functions

// For development purposes:
#include <sys/printk.h>   // to provide (Zephyr?) printk()

#include "readings-set.h"
#include "common.h"
#include "conversions.h"
#include "app-return-values.h"
#include "routine-options.h"



//----------------------------------------------------------------------
// - SECTION - pound defines
//----------------------------------------------------------------------

#define WIDTH_OF_INTEGER_PART (3)


//----------------------------------------------------------------------
// - SECTION - file scoped variabes
//----------------------------------------------------------------------

static uint32_t flag_readings_set_ready = FALSE;

// File scoped index to triplets of accelerometer readings:
static uint32_t readings_index_fsv = 0;

#if 0 // this structure declaration moved to common.h:
struct acc_reading_triplet
{
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t z = 0;
};
#endif

// 2021-11-03 - Should have seen this at outset but to have the array
//  for our acceleromeer readings set be more structured, to be an
//  array of structure elements will make indexing the readings much
//  easier for things like to display select readings, and also makes
//  our coding less error prone in terms of over-running the readings
//  array index past the end of the array:

// uint8_t readings_set[BYTES_PER_XYZ_READINGS_TRIPLET * (FULL_READINGS_SET - 0)];
struct acc_reading_triplet readings_set[FULL_READINGS_SET];



//----------------------------------------------------------------------
// - SECTION - prototypes
//----------------------------------------------------------------------



//----------------------------------------------------------------------
// - SECTION - File scoped or global variables -
//----------------------------------------------------------------------

uint32_t readings_set__set_flag_readings_ready(const uint32_t val)
{
// If mutex available and we could update flag:
    return 0;
}



uint32_t readings_set__get_flag_readings_ready(uint32_t* val)
{
    *val = flag_readings_set_ready;
    return 0;
}



// Remember readings index is pointer to triplets of x,y,z readings and not individual readings:

uint32_t readings_set__current_reading_index(void)
{
    return readings_index_fsv;
}


//
// 2021-11-05 NOTE - this routine designed upon x,y,z readings triplet data structure:
//
uint32_t readings_set__store_readings(
                                       const struct acc_reading_triplet* readings_from_caller,
                                       const uint32_t triplets_count,
                                       uint32_t *count_readings_stored
                                     )
{

    uint32_t i = 0;
    uint32_t readings_ready;
    uint32_t rstatus = ROUTINE_OK;

// Not yet implemented - test for low resolution eight-bit wide readings

// Not yet implemented - assure byte count is even:

#if DEV__DIAG_MSGS_STORE_READINGS == 1
printk("- readings-set.c - called to store %u readings triplets,\n", triplets_count);
#endif


    rstatus = readings_set__get_flag_readings_ready(&readings_ready);

// While unfilled readings remain, write them to the preset set:

    if ( readings_ready == FALSE )
    {
        while ( ( readings_index_fsv < FULL_READINGS_SET ) && ( i < triplets_count ) )
        {
            readings_set[readings_index_fsv].x = readings_from_caller[i].x;
            readings_set[readings_index_fsv].y = readings_from_caller[i].y;
            readings_set[readings_index_fsv].z = readings_from_caller[i].z;
            readings_index_fsv++;
            i++;

#ifdef PULSE_DEV__SHOW_GROUPS_OF_1024_READINGS_CAPTURED
// - DEV BEGIN - show message at 1kB readings stored internals:
            if ( ( readings_index_fsv % 1024 ) == 0 )
            {
                printk("- readings-set.c - stored 1024 reading triplets, now at %u total triplets,\n", readings_index_fsv);
            }
// - DEV END -
#endif
        }

        if ( readings_index_fsv >= FULL_READINGS_SET )
        {
            rstatus = readings_set__set_flag_readings_ready(TRUE);
        }

    }
    else
    {
        rstatus = RS_ACCELEROMETER_READINGS_SET_FILLED;
printk("- readings-set.c - readings set full, index to latest pointing to %u,\n", readings_index_fsv);
    }


// If we make it here and local index 'i' is yet zero then we didn't store any
// readings at all, note this in return value to caller:

    if ( i == 0 )
    {
        rstatus = RS_STORE_READINGS_CALLED_BUT_NONE_WRITTEN;
    }

    *count_readings_stored = i;

    return rstatus;
}



uint32_t readings_set__share_latest_set(uint8_t *callers_array)
{
    uint32_t readings_ready = FALSE;
    uint32_t rstatus = readings_set__get_flag_readings_ready(&readings_ready);

    if ( readings_ready )
    {
        memcpy(callers_array, &readings_set, FULL_READINGS_SET);

// Reset some stuff to permit next set of readings to be gathered:
        rstatus = readings_set__set_flag_readings_ready(FALSE);
        readings_index_fsv = 0;
    }
    else
    {
        rstatus = RS_ACCELEROMETER_READINGS_SET_NOT_READY;
    }

    return rstatus;
}



void readings_set__start_new_set(void)
{
    readings_index_fsv = 0;
}



//
// 2021-11-05 NOTE - this routine designed upon x,y,z readings triplet data structure:
//
uint32_t readings_set__show_latest_set(const uint32_t option)
{
    uint32_t readings_ready = FALSE;
    uint32_t index_to_latest_reading = 0;
    uint32_t i = 0;
    uint32_t rstatus = ROUTINE_OK;

    rstatus = readings_set__get_flag_readings_ready(&readings_ready);

    if ( readings_ready == TRUE )
    {
        index_to_latest_reading = FULL_READINGS_SET;
        printk("- readings_set - readings ready, showing set of %u readings:\n", 
          index_to_latest_reading);
    }
    else
    {
        index_to_latest_reading = readings_index_fsv;
        if ( index_to_latest_reading == 0 )
        {
            printk("- readings_set - no acceleration readings to show.\n");
            rstatus = RS_ACCELEROMETER_READINGS_SET_EMPTY;
        }
        else
        {
            printk("- readings_set - readings ready, showing set of %u readings:\n",
              index_to_latest_reading);
            rstatus = RS_ACCELEROMETER_READINGS_SET_FILLED_PARTLY;
        }
    }


#define TEN_READINGS_TRIPLETS ( 10 * BYTES_PER_XYZ_READINGS_TRIPLET )

// Here we know how many readings sets there are can be shown, check
// caller's option to decide how many to show:
    if ( index_to_latest_reading > 0 )
    {
        if ( (option == SHOW_FIRST_AND_LAST) && (index_to_latest_reading > TEN_READINGS_TRIPLETS) )
        {
// --- 1103 DEV BEGIN ---
// 2021-11-03 WED - work to correct starting index of tail end of readings,
//  we want this index to land on an X-axis value, even though all powers
//  of two do not align on the beginning of readings triplets.

            uint32_t start_of_last_readings = 0;
            start_of_last_readings = (index_to_latest_reading / READINGS_PER_TRIPLET) * BYTES_PER_XYZ_READINGS_TRIPLET; 
            printk("- 1103 DEV - BYTES PER XYZ READINGS TRIPLET = %u,\n", BYTES_PER_XYZ_READINGS_TRIPLET);
            printk("- 1103 DEV - index of latest reading = %u,\n", index_to_latest_reading);
            printk("- 1103 DEV - figuring starting index of last readings to be %u,\n",
              start_of_last_readings);
// --- 1103 DEV END ---


#define FIRST_N (6)
#define LAST_M (6)

            printk("\n\nfirst %u of acceleration readings for axes x, y, z:\n\n", FIRST_N);

// --- 1103 DEV ---
            for ( i = 0; i < FIRST_N; i++ )
            {
                printk("  %*sreadings %u:   %*s%2.3fG, %*s%2.3fG, %*s%2.3fG\n",
                  WIDTH_OF_INTEGER_PART, " ",
                  i,
                  WIDTH_OF_INTEGER_PART, " ", reading_in_g((uint32_t)readings_set[i].x, 0, 0),
                  WIDTH_OF_INTEGER_PART, " ", reading_in_g((uint32_t)readings_set[i].y, 0, 0),
                  WIDTH_OF_INTEGER_PART, " ", reading_in_g((uint32_t)readings_set[i].z, 0, 0)
                );
            }

            printk("\nlatest %u of acceleration readings for axes x, y, z:\n\n", LAST_M);

            for ( i = index_to_latest_reading - LAST_M; i < index_to_latest_reading; i++ )
            {
                printk("  readings %u:   %*s%2.3fG, %*s%2.3fG, %*s%2.3fG\n",
                  i,
                  WIDTH_OF_INTEGER_PART, " ", reading_in_g((uint32_t)readings_set[i].x, 0, 0),
                  WIDTH_OF_INTEGER_PART, " ", reading_in_g((uint32_t)readings_set[i].y, 0, 0),
                  WIDTH_OF_INTEGER_PART, " ", reading_in_g((uint32_t)readings_set[i].z, 0, 0)
                );
            }

            printk("\n\n");   // in routine readings_set__show_latest_set()

//
//        else if ( index_to_latest_reading <= 10 ) { ... }
//
        }
        else
        {
            for ( i = 0; i < index_to_latest_reading; i++ )
            {
                printk("%*s%2.3fG, %*s%2.3fG, %*s%2.3fG\n",
                  WIDTH_OF_INTEGER_PART,
                  " ",
                  reading_in_g((uint32_t)readings_set[i].x, 0, 0),
                  WIDTH_OF_INTEGER_PART,
                  " ",
                  reading_in_g((uint32_t)readings_set[i].y, 0, 0),
                  WIDTH_OF_INTEGER_PART,
                  " ",
                  reading_in_g((uint32_t)readings_set[i].z, 0, 0)
                );

            }

        } // end ELSE block to show all stored readings

    } // end IF-block checks whether there are readings to show

    return rstatus;
}



/*
 * ---------------------------------------------------------------------
 *  @param    Expects uint8_t buffer size of at least FULL_READINGS_SET
 *
 *  @return   Returns updated buffer with latest x-axis data gathered
 *            from enabled accelerometer on board.
 * ---------------------------------------------------------------------
 */

uint32_t copy_x_axis_data_to(uint8_t *callers_x_axis_buffer)
{
    for ( int i = 0; i < FULL_READINGS_SET; i++ )
    {
        callers_x_axis_buffer[i] = (uint8_t)(readings_set[i].x & 0xFF);
    }
    return 0;
}



// --- EOF ---
