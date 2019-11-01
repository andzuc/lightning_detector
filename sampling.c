#include <string.h>
#include "sampling.h"


sampling* sampling_ctor(long t,uint8_t log2size, void (*out)(const char*))
{
  size_t samplingSize=sizeof(sampling);
  sampling* s=(sampling*)malloc(samplingSize);
  if(s==NULL)
    {
      out("malloc failed: resetting");
      return NULL;
    }
  size_t dataSize=sizeof(sample[1<<log2size]);
  s->data=(sample (*)[])malloc(dataSize);
  if(s->data==NULL)
    {
      out("malloc failed: resetting");
      return NULL;
    }
  s->log2size=log2size;
  s->maxSamples=1<<log2size;
  return s;
}

void sampling_reset(sampling* s, long t)
{
  size_t dataSize=sizeof(sample[1<<(s->log2size)]);
  memset(s->data,0,dataSize);
  s->next=0;
  s->firstTime=t;
  s->lastTime=t;
  s->lastSend=t;
  s->nspikes=0;
  s->ncross=0;
  s->mwSumLast=0;
  s->mwSum=0;
  s->mwSqSumLast=0;
  s->mwSqSum=0;
  s->variance=0;
  s->sendIdx=0;
}
