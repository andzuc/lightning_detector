#ifndef SAMPLING_
#define SAMPLING_

#include <stdint.h>
#include "util_math.h"

typedef struct
{
  uint16_t deltat;
  int16_t value;
} sample;

typedef struct
{
  sample (*data)[];
  uint16_t next;
  uint8_t size;
  uint16_t maxSamples; // 2^size
  long firstTime;
  long lastTime;
  //int16_t zeroValue;
  uint16_t nspikes;
  uint16_t ncross;
  int32_t mwSumLast;
  int32_t mwSum;
  int16_t meanLast;
  int16_t mean;
  int32_t mwSqSumLast;
  int32_t mwSqSum;
  int32_t variance;
} sampling;

/*
  if b is a power of 2 then a%b can be optimized
  https://stackoverflow.com/questions/11040646/faster-modulus-in-c-c
 */
#define fastMod(a,b) ((a)&((b)-1))
#define roundDiv(a,b) ((fastMod(a,b)*2<=(b))?((a)>>(b)):(((a)>>(b))+1))

#define getSampleImpl(s,i) ((*(s).data)[(i)])
#define addSample(s,d,t) (((s).next < (s).maxSamples) ?			\
			  (&((getSampleImpl((s),(s).next++))=((sample){(t),(d)}))) : \
			  (((s).next = 0),(&((getSampleImpl((s),(s).next++))=((sample){(t),(d)})))))
/*
  address = (next+i) % maxSamples
*/
#define getSample(s,i) (getSampleImpl(s,(fastMod((i)+(s).next,(s).maxSamples))))
#define getFirst(s) (getSample((s),0).value)
#define getLast(s) (getSample((s),(s).maxSamples-1).value)
#define getLastMinusOne(s) (getSample((s),(s).maxSamples-2).value)

#define hasSpike(s) (ABS(getLast(s) - getLastMinusOne(s)) > SPIKE)
#define zeroCross(s) (getLast(s) >= (s).mean && getLastMinusOne(s) < (s).mean)
#define zeroDelta(s) (ABS(getLast(s) - (s).mean))

#define updSum(s) (((s).mwSumLast = (s).mwSum),				\
		   ((s).mwSum = (s).mwSumLast + getLast(s) - getFirst(s)))
#define updMean(s) (((s).meanLast = roundDiv((s).mwSumLast, (s).size)), \
		    ((s).mean = roundDiv((s).mwSum, (s).size)))

/*
  0) n*VAR = n*VARLAST + ((LAST - MEAN)^2 - (FIRST - MEAN?)^2)
  Using a^2 - b^2 = (a+b) * (a-b), we have:
  1) n*VAR = n*VARLAST + ((LAST+FIRST-2*MEAN) * (LAST-FIRST))
 */
#define updSqSum(s) (((s).mwSqSumLast = (s).mwSqSum),			\
		     ((s).mwSqSum = (s).mwSqSumLast +			\
		      (getLast(s) + getFirst(s) - ((s).mean<<2)) *	\
		      (getLast(s) - getFirst(s))))
#define updVariance(s) ((s).variance = roundDiv((s).mwSqSum, (s).size))

void reset(sampling* s, long t);
long getFirstTime(sampling* s);

#endif // SAMPLING_
