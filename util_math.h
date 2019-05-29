#ifndef UTIL_MATH_
#define UTIL_MATH_
#include <stdint.h>

/* Fast RMS calculaion */
/* https://stackoverflow.com/a/28812301 */
extern uint16_t rms_filter(uint16_t sample);

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr)&=~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr)|=_BV(bit))
#endif

// software reset
void (*resetMCU)(void);

// number of elements in array x
#define LENGTH(A) (sizeof(A)/sizeof((A)[0]))
#define ABS(N) ((N<0)?(-N):(N))

#endif // UTIL_MATH_
