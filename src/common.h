#ifndef _COMMON_H
#define _COMMON_H



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMMON APP ENTITY:  basic logic symbols
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMMON APP ENTITY:  labels for Boolean values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

enum flag_event_e
{
    FLAG_CLEARED = 0,
    FLAG_SET
};

#define VALUE_OF_FLAG_SET     FLAG_SET
#define VALUE_OF_FLAG_CLEARED FLAG_CLEARED

#define SET_FLAG   VALUE_OF_FLAG_SET
#define CLEAR_FLAG VALUE_OF_FLAG_CLEARED



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMMON APP ENTITY:  TIME PERIODS
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define ONE_MILLISECOND (1000)


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMMON APP ENTITY:  IIS2DH accelerometer x,y,z readings defines
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#define BYTES_PER_READING (2)
#define READINGS_PER_TRIPLET (3)
#define BYTES_PER_XYZ_READINGS_TRIPLET ( BYTES_PER_READING * READINGS_PER_TRIPLET )

struct acc_reading_triplet
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
};



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMMON APP ENTITY:  output line and data formatting
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// 2022-02-22 Note, for some reason terminal emulator `minicom` overwrites
//  present line when Pulse Pro writes to minicom's connected UART
//  and formats line endings with '\n' alone.  This problem is solved
//  by providing line endings with a newline character followed by
//  a carriage return \r.  This line termination however creates double
//  spaced output in terminal emulator . . . ask Josh which terminal
//  emulator he using - TMH
//
//  Following defines created to allow for easier switching between
//  terminal emulators.  In the future a CLI command can be added to
//  allow developers to change line end formatting while firmware is
//  running:

#define ONE_NEWLINE_COMPATIBLE_WITH_MINICOM "\n\r"
#define TWO_NEWLINES_COMPATIBLE_WITH_MINICOM "\n\r\n\r"

#define ONE_NEWLINE ONE_NEWLINE_COMPATIBLE_WITH_MINICOM
#define TWO_NEWLINES TWO_NEWLINES_COMPATIBLE_WITH_MINICOM


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMMON APP ENTITY:  macros to compact repetitive and or verbose code
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

/*
 *----------------------------------------------------------------------
 * @Brief   Following macro used in command line parsing.  Checks
 *          whether selected argument from lateset command line input
 *          matches a particular string.
 *----------------------------------------------------------------------
 */

//#include "thread-simple-cli.h"

// NOTE variables for macro (this may not work depending on macro expansion - TMH):
//uint32_t match = 0;
//char argument[SUPPORTED_ARG_LENGTH];

// "zph_" means "Place Holder" for variables which are normally declared
// at a routine scope:

#define ZCHECK_IF_DECIMAL_AT_ARG_INDEX(n, zph_decimal_value) \
match += arg_is_decimal(n, &zph_decimal_value);

#define ZSTRNCMP_ARGUMENT(index_to_argument, pattern, up_to_byte_length, zph_argument, zph_match) \
rstatus = arg_n(index_to_argument, zph_argument); \
zph_match += strncmp(zph_argument, pattern, up_to_byte_length);

enum cli_argument_index_labels
{
    ARG_FIRST = 0,
    ARG_SECOND,
    ARG_THIRD,
    ARG_FOURTH,
    ARG_FIFTH,
    ARG_SIXTH,
    ARG_SEVENTH,
    ARG_EIGTH
};



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// COMMON APP ENTITY:  generic options
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

enum pulse_pro_generic_integer_options
{
    DEFAULT_OPTION_OF_ZERO
};



#endif // _COMMON_H
