#include <math.h>

extern "C" {
#include "util_math.h"
#include "sampling.h"
#include <fastadc.h>
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
static int16_t analogReadA0(void);
static bool send_trigger(sampling* s);
static void send_proc(sampling* s);
static void serial_print(const char* s);

sampling* buffer;

void setup() {
#if FASTADC
  // set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
#endif
  ADCSRA |= _BV(ADEN);    // Enable the ADC

  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(A0,INPUT);
  analogReference(INTERNAL);

  ADMUX = 0;         // use #1 ADC
  ADMUX |= _BV(REFS0) | _BV(REFS1);    // use internal 1.1V Voltage Reference
  ADMUX &= (~_BV(ADLAR));   // clear for 10 bit resolution (default)

  Serial.print("ADMUX: ");
  Serial.println(ADMUX);
  Serial.print("ADCSRA: ");
  Serial.println(ADCSRA);
  Serial.println("sampling setup");
  buffer=sampling_ctor(micros,				\
		       ADCsingleREAD_10bit,		\
		       send_trigger,			\
		       send_proc,			\
		       SAMPLES_SIZE,			\
		       serial_print);
}

void loop()
{
  sampling_acquire(buffer);
}

int16_t analogReadA0(void)
{
  return analogRead(A0);
}

void serial_print(const char* s)
{
  Serial.print(s);
}

bool send_trigger(sampling* s)
{
  return (s->variance)>MAXVAR			\
    || (s->lastTime)-(s->lastSend)>30E6l;
}

void send_proc(sampling* s)
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
  uint16_t dt=timeSpan/(s->next);
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
  s->lastSend=micros();
  digitalWrite(LED_BUILTIN, LOW);
}
