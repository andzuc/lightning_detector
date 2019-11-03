#include <string.h>
#include "sampling.h"


sampling* sampling_ctor(unsigned long (*get_micros)(void),		\
			int16_t (*get_sample)(void),			\
			bool (*send_trigger)(sampling*),		\
			void (*send_proc)(sampling*),			\
			uint8_t log2size,				\
			void (*out)(const char*))
{
  size_t samplingSize=sizeof(sampling);
  sampling* s=(sampling*)malloc(samplingSize);
  if(s==NULL)
    {
      out("sampling_ctor: malloc failed: resetting\n");
      return NULL;
    }
  size_t dataSize=sizeof(sample[1<<log2size]);
  s->data=(sample (*)[])malloc(dataSize);
  if(s->data==NULL)
    {
      out("sampling_ctor: malloc failed: resetting\n");
      return NULL;
    }
  s->log2size=log2size;
  s->maxSamples=1<<log2size;
  
  s->get_micros=get_micros;
  s->get_sample=get_sample;
  s->send_trigger=send_trigger;
  s->send_proc=send_proc;
  s->out=out;
  
  sampling_reset(s);
  out("sampling_ctor: starting\n");
  return s;
}

void sampling_reset(sampling* s)
{
  unsigned long t=s->get_micros();
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

void sampling_acquire(sampling* s)
{
#ifdef SAMPLING_DEBUG
  int maxidx=(s->maxSamples)-1;
  int last=((maxidx+(s->next)) & maxidx);
  s->out("last:");
  s->out(last);
  s->out(" vlast:");
  s->out(s->data[last].value);
#endif

  int16_t value=s->get_sample();
  long sampleTime=s->get_micros();
  addSample(*s,value);
  if(s->next==1) s->firstTime=sampleTime;
  s->lastTime=sampleTime;
  
#ifdef SAMPLING_DEBUG
  s->out(" vcur:");
  s->out(value);
  s->out(" firstT:");
  s->out(buffer.firstTime);
  s->out(" lastT:");
  s->out(buffer.lastTime);
  s->out("\n");
#endif

  updSum(*s);
  updMean(*s);
  updSqSum(*s);
  updVariance(*s);
  //if(zeroCross(buffer)) ++buffer.ncross;

  /* if(hasSpike(buffer)) */
  /*   { */
  /*     ++buffer.nspikes; */
  /*     if(buffer.nspikes>SPIKEN */
  /*     	 && buffer.next==buffer.maxSamples) */
  /*     	send(&buffer); */
  /*   } */

  if((s->sendIdx)>0)
    {
      if(s->next==s->sendIdx) s->sendIdx=0;
    }
  else
    {
      if(s->send_trigger(s))
	{
	  s->sendIdx=s->next;
	  s->send_proc(s);
	}
    }
}
