#include <math.h>

extern "C" {
#include "util_math.h"
#include "sampling.h"
}

#define FASTADC 1
#define SAMPLES 256
#define SPIKE 50
#define SENSFREQ 8000

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

  size_t dataSize=sizeof(sample[SAMPLES]);
  delay(3000);
  buffer.data=(sample (*)[])malloc(dataSize);
  if(buffer.data==NULL)
    {
      Serial.println("malloc failed: resetting");
      delay(5000);
      resetMCU();
    }
  buffer.maxsamples=SAMPLES;
}

void loop()
{
  long sampleTime=micros();
  if(buffer.count==0)
    {
      buffer.firstTime=buffer.lastTime=sampleTime;
      buffer.zeroValue=analogRead(A7);
    }
  sample* s=addSample(buffer,analogRead(A0),(unsigned byte)(sampleTime-buffer.lastTime));
  updVariance(buffer);
  if(zeroCross(buffer)) ++buffer.ncross;
  buffer.lastTime=micros();
  
  if(hasSpike(buffer))
    {
      ++buffer.nspikes;
      if(buffer.nspikes>4 && !buffer.send) buffer.send=true;
    }
  if(buffer.send&&!s) send(&buffer);
  if(!s) reset(&buffer);
}

void send(sampling* s)
{
  long timeSpan=(s->lastTime)-(s->firstTime);
  long f0=1E6l/timeSpan;
  long sampleRate=(s->count)*f0;
  long sensFreq=(s->ncross)*f0;
  double stdDev=sqrt((s->variance)/(s->count));
  if(sensFreq<SENSFREQ) return;
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("; Sample Rate ");
  Serial.println(sampleRate);
  Serial.println("; Channels 1");
  Serial.print("; timeSpan ");
  Serial.println(timeSpan);
  Serial.print("; zeroValue ");
  Serial.println(s->zeroValue);
  Serial.print("; nspikes ");
  Serial.println(s->nspikes);
  Serial.print("; ncross ");
  Serial.println(s->ncross);
  Serial.print("; sensFreq ");
  Serial.println(sensFreq);
  Serial.print("; stdDev ");
  Serial.println(stdDev);
  long t=s->firstTime;
  for (uint16_t i=0; i<(s->count); i++){
    Serial.print(t);
    Serial.print("\t");
    double v=((double)getSample(*s,i).value-512)/512;   
    Serial.print(v);
    Serial.print("\t");
    Serial.println(getSample(*s,i).value);
    t+=getSample(*s,i).deltat;
  }
  digitalWrite(LED_BUILTIN, LOW);
}
