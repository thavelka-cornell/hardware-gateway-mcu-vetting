////////////////////////////////////////////////////////////////////////////////
// syserr.c -- System Error Accounting
//
// @Note Source file from Pulse V1
////////////////////////////////////////////////////////////////////////////////

#define _COMPILING_syserr_c_

#include "pulse-v1-syserr.h"

////////////////////////////////////////////////////////////////////////////////
static uint8_t  s_syserrs[NUM_SYSERRS]; // count of each error type, each saturates at 0xFF
static uint32_t s_syserr_cnt; // total count of reported errors, saturates at 0xFFFFFFFF
static uint8_t  s_culprits[NUM_CULPRITS]; // count of each culprit type, each saturates at 0xFF

////////////////////////////////////////////////////////////////////////////////
static void internal_clear_all(void)
{
	unsigned i;
	for ( i = 0; i < NUM_SYSERRS; i++ )
		s_syserrs[i] = 0;
	s_syserr_cnt = 0;
	for ( i = 0; i < NUM_CULPRITS; i++ )
		s_culprits[i] = 0;
}

////////////////////////////////////////////////////////////////////////////////
void syserr_init(void)
{
	internal_clear_all();
}

////////////////////////////////////////////////////////////////////////////////
void syserr_raise(syserr_t err)
{
	static uint8_t s_blink_serious = 0;

	if ( err > SYSERR_NONE && err < NUM_SYSERRS )
	{
		// increment (with saturation) individual error count
		if ( s_syserrs[err] != 0xFF )
			s_syserrs[err]++;

		// increment (with saturation) total error count
		if ( s_syserr_cnt != 0xFFFFFFFF )
			s_syserr_cnt++;
	}
}

////////////////////////////////////////////////////////////////////////////////
void syserr_assert(culprit_t culprit)
{
	if (culprit >= 0 && culprit < NUM_CULPRITS)
	{
		// increment (with saturation) individual culprit count
		if ( s_culprits[culprit] != 0xFF )
			s_culprits[culprit]++;
	}
	syserr_raise(SYSERR_ASSERT_FAIL);
}

////////////////////////////////////////////////////////////////////////////////
uint32_t syserr_total(void)
{
	return s_syserr_cnt;
}

////////////////////////////////////////////////////////////////////////////////
uint8_t syserr_check(syserr_t err)
{
	if (err == SYSERR_MAX_IDX) // same as SYSERR_NONE
		return NUM_SYSERRS - 1; // highest SYSERR index

	if ( err < NUM_SYSERRS )
		return s_syserrs[err];

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
void syserr_clear_all(void)
{
	internal_clear_all();
}
