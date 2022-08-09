/*
 * =====================================================================
 *
 *  @Project   Pulse Stage 1 firmware
 *
 *  @File      data-model-stage1.c
 *
 *  @Notes From project to which this file belongs, this file needs
 *     to know the following acceleration data details:
 *
 *     +  N, sample size of acceleration data set
 *
 *     +  x,y,z axes data stored in each their own uint8_t buffers
 *
 *   At first draft porting to Pulse Stage 1, the larger project
 *   defines sample size in header file readings-set.h, in symbol
 *   FULL_READINGS_SET.  Also as of first draft port x,y,z readings
 *   are stored in one buffer (one array) of readings triplets.  While
 *   not ideal the first draft VRMS calculation branch copies x-axis
 *   data to the data model's x-axis buffer.
 *
 *   TO-DO:  revamp acceleration readings stores to separate uint8_t
 *   arrays.  - TMH
 *
 *
 *  @References
 *
 *  Zephyr RTOS most precise timing value available to application code:
 *     +  2022-02-10 - https://docs.zephyrproject.org/2.6.0/reference/kernel/timing/clocks.html?highlight=k_cycle_get_32
 *
 *
 * =====================================================================
 */


//----------------------------------------------------------------------
//  References
//
//    +  https://devzone.nordicsemi.com/f/nordic-q-a/51656/how-to-include-math-cmsis-dsp-library-in-nrf9160
//----------------------------------------------------------------------



//----------------------------------------------------------------------
// - SECTION - includes
//----------------------------------------------------------------------

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
// --- DEV 0208 ---
#include <stdio.h>                 // to provide snprintf() when enabled in prj.conf or other Kconfig
//#include "thread-simple-cli.h"     // to provide printk_cli()
#include <kernel.h>                // to provide k_cycle_get_32()
// --- DEV 0208 ---

#include <arm_math.h>

#include <sys/printk.h>            // to provide (Zephyr?) printk()
//#include "../../modules/hal/cmsis/CMSIS/DSP/Include/arm_math.h"

#include "pulse-v1-axis-sel.h"
#include "pulse-v1-syserrs.h"      // to provide Pulse V1 enumerated error symbols
#include "pulse-v1-syserr.h"       // to provide Pulse V1 error accounting

//#include "conversions.h"           // to provide JSON scaling value for VRMS results
//#include "iis2dh-registers.h"
//#include "pulse-pro-return-values.h"
//#include "readings-set.h"
//#include "scoreboard.h"
//#include "thread-simple-cli.h"

#include "app-return-values.h"     // to provide ROUTINE_OK and related
#include "development-defines.h"   // to provide JSON scaling value for VRMS results
#include "iis2dh-registers.h"



#if 0 // Ted going with more directly copied code for now.
float from_acceleration_calculate_vrms(uint32_t axis)
{
// --- VAR BEGIN ---

    arm_status cmsis_math_routine_status;

    static arm_rfft_fast_instance_f32 fft_instance;

// --- VAR END ---

    if ( axis == 1 ) { }

    return 0.0;
}
#endif


//----------------------------------------------------------------------
// - SECTION - development routines
//----------------------------------------------------------------------

void show_byte_buffer(const uint8_t *callers_buffer, const uint32_t size_of_buffer)
{
#define ELEMENTS_PER_LINE (32)
#define SPACER_BETWEEN_N_ELEMENTS_N_EQUALS (8)

    printk("  ");

    for ( int i = 0; i < size_of_buffer; i++ )
    {
        if ( (i > 0) && ((i % ELEMENTS_PER_LINE) == 0) )
            { printk("\n"); }

        if ( (i > 0) && ((i % SPACER_BETWEEN_N_ELEMENTS_N_EQUALS) == 0) )
            { printk("  "); }

        printk("  %03u", callers_buffer[i]);
    }

    printk("\n\n");   // in routine show_byte_buffer()
}



//----------------------------------------------------------------------
// - SECTION - routine definitions, production routines
//----------------------------------------------------------------------

// --- CODE COPY BEGIN ---

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//  Code copied directly from Pulse V1 data-model.c:
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

// (STEP 2 in code porting)

// number of samples per data set
//#define N (2048)
#define N FULL_READINGS_SET

////////////////////////////////////////////////////////////////////////////////
// raw (integer) accelerometer data buffers
////////////////////////////////////////////////////////////////////////////////
static int8_t acc_x[N];
static int8_t acc_y[N];
static int8_t acc_z[N];
static int8_t *acc_bufs[AXES] = {acc_x, acc_y, acc_z};
#define GET_AXIS_BUF(axis) (acc_bufs[axis])

 // sample freuency (units: Hz)
#define Fs (5376)

// frequency resolution (units: Hz/bin)
static float Fr = (float)Fs / (float)N;


// display control
static bool s_dbgo = true;
#define DBG s_dbgo



// (STEP 3 in code porting)

////////////////////////////////////////////////////////////////////////////////
// computation (float) data buffers (and management functions)
////////////////////////////////////////////////////////////////////////////////
#define USE_F32_BUF3 0

// buffers
static float f32_buf_1[N]; bool f32_buf_1_alloced = false;
static float f32_buf_2[N]; bool f32_buf_2_alloced = false;
#if USE_F32_BUF3
static float f32_buf_3[N]; bool f32_buf_3_alloced = false;
#endif

// init buffer management
static void init_f32_bufs(void)
{
    f32_buf_1_alloced = false;
    f32_buf_2_alloced = false;
#if USE_F32_BUF3
    f32_buf_3_alloced = false;
#endif
}

// allocate a buffer
static float * alloc_f32_buf(void)
{
    if (!f32_buf_1_alloced)
    {
        f32_buf_1_alloced = true;
        return f32_buf_1;
    }
    if (!f32_buf_2_alloced)
    {
        f32_buf_2_alloced = true;
        return f32_buf_2;
    }
#if USE_F32_BUF3
    if (!f32_buf_3_alloced)
    {
        f32_buf_3_alloced = true;
        return f32_buf_3;
    }
#endif
    syserr_raise(SYSERR_ADM_ALLOC);
    return (float*)0;
}

// transfer buffer from one var to another
static float * handoff_f32_buf(float **buf)
{
    float *p = *buf;
    *buf = (float*)0;
    return p;
}

// deallocate a buffer
static void free_f32_buf(float **buf)
{
    if (*buf == f32_buf_1)
        f32_buf_1_alloced = false, *buf = (float*)0;
    else if (*buf == f32_buf_2)
        f32_buf_2_alloced = false, *buf = (float*)0;
#if USE_F32_BUF3
    else if (*buf == f32_buf_3)
        f32_buf_3_alloced = false, *buf = (float*)0;
#endif
    else
        syserr_raise(SYSERR_ADM_FREE);
}



// (STEP 4 - code from st_iis2dh.c)


////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

// returns <float> square of <float> input
static float sq(float x)
{
    return x * x;
}


// returns <float> scalar absolute value (i.e. magnitude) of <float,float> complex value
static float absf_complex(float re, float im)
{
    return sqrtf(re*re + im*im);
}


// returns <float> omega value for given index (0 to N/2)
//   uses Fr (nonlocal) value
static float omega(unsigned idx)
{
    return 2.0 * PI * (float)idx * Fr;
}



// returns X-axis value (i.e. frequency) given an index (0 to N/2)
static float x_axis(unsigned idx)
{
    return (float)idx * Fr;
}



float acc_raw_to_float_mg(int16_t raw)
{
	float x = 0.0;
        uint8_t iis2dh_full_scale_setting;
        uint32_t rstatus = ROUTINE_OK;
        rstatus = scoreboard__get_IIS2DH_CTRL_REG4_full_scale_config_bits(&iis2dh_full_scale_setting);

//	switch (GET_CR4_FS())
	switch (iis2dh_full_scale_setting)
	{
	case ACC_FULL_SCALE_2G:   // CR4_FS_2G:
		x = (float)raw * (1.0f / 16.0f);
		break;
	case ACC_FULL_SCALE_4G:   // CR4_FS_4G:
		x = (float)raw * (2.0f / 16.0f);
		break;
	case ACC_FULL_SCALE_8G:   // CR4_FS_8G:
		x = (float)raw * (4.0f / 16.0f);
		break;
	case ACC_FULL_SCALE_16G:   // CR4_FS_16G:
		x = (float)raw * (12.0f / 16.0f);
		break;
	}
	return x;
}


float acc_raw_to_float_g(int16_t raw)
{
	return acc_raw_to_float_mg(raw) / 1000.0;
}





// (STEP 1 in code porting)

// calculate vrms from acceleration data
static float calc_acc_to_vrms(unsigned axis)
{
    unsigned i, n;
    unsigned idx_10Hz, idx_1kHz;

    arm_status s;
    static arm_rfft_fast_instance_f32 fft_inst;
    float *raw_vals, *fft_inp, *fft_out, *fft, *aps, *apsd, *vpsd;
    float power_time_domain;
    float power_freq_domain;
    float vrms;
    int8_t *acc_buf = GET_AXIS_BUF(axis);


// --- 1105 DEV BEGIN ---
// printk("Begin 1105 DEV work, acc_buf detected size is %u bytes,\n", sizeof(acc_buf));  ...pointer to array has itself size int.
#ifdef PULSE_DEV__VRMS_SHOW_RAW_DATA_BUFFER
    printk("top of calc_acc_to_vrms() acc_buf holds:\n\n");
    show_byte_buffer(acc_buf, FULL_READINGS_SET);
#endif
    uint32_t rstatus = copy_x_axis_data_to(acc_buf);

    if ( rstatus != ROUTINE_OK ) { printk("Routine 'copy x axis data' returns error or warning status\n"); }
#ifdef PULSE_DEV__VRMS_SHOW_RAW_DATA_BUFFER
    printk("after copy of latest x-axis readings acc_buf holds:\n\n");
    show_byte_buffer(acc_buf, FULL_READINGS_SET);
#endif

//    printk("returning early . . .\n\n");
//    return 0.0; 
// --- 1105 DEV END ---


    // convert samples to floats in 'g'
    raw_vals = alloc_f32_buf();
    for (i = 0; i < N; i++)
        raw_vals[i] = acc_raw_to_float_g((int16_t)acc_buf[i] << 8); // return value in units of 'g'

    // calc power time domain (for validation of Parseval’s Theorem)
    power_time_domain = 0.0;
    for (i = 0; i < N; i++)
        power_time_domain += sq(raw_vals[i]);
    power_time_domain = power_time_domain / (float)N;

    // apply Hann window function
    for (i = 0; i < N; i++)
        raw_vals[i] *= 0.5 * (1.0 - cosf(2 * PI * (float)i / (float)N));

    // Real FFT
    s = arm_rfft_fast_init_f32 // returns ARM_MATH_SUCCESS on success
        ( &fft_inst // arm_rfft_fast_instance_f32 * S -- [in,out] points to instance struct
        , (uint16_t)N // uint16_t fftLen -- [in] length of the Real Sequence (32, 64, 128, 256, 512, 1024, 2048, 4096)
        );
    fft_inp = handoff_f32_buf(&raw_vals);
    fft_out = alloc_f32_buf();
    arm_rfft_fast_f32
        ( &fft_inst // arm_rfft_fast_instance_f32 * S -- [in] points to an arm_rfft_fast_instance_f32 structure
        , fft_inp // float32_t * p -- [in] points to the input buffer
        , fft_out // float32_t * pOut -- [in] points to the output buffer
        , 0 // uint8_t ifftFlag -- [in] RFFT if flag is 0, RIFFT if flag is 1
        );
    free_f32_buf(&fft_inp);

    // unpack FFT output array, taking absolute values
    // see CMSIS docs... first two locations are real (0 and N/2), the rest are complex value pairs
    fft = alloc_f32_buf();
    fft[0] = fabsf(fft_out[0]);
    fft[N/2] = fabsf(fft_out[1]);
    for (i = 1; i < N/2; i++)
        fft[i] = absf_complex(fft_out[i*2+0], fft_out[i*2+1]);
    free_f32_buf(&fft_out);

    // normalize FFT values
    for (i = 0; i < N/2+1; i++)
        fft[i] /= (float)N;

    // acceleration power spectrum
    aps = alloc_f32_buf();
    for (i = 0; i < N/2+1; i++)
        aps[i] = 2.0 * sq(fft[i]);
    free_f32_buf(&fft);

    // calc power frequency domain (for validation of Parseval’s Theorem)
    power_freq_domain = 0.0;
    for (i = 0; i < N/2+1; i++)
        power_freq_domain += aps[i];

    // verify energy conservation (via Parseval’s Theorem)
#if 0 // - 2021-11-05 - disabled for now until determined needed - TMH
    NRF_LOG_INFO("data_model: power_time_domain=%d, power_freq_domain=%d, diff=%d"
        , (int)(power_time_domain * 1000.0)
        , (int)(power_freq_domain * 1000.0)
        , (unsigned)(fabsf(power_time_domain - power_freq_domain) * 1000.0)
        );
#endif
//    if (DBG) dbg_uart_printf("adm: power_time_domain=%.3f, power_freq_domain=%.3f, diff=%.3f, ratio=%.3f\n"
#if PULSE_DEV__SHOW_ORIGINAL_CALCULATION_SUMMARY == 1
    if (DBG) printk("adm: power_time_domain=%.3f, power_freq_domain=%.3f, diff=%.3f, ratio=%.3f\n"
        , power_time_domain
        , power_freq_domain
        , fabsf(power_time_domain - power_freq_domain)
        , (power_time_domain / power_freq_domain)
        );
#endif

    // acceleration power spectrum density
    apsd = alloc_f32_buf();
    for (i = 0; i < N/2+1; i++)
        apsd[i] = aps[i] / Fr;
    free_f32_buf(&aps);

    // bandpass filter (i.e. data selection)
    idx_10Hz = 4;
    idx_1kHz = 382;
    // num = idx_1kHz - idx_10Hz + 1;

    // velocity psd (from accel psd)
    vpsd = alloc_f32_buf();
    for (i = idx_10Hz; i <= idx_1kHz; i++)
        vpsd[i] = apsd[i] / sq(omega(i));
    free_f32_buf(&apsd);

    // convert velocity psd (from m/s^2 to in/sec^2)
    for (i = idx_10Hz; i <= idx_1kHz; i++)
        vpsd[i] *= (float)(386*386);

    // log/log integration of area under vspd curve
    vrms = 0.0;
    for (n = idx_10Hz; n <= idx_1kHz; n++)
    {
        float slope;
        // determine slope
        slope = logf(vpsd[n+1] / vpsd[n]) / logf(x_axis(n+1) / x_axis(n));
        // add to sum
        if (slope < -1.001 || slope > -0.999)
            vrms += (vpsd[n+1] * x_axis(n+1) - vpsd[n] * x_axis(n)) / (slope + 1.0);
        else
            vrms += vpsd[n] * x_axis(n) * logf(x_axis(n+1) / x_axis(n));
    }
    vrms = 2.0 * sqrtf(vrms) * 25.4; //25.4 is a in/s to mm/s, mobile app is looking for mm/s
    
    //compensate for the noise in the accelerometer at close to 0 in/s
    if(vrms < 0.2) // units of mm/s
    {
        vrms = 0.0;
    }

    free_f32_buf(&vpsd);

    return vrms;

} // end routine calc_acc_to_vrms()

// --- CODE COPY END ---



// Note:  Can't prototype statically declared funcdtions so creating a
//   second development routine section here . . .

//----------------------------------------------------------------------
// - SECTION - development routines, second group
//----------------------------------------------------------------------

// static float calc_acc_to_vrms(unsigned axis)

uint32_t on_event__readings_done__calculate_vrms(uint32_t event)
{
    uint32_t axis = 0; 
    uint32_t rstatus = 0;
// --- DEV 0208 ---
    char lbuf[SIZE_OF_MESSAGE_SHORT] = {0};
    uint32_t cycle_count = 0;
// --- DEV 0208 ---

    float vrms = calc_acc_to_vrms(axis);

    rstatus = scoreboard__set_vrms_value_in_integer_json_ready_form((uint32_t)(vrms * VRMS_POWER_OF_TEN_SCALING_FOR_JSON));

    if ( rstatus == ROUTINE_OK )
    {
#ifdef DEV_0808__SCOREBOARD_AVAILABLE
        rstatus = scoreboard__update_flag(DM__VRMS_CALCULATION_COMPLETE, SET_FLAG); 
#else
        printk("- DEV 0808 - calculating vRMS without scoreboard module.\n");
#endif
    }

#if PULSE_DEV__SHOW_LATEST_VRMS_VALUE == 1
    printk("- 1105 - calculated VRMS of %f\n", vrms);
#endif
// --- DEV 0208 ---
//    snprintf(lbuf, sizeof(lbuf), "data model vRMS: %f\n\r", vrms); 
    cycle_count = k_cycle_get_32();
    snprintf(lbuf, sizeof(lbuf), "kernel cycles at %08u, data model vRMS: %f\n\r", cycle_count, vrms); 
#if 0
    printk_cli(lbuf);
#else
    printk("%s", lbuf);
#endif
// --- DEV 0208 ---

#if PULSE_DEV__VRMS_VALUES_TO_CLI_UART == 1
    char lbuf[SIZE_OF_MESSAGE_SHORT];  // eighty characters as of 2022-02-08 - TMH
    snprintf(lbuf, "vrms:  %f\n\r", vrms);
#if 0
    printk_cli(lbuf);
#else
    printk("%s", lbuf);
#endif
#endif

// --- DEV 1205 ---
// Here test latest vrms value against present alert-able threshold, set alert accordingly:
// ...or create a test in pulse-pro-alerts.c, for better functionality factoring.
// --- DEV 1205 ---

    return 0;
}


// --- EOF ---
