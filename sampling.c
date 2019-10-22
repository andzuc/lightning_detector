#include "sampling.h"

void reset(sampling* s, long t)
{
  s->next=0;
  s->firstTime=t;
  s->lastTime=t;
  s->nspikes=0;
  s->ncross=0;
  s->mwSumLast=0;
  s->mwSum=0;
  s->mwSqSumLast=0;
  s->mwSqSum=0;
  s->variance=0;
  s->sendIdx=0;
}
