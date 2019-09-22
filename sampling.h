#ifndef SAMPLING_
#define SAMPLING_

#include <stdint.h>
#include "util_math.h"

// number of elements in array x
#define LENGTH(A) (sizeof(A)/sizeof((A)[0]))
#define ABS(N) ((N<0)?(-N):(N))

typedef struct
{
  uint8_t deltat;
  int16_t value;
} sample;

typedef struct
{
  sample (*data)[];
  uint16_t count;
  uint16_t maxsamples;
  long firstTime;
  long lastTime;
  int16_t zeroValue;
  uint16_t nspikes;
  uint16_t ncross;
  uint32_t variance;
  bool send;
} sampling;

#define getSample(s,i) ((*(s).data)[(i)])
#define addSample(s,d,t) (((s).count < (s).maxsamples) ?		\
			  &((getSample((s),(s).count++))=((sample){(t),(d)})) : NULL)
#define hasSpike(s) (((s).count >= 2) ?					\
		     (ABS(getSample((s),(s).count).value -		\
			  getSample((s),(s).count-1).value) > SPIKE) : false)
#define zeroCross(s) (((s).count >= 2) ?				\
		      (getSample((s),(s).count).value >= (s).zeroValue && \
		       getSample((s),(s).count-1).value < (s).zeroValue ) : false)
#define zeroDelta(s) (ABS(getSample((s),(s).count) - (s).zeroValue))
#define updVariance(s) (((s).count >= 1) ?				\
			((s).variance += getSample((s),(s).count).value^2) : \
			0)

void reset(sampling* s);

#endif // SAMPLING_
