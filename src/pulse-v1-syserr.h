////////////////////////////////////////////////////////////////////////////////
// syserr.h -- System Error Accounting
////////////////////////////////////////////////////////////////////////////////

#ifndef _syserr_h_INCLUDED_
#define _syserr_h_INCLUDED_

#include <stdbool.h>
#include <stdint.h>

#include "pulse-v1-syserrs.h"

// application integration
// -----------------------
void syserr_init(void);  // initialize module, call once at start of application

// error reporting
// ---------------
void syserr_raise (syserr_t err);
void syserr_assert(culprit_t culprit);

// accounting access/maintenance
// -----------------------------
uint32_t syserr_total(void); // returns total count of all errors reported
uint8_t  syserr_check(syserr_t err); // returns count (upto saturation limit) of errors reported for specific type
void     syserr_clear_all(void); // clears all error counts (including total)

#endif // _syserr_h_INCLUDED_
