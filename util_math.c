#include "util_math.h"

#define INITIAL 512  /* Initial value of the filter memory. */
#define SAMPLES 512

uint16_t rms_filter(uint16_t sample)
{
  static uint16_t rms = INITIAL;
  static uint32_t sum_squares = 1UL * SAMPLES * INITIAL * INITIAL;

  sum_squares -= sum_squares / SAMPLES;
  sum_squares += (uint32_t) sample * sample;
  if (rms == 0) rms = 1;    /* do not divide by zero */
  rms = (rms + sum_squares / SAMPLES / rms) / 2; /* Newton–Raphson sqrt */
  return rms;
}
