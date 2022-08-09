////////////////////////////////////////////////////////////////////////////////
// axis_sel.h -- Axis Selection module
//
// @Note Source file from Pulse V1
////////////////////////////////////////////////////////////////////////////////

#ifndef _AXIS_SEL_H_INCLUDED_
#define _AXIS_SEL_H_INCLUDED_

#include <stdbool.h>
#include <stdint.h>

// axis indexing
typedef enum
{ AXIS_X = 0
, AXIS_Y = 1
, AXIS_Z = 2
, AXES = 3
} axis_t;

// axis selection
typedef enum
{ AXIS_SEL_NONE = 0
, AXIS_SEL_X    = (1 << AXIS_X)
, AXIS_SEL_Y    = (1 << AXIS_Y)
, AXIS_SEL_Z    = (1 << AXIS_Z)
, AXIS_SEL_XY   = (AXIS_SEL_X | AXIS_SEL_Y)
, AXIS_SEL_XZ   = (AXIS_SEL_X | AXIS_SEL_Z)
, AXIS_SEL_YZ   = (AXIS_SEL_Y | AXIS_SEL_Z)
, AXIS_SEL_XYZ  = (AXIS_SEL_X | AXIS_SEL_Y | AXIS_SEL_Z)
, AXIS_SEL_MASK = 7
} axis_sel_t;

// set axis selection
void axis_sel_set(axis_sel_t as);

// get current axis selection
axis_sel_t axis_sel_get(void);

// convert enum value to string
const char *axis_sel_to_str(axis_sel_t as);

#endif // _AXIS_SEL_H_INCLUDED_
