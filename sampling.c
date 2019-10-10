#include "sampling.h"

void reset(sampling* s, long t)
{
  s->next=0;
  s->firstTime=t;
  s->lastTime=t;
  s->nspikes=0;
  s->ncross=0;
  s->meanLast=0;
  s->mean=0;
  s->variance=0;
}

long getFirstTime(sampling* s)
{
  long firstTime=s->lastTime;
  for (uint16_t i=0; i<(s->maxSamples); i++){
    firstTime-=getSample(*s,i).deltat;
  }
  return firstTime;
}
