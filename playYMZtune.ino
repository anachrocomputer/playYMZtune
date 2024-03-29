/* playYMZtune --- play a tune on the YMZ284 sound chip             2017-02-15 */
/* Copyright (c) 2017 John Honniball                                           */

#define CS_PIN    (2)
#define CLK_PIN   (3)
#define WR_PIN    (4)
#define A0_PIN    (5)
#define D0_PIN    (6)

#define TONEPERIODA_REG     (0)
#define TONEPERIODB_REG     (2)
#define TONEPERIODC_REG     (4)
#define NOISEPERIOD_REG     (6)
#define ENABLE_REG          (7)
#define AMPLITUDEA_REG      (8)
#define AMPLITUDEB_REG      (9)
#define AMPLITUDEC_REG      (10)
#define ENVELOPEPERIOD_REG  (11)
#define ENVELOPEMODE_REG    (13)
#define IOPORTA_REG         (14)  // AY-3-8910 only; not present on YMZ284
#define IOPORTB_REG         (15)

#define CHANNELA  (0)
#define CHANNELB  (1)
#define CHANNELC  (2)

#define IOPORTA   (0)
#define IOPORTB   (1)

#define AMPLITUDE_ENV (16)
#define AMPLITUDE_MAX (15)
#define AMPLITUDE_OFF (0)

#define ENV_CONT    (8)
#define ENV_ATTACK  (4)
#define ENV_ALT     (2)
#define ENV_HOLD    (1)

#define D2  38

#define E2  40

#define C3  48

#define F3  53
#define F3s 54
#define G3  55
#define G3s 56
#define A3  57
#define A3s 58
#define B3  59
#define C4  60
#define C4s 61
#define D4  62
#define D4s 63
#define E4  64
#define F4  65
#define F4s 66
#define G4  67
#define B4  71

int tp[] = {//Frequencies related to MIDI note numbers
  15289, 14431, 13621, 12856, 12135, 11454, 10811, 10204,//0-o7
  9631, 9091, 8581, 8099, 7645, 7215, 6810, 6428,//8-15
  6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050,//16-23
  3822, 3608, 3405, 3214, 3034, 2863, 2703, 2551,//24-31
  2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607,//32-39
  1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012,//40-47
  956, 902, 851, 804, 758, 716, 676, 638,//48-55
  602, 568, 536, 506, 478, 451, 426, 402,//56-63
  379, 358, 338, 319, 301, 284, 268, 253,//64-71
  239, 225, 213, 201, 190, 179, 169, 159,//72-79
  150, 142, 134, 127, 119, 113, 106, 100,//80-87
  95, 89, 84, 80, 75, 71, 67, 63,//88-95
  60, 56, 53, 50, 47, 45, 42, 40,//96-103
  38, 36, 34, 32, 30, 28, 27, 25,//104-111
  24, 22, 21, 20, 19, 18, 17, 16,//112-119
  15, 14, 13, 13, 12, 11, 11, 10,//120-127
  0//off
};

uint8_t EnableReg = 0;

void setup(void)
{
  int i;

  Serial.begin(9600);

  initPSG();

  setTonePeriod(CHANNELA, 142);  // (2000000 / 32) / frequency
  setNoisePeriod(15);
#ifdef ENVELOPE
  setToneEnable(CHANNELA, true);
  setAmplitude(CHANNELA, AMPLITUDE_ENV);
  setAmplitude(CHANNELB, AMPLITUDE_OFF);  
  setAmplitude(CHANNELC, AMPLITUDE_OFF);
#else
  setToneEnable(CHANNELA, true);
  setToneEnable(CHANNELB, true);
  setToneEnable(CHANNELC, true);
  setAmplitude(CHANNELA, AMPLITUDE_ENV);
  setAmplitude(CHANNELB, AMPLITUDE_ENV);  
  setAmplitude(CHANNELC, AMPLITUDE_ENV);
#endif
  setEnvelopePeriod(1024);
#ifdef ENVELOPE
  setEnvelopeMode(ENV_CONT | ENV_ALT);
#else
  //setEnvelopeMode(0);
#endif
}

void loop(void)
{
#ifdef ENVELOPE
  int ana;

  ana = analogRead(0);

  setEnvelopePeriod(ana);

  ana = analogRead(1);
  
  setTonePeriod(CHANNELA, ana);

  delay(20);
#else
  setnote(0, D2);
  setnote(1, F3s);
  setnote(2, E4);
  setEnvelopePeriod(2000);
  setEnvelopeMode(0);

  delay(150);

  setnote(0, D2);
  setnote(1, F3s);
  setnote(2, E4);
  setEnvelopePeriod(2000);
  setEnvelopeMode(0);

  delay(150);

  delay(150);

  setnote(0, D2);
  setnote(1, F3s);
  setnote(2, E4);
  setEnvelopePeriod(2000);
  setEnvelopeMode(0);

  delay(150);

  delay(150);

  setnote(0, D2);
  setnote(1, F3s);
  setnote(2, C4);
  setEnvelopePeriod(2000);
  setEnvelopeMode(0);

  delay(150);

  setnote(0, D2);
  setnote(1, F3s);
  setnote(2, E4);
  setEnvelopePeriod(2000);
  setEnvelopeMode(0);

  delay(150);

  delay(150);

  setnote(0, G3);
  setnote(1, B3);
  setnote(2, G4);
  setEnvelopePeriod(2000);
  setEnvelopeMode(0);

  delay(150);
  
  delay(2000);
#endif
}

void setnote(const int chan, const int midi)
{
  setTonePeriod(chan, tp[midi]);
}


/* initPSG --- initialise chip and disable all noise and tone channels */

void initPSG(void)
{
  int i;

  // Generate 2MHz clock on Pin 3
  pinMode(CLK_PIN, OUTPUT);
  TCCR2A = 0x23;
  TCCR2B = 0x09;
  OCR2A = 7;
  OCR2B = 1;

  delay(10);
  
  // Chip Select pin
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  // Write pin
  pinMode(WR_PIN, OUTPUT);
  digitalWrite(WR_PIN, HIGH);

  // A0 pin
  pinMode(A0_PIN, OUTPUT);
  digitalWrite(A0_PIN, LOW);

  // D0-D7 pins
  for (i = D0_PIN; i < (D0_PIN + 8); i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  
  EnableReg = 0x3f; // All noise and tone channels disabled

  aywrite(ENABLE_REG, EnableReg);
  aywrite(IOPORTB_REG, 0);
}


/* setToneEnable --- enable or disable tone generation on a given channel */

void setToneEnable(const int channel, const int enable)
{
  const uint8_t mask = 1 << channel;

  if (enable) {
    EnableReg &= ~mask;
  }
  else {
    EnableReg |= mask;
  }

  aywrite(ENABLE_REG, EnableReg);
}


/* setNoiseEnable --- enable or disable noise generation on a given channel */

void setNoiseEnable(const int channel, const int enable)
{
  const uint8_t mask = 1 << (3 + channel);

  if (enable) {
    EnableReg &= ~mask;
  }
  else {
    EnableReg |= mask;
  }

  aywrite(ENABLE_REG, EnableReg);
}


/* setPortDirection --- set I/O port to input or output */

void setPortDirection(const int channel, const int direction)
{
  const uint8_t mask = 1 << (6 + channel);

  if (direction == INPUT) {
    EnableReg &= ~mask;
  }
  else if (direction == OUTPUT) {
    EnableReg |= mask;
  }

  aywrite(ENABLE_REG, EnableReg);
}


/* setPortOutputs --- send a byte to the output port pins */

void setPortOutputs(const int channel, const int val)
{
  aywrite(IOPORTA_REG + channel, val);
}


/* setAmplitude --- set amplitude for a given channel */

void setAmplitude(const int channel, const int amplitude)
{
  aywrite(AMPLITUDEA_REG + channel, amplitude);
}


/* setTonePeriod --- set tone period for a given channel */

void setTonePeriod(const int channel, const unsigned int period)
{
  aywrite(TONEPERIODA_REG + (channel * 2), period & 0xff);
  aywrite(TONEPERIODA_REG + (channel * 2) + 1, period >> 8);
}


/* setNoisePeriod --- set period of the noise generator */

void setNoisePeriod(const int period)
{
  aywrite(NOISEPERIOD_REG, period);
}


/* setEnvelopePeriod --- set period of envelope generator */

void setEnvelopePeriod(const unsigned int envelope)
{
  aywrite(ENVELOPEPERIOD_REG, envelope & 0xff);
  aywrite(ENVELOPEPERIOD_REG + 1, envelope >> 8);
}


/* setEnvelopeMode --- set mode of envelope generator */

void setEnvelopeMode(const unsigned int mode)
{
  aywrite(ENVELOPEMODE_REG, mode);
}


/* aywrite --- write a byte to a given register in the PSG */

void aywrite(const int reg, const int val)
{
  ymzwrite(LOW, reg);
  ymzwrite(HIGH, val);
}


/* ymzwrite --- emulate a bus cycle to write a single byte to the chip */

void ymzwrite(const int a0, const int val)
{
  int i;

#ifdef SLOW
  digitalWrite(A0_PIN, a0);

  for (i = 0; i < 8; i++) {
    if (val & (1 << i))
      digitalWrite(D0_PIN + i, HIGH);
    else
      digitalWrite(D0_PIN + i, LOW);
  }

  // CS LOW
  digitalWrite(CS_PIN, LOW);

  // WR LOW
  digitalWrite(WR_PIN, LOW);

  // WR HIGH
  digitalWrite(WR_PIN, HIGH);

  // CS HIGH
  digitalWrite(CS_PIN, HIGH);
#else
  // A0 on Arduino Pin 5, Port D bit 5
  if (a0)
    PORTD |= (1 << 5);
  else
    PORTD &= ~(1 << 5);

  // D0-D1 on Port D bits 6 and 7; D2-D7 on Port B bits 0-5
  PORTD = (PORTD & 0x3f) | ((val & 0x03) << 6);
  PORTB = val >> 2;

  // CS on Arduino Pin 2, Port D bit 2
  PORTD &= ~(1 << 2);

  // WR on Arduino Pin 4, Port D bit 4
  PORTD &= ~(1 << 4);

  // Minimum CS LOW time is 30ns; no need for NOP here
  
  // WR on Arduino Pin 4, Port D bit 4
  PORTD |= (1 << 4);

  // CS on Arduino Pin 2, Port D bit 2
  PORTD |= (1 << 2);
#endif
}

