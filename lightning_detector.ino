extern "C" {
#include "util_math.h"
}

#define FASTADC 1
#define SAMPLES 512
#define SPIKE 20

typedef struct
{
  unsigned char deltat;
  int16_t value;
} sample;

typedef struct
{
  long firstTime;
  long lastTime;
  sample (*data)[SAMPLES];
  int count;
  uint16_t rms;
  //rms_filter_status* rms_status;
  bool send;
} sampling;

#define getSample(s,i) ((*(s).data)[(i)])
#define addSample(s,d,t) (((s).count < SAMPLES) ?			\
			  &((getSample((s),(s).count++))=((sample){(t),(d)})) : NULL)
#define hasSpike(s) (((s).count >= 2) ?					\
		     (ABS(getSample(s,(s).count-1).value -		\
			  getSample(s,(s).count-2).value) > SPIKE) : false)

static void reset(sampling* s);
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
  //analogReference(INTERNAL);

  size_t dataSize=sizeof(sample[SAMPLES]);
  Serial.print("; dataSize ");
  Serial.println(dataSize);
  delay(3000);
  //buffer.rms_status=new_rms_filter(512,SAMPLES);
  buffer.data=(sample(*)[SAMPLES])malloc(dataSize);
  if(buffer.data==NULL)
    {
      Serial.println("malloc failed: resetting");
      delay(5000);
      resetMCU();
    }
}

void loop()
{
  long sampleTime=micros();
  if(buffer.count==0) buffer.firstTime=buffer.lastTime=sampleTime;
  sample* s=addSample(buffer,analogRead(A0),(unsigned byte)(sampleTime-buffer.lastTime));
  //buffer.rms=rms_filter(s->value);
  buffer.lastTime=micros();

  if(!buffer.send && hasSpike(buffer)) buffer.send=true;
  if(buffer.send&&!s) send(&buffer);
  if(!s) reset(&buffer);
}

void reset(sampling* s)
{
  s->count=0;
  s->send=false;
}

void send(sampling* s)
{
  digitalWrite(LED_BUILTIN, LOW);
  long timeSpan=(s->lastTime)-(s->firstTime);
  long sampleRate=(s->count)*1E6l/timeSpan;
  Serial.print("; Sample Rate ");
  Serial.println(sampleRate);
  Serial.println("; Channels 1");
  Serial.print("; RMS ");
  Serial.println(s->rms);
  long t=s->firstTime;
  for (int i=0; i<(s->count); i++){
    Serial.print(t);
    Serial.print("\t");
    double v=((double)getSample(*s,i).value-512)/512;   
    Serial.print(v);
    Serial.print("\t");
    Serial.println(getSample(*s,i).value);
    t+=getSample(*s,i).deltat;
  }
  digitalWrite(LED_BUILTIN, HIGH);
}
