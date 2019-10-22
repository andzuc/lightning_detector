#include <math.h>

extern "C" {
#include "util_math.h"
#include "sampling.h"
}

//#define DEBUG
#define FASTADC 1
#define SAMPLES_SIZE 8
#define SPIKE 10
#define SPIKEN 5
#define MAXSTDDEV 0
#define MAXVAR 576
#define SENSFREQ 0

static void send(sampling* s);

sampling buffer;

void setup() {
#if FASTADC
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
#endif

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0,INPUT);
  pinMode(A7,INPUT);
  analogReference(INTERNAL);

  size_t dataSize=sizeof(sample[1<<SAMPLES_SIZE]);
  delay(3000);
  buffer.data=(sample (*)[])malloc(dataSize);
  if(buffer.data==NULL)
    {
      Serial.println("malloc failed: resetting");
      delay(5000);
      resetMCU();
    }
  memset(buffer.data,0,dataSize);
  buffer.size=SAMPLES_SIZE;
  buffer.maxSamples=1<<SAMPLES_SIZE;
  reset(&buffer,micros());
}

void loop()
{
#ifdef DEBUG
  int last=(((255)+(buffer).next) & ((buffer).maxSamples-1));
  Serial.print("last:");
  Serial.print(last);
  Serial.print(" vlast:");
  Serial.print((*(buffer.data))[last].value);
#endif

  //buffer.firstTime+=getSample(buffer,0).deltat;
  int16_t value=analogRead(A0);
  long sampleTime=micros();
  sample* s=addSample(buffer,value);
  if(buffer.next==1) buffer.firstTime=sampleTime;
  buffer.lastTime=sampleTime;
  
#ifdef DEBUG
  Serial.print(" vcur:");
  Serial.print(value);
  Serial.print(" firstT:");
  Serial.print(buffer.firstTime);
  Serial.print(" lastT:");
  Serial.println(buffer.lastTime);
#endif

  updSum(buffer);
  updMean(buffer);
  updSqSum(buffer);
  updVariance(buffer);
  //if(zeroCross(buffer)) ++buffer.ncross;

  /* if(hasSpike(buffer)) */
  /*   { */
  /*     ++buffer.nspikes; */
  /*     if(buffer.nspikes>SPIKEN */
  /*     	 && buffer.next==buffer.maxSamples) */
  /*     	send(&buffer); */
  /*   } */

  if(buffer.sendIdx>0)
    {
      if(buffer.next==buffer.sendIdx) buffer.sendIdx=0;
    }
  else
    {
      if(buffer.variance>MAXVAR)
	{
	  buffer.sendIdx=buffer.next;
	  send(&buffer);
	}
    }
}

void send(sampling* s)
{
  long timeSpan=(s->lastTime)-(s->firstTime);
  long f0=1E6l/timeSpan;
  long sampleRate=(s->next)*f0;
  long sensFreq=(s->ncross)*f0;
  double stdDev=sqrt(MAX(s->variance,0));
  if(sensFreq<SENSFREQ) return;
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("; Sample Rate ");
  Serial.println(sampleRate);
  Serial.println("; Channels 1");
  Serial.print("; firstTime: ");
  Serial.println(s->firstTime);
  Serial.print("; lastTime: ");
  Serial.println(s->lastTime);
  Serial.print("; timeSpan ");
  Serial.println(timeSpan);
  Serial.print("; next ");
  Serial.println(s->next);
  Serial.print("; nspikes ");
  Serial.println(s->nspikes);
  Serial.print("; ncross ");
  Serial.println(s->ncross);
  Serial.print("; sensFreq ");
  Serial.println(sensFreq);
  Serial.print("; mean ");
  Serial.println(getLast(*s).mean);
  Serial.print("; variance ");
  Serial.println(s->variance);
  Serial.print("; stdDev ");
  Serial.println(stdDev);
  long t=(s->firstTime);
  uint16_t dt=roundDiv(timeSpan,(s->size));
  for (uint16_t i=0; i<(s->maxSamples); i++){
    sample sc=getSample(*s,i);
    // time in micros
    if(i>0) t+=dt;
    Serial.print(t);
    // normalized value: -1 <= v <=1
    Serial.print("\t");
    double v=((double)sc.value-512)/512;   
    Serial.print(v);
    // raw value
    Serial.print("\t");
    Serial.print(sc.value);
    // raw deltat
    Serial.print("\t");
    Serial.println(dt);
  }
  s->nspikes=0;
  digitalWrite(LED_BUILTIN, LOW);
}
