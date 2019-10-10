#ifndef SAMPLING_
#define SAMPLING_

#include <stdint.h>
#include "util_math.h"

// number of elements in array x
#define LENGTH(A) (sizeof(A)/sizeof((A)[0]))
#define ABS(N) ((N<0)?(-N):(N))

typedef struct
{
  uint16_t deltat;
  int16_t value;
} sample;

typedef struct
{
  sample (*data)[];
  uint16_t next;
  uint16_t maxSamples;
  long firstTime;
  long lastTime;
  //int16_t zeroValue;
  uint16_t nspikes;
  uint16_t ncross;
  int16_t meanLast;
  int16_t mean;
  uint32_t variance;
} sampling;


/*
  address = (next+i) % maxSamples, but if maxsamples is a power of two then optimize
  https://stackoverflow.com/questions/11040646/faster-modulus-in-c-c
*/
#define getSampleImpl(s,i) ((*(s).data)[(i)])
#define addSample(s,d,t) (((s).next < (s).maxSamples) ?			\
			  (&((getSampleImpl((s),(s).next++))=((sample){(t),(d)}))) : \
			  (((s).next = 0),(&((getSampleImpl((s),(s).next++))=((sample){(t),(d)})))))
#define getSample(s,i) (getSampleImpl(s,(((i) + (s).next) & ((s).maxSamples-1))))
#define getLast(s) (getSample((s),(s).maxSamples-1).value)
#define getLastMinusOne(s) (getSample((s),(s).maxSamples-2).value)
#define hasSpike(s) (ABS(getLast(s) - getLastMinusOne(s)) > SPIKE)
#define zeroCross(s) (getLast(s) >= (s).mean && getLastMinusOne(s) < (s).mean)
#define zeroDelta(s) (ABS(getLast(s) - (s).mean))
#define updMean(s) (((s).meanLast = (s).mean),				\
		    ((s).mean = (s).meanLast + (getLast(s) - (s).meanLast)/(s).maxSamples))
#define updVariance(s) ((s).variance += ((getLast(s)-(s).meanLast) * (getLast(s)-(s).mean)))

void reset(sampling* s, long t);
long getFirstTime(sampling* s);

#endif // SAMPLING_
