#include "sampling.h"

void reset(sampling* s)
{
  s->count=0;
  s->nspikes=0;
  s->ncross=0;
  s->variance=0;
  s->send=false;
}
