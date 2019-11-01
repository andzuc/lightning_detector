#ifndef SAMPLING_
#define SAMPLING_

#include <stdint.h>
#include <stdlib.h>
#include "util_math.h"

typedef struct
{
  int16_t value;
  int16_t mean;
} sample;

typedef struct
{
  sample (*data)[];
  uint16_t next;
  uint8_t log2size;
  uint16_t maxSamples; // 2^log2size
  long firstTime;
  long lastTime;
  long lastSend;
  //int16_t zeroValue;
  uint16_t nspikes;
  uint16_t ncross;
  int32_t mwSumLast;
  int32_t mwSum;
  int32_t mwSqSumLast;
  int32_t mwSqSum;
  int32_t variance;
  uint16_t sendIdx;
} sampling;

/*
  if b is a power of 2 then a%b can be optimized
  https://stackoverflow.com/questions/11040646/faster-modulus-in-c-c
 */
#define fastMod(a,b) ((a)&((b)-1))
/*
  roundDiv(a,q): (a mod bq<=(q/2)?(a/q):((a/q)+1)
  optimized: b is the log2 of the divisor q
 */
#define roundDiv(a,b) (((fastMod(a,(1<<(b)))<<1)<=(1<<b))?((a)>>(b)):(((a)>>(b))+1))

#define getSampleImpl(s,i) ((*(s).data)[(i)])
#define addSample(s,d) (((s).next < (s).maxSamples) ?			\
			(&((getSampleImpl((s),(s).next++))=((sample){(d),0}))) : \
			(((s).next = 0),(&((getSampleImpl((s),(s).next++))=((sample){(d),0})))))
/*
  address = (next+i) % maxSamples
*/
#define getSample(s,i) (getSampleImpl(s,(fastMod((i)+(s).next,(s).maxSamples))))
#define getFirst(s) (getSample((s),0))
#define getLast(s) (getSample((s),(s).maxSamples-1))
#define getLastMinusOne(s) (getSample((s),(s).maxSamples-2))

#define hasSpike(s) (ABS(getLast(s).value - getLastMinusOne(s).value) > SPIKE)
#define zeroCross(s) (getLast(s).value >= getLast(s).mean && \
		      getLastMinusOne(s).value < getLast(s).mean)
#define zeroDelta(s) (ABS(getLastSample(s).value - getLastSample(s).mean))

#define updSum(s) (((s).mwSumLast = (s).mwSum),				\
		   ((s).mwSum = (s).mwSumLast + getLast(s).value - getFirst(s).value))
#define updMean(s) ((getLastMinusOne(s).mean = roundDiv((s).mwSumLast, (s).log2size)), \
		    (getLast(s).mean = roundDiv((s).mwSum, (s).log2size)))

/*
  0) n*VAR = n*VARLAST + ((LAST - MEAN_LAST)^2 - (FIRST - MEAN_FIRST)^2)
  Using a^2 - b^2 = (a+b) * (a-b), we have:
  1) n*VAR = n*VARLAST + ((LAST - MEAN_LAST + FIRST - MEAN_FIRST) * (LAST - MEAN_LAST - FIRST + MEAN_FIRST))
 */
#define updSqSum(s) (((s).mwSqSumLast = (s).mwSqSum),			\
		     ((s).mwSqSum = (s).mwSqSumLast +			\
		      (getLast(s).value - getLast(s).mean + getFirst(s).value - getFirst(s).mean) * \
		      (getLast(s).value - getLast(s).mean - getFirst(s).value + getFirst(s).mean)))
#define updVariance(s) ((s).variance = roundDiv((s).mwSqSum, (s).log2size))

sampling* sampling_ctor(long t, uint8_t log2size, void (*out)(const char*));
void sampling_reset(sampling* s, long t);

#endif // SAMPLING_
