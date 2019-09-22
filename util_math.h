#ifndef UTIL_MATH_
#define UTIL_MATH_
#include <stdint.h>

// number of elements in array x
#define LENGTH(A) (sizeof(A)/sizeof((A)[0]))
#define ABS(N) ((N<0)?(-N):(N))

// boolean
#define bool uint8_t
#define true 1
#define false 0

/* Fast RMS calculaion */
/* https://stackoverflow.com/a/28812301 */
uint16_t rms_filter(uint16_t sample);

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr)&=~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr)|=_BV(bit))
#endif

// software reset
void (*resetMCU)(void);

#endif // UTIL_MATH_
