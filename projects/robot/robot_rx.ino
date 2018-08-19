-e //Keszitette: Fenyes Balazs (fenyesb.github.io)

//robot rx
/*
  vcc                gnd
  ...  pb0      0    adc0/pa0 mld 10
  ...  pb1      1    adc1/pa1 mrd 9
  rst  pb3/rst  .    adc2/pa2 sr  8
  mls  pb2~/oc0a      2    adc3/pa3 sl  7
  mrs  adc7/pa7~/oc0b 3    adc4/pa4 rx  6 pcint4
  led  adc6/pa6~ 4    adc5/pa5 ...~ 5
*/
#define LED 4
#define ML_SPEED 2
#define MR_SPEED 3
#define ML_DIR 10
#define MR_DIR 9
#define SENSOR_RIGHT 2
#define SENSOR_LEFT 3
#define RX 6
#define PWM analogWrite2
//|^^^^|________| 0
//|^^^^^^^^|____| 1
static volatile uint32_t rising = 0;
#define BUFLEN 4
static volatile uint8_t buf[BUFLEN] = {0};
static volatile uint8_t buf_valid[BUFLEN] = {0};
static volatile bool bvalid = false;
static volatile uint8_t byteptr = 0, bitptr = 0;

#define UNIT_TIME 1250
#define TIME(n) ((n)*UNIT_TIME+UNIT_TIME/2) //1250, 2500, 3750, 5000
#include "wiring_private.h"
#include "pins_arduino.h"

void analogWrite2(uint8_t pin, int val)
{
  pinMode(pin, OUTPUT);

  if (val <= 0)
  {
    digitalWrite(pin, LOW);
  }
  else if (val >= 255)
  {
    digitalWrite(pin, HIGH);
  }
  else
  {
    uint8_t timer = digitalPinToTimer(pin);
    if ( timer == TIMER0A) {
      // connect pwm to pin on timer 0, channel A
      sbi(TCCR0A, COM0A1);
      cbi(TCCR0A, COM0A0);
      OCR0A = val; // set pwm duty
    } else if ( timer == TIMER0B) {
      // connect pwm to pin on timer 0, channel B
      sbi(TCCR0A, COM0B1);
      cbi(TCCR0A, COM0B0);
      OCR0B = val; // set pwm duty
    }
  }
}
uint8_t flipbit = 0;
ISR(PCINT0_vect)
{
  uint8_t pin = PINA & _BV(PA4);
  uint32_t us = micros();
  if (pin)
  {
    rising = us;
  } else {
    uint32_t high = us - rising;
    if (high < TIME(1))
    {
      buf[byteptr] <<= 1;
      buf[byteptr] &= ~1;
    } else if (high < TIME(2))
    {
      buf[byteptr] <<= 1;
      buf[byteptr] |= 1;
    } else if (high < TIME(3))
    {
      byteptr = BUFLEN - 1;
      bitptr = 7;
    }
    bitptr = (bitptr + 1) % 8;
    if (bitptr == 0)
      byteptr++;
    if (byteptr >= BUFLEN)
    {
      byteptr = 0;
      for (uint8_t i = 0; i < BUFLEN; i++)
        buf_valid[i] = buf[i];
      bvalid = true;
    }
  }
}
int middle;
struct
{
  uint8_t ax, ay, sw; //sw == 1 => follow line
} tx;
struct
{
  int left, right;
} motors;
bool serial_ok = false;
void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(RX, INPUT);
  pinMode(ML_DIR, OUTPUT);
  pinMode(MR_DIR, OUTPUT);
  pinMode(ML_SPEED, OUTPUT);
  pinMode(MR_SPEED, OUTPUT);
  pinMode(SENSOR_LEFT, INPUT);
  pinMode(SENSOR_RIGHT, INPUT);
  GIMSK = _BV(PCIE0); //only enable pcint on pa0..7
  PCMSK0 = _BV(PCINT4); //rx
  tx.ax = 127;
  tx.ay = 127;
  tx.sw = 1;
  setmotor(0, 0);
  motors.left = 0;
  motors.right = 0;
  delay(500);
  middle = (analogRead(SENSOR_LEFT) - analogRead(SENSOR_RIGHT)) / 4;
  for (uint8_t i = 0; i < 6; i++)
  {
    digitalWrite(LED, i & 1);
    delay(50);
  }
}

#define MAXPOWER 256
signed char mmap(uint8_t x) //0..255 => -MAX..MAX
{
  return (uint16_t)x * MAXPOWER / 256 - (MAXPOWER / 2);
}
#define DELTA 10
void setmotor(int ml, int mr)
{
  if (ml > motors.left)
  {
    motors.left += DELTA;
  }
  else
  {
    motors.left -= DELTA;
  }
  if (mr > motors.right)
  {
    motors.right += DELTA;
  }
  else
  {
    motors.right -= DELTA;
  }
  digitalWrite(ML_DIR, motors.left >= 0);
  digitalWrite(MR_DIR, motors.right >= 0);
  PWM(ML_SPEED, abs(motors.left));
  PWM(MR_SPEED, abs(motors.right));
}
void loop() {
  if (tx.sw == 0)
  { //manual
    signed char ml = mmap(tx.ax) + mmap(tx.ay); //-100..100
    signed char mr = mmap(tx.ax) - mmap(tx.ay); //-100..100
    uint8_t total_power = max(abs(mmap(tx.ax)), abs(mmap(tx.ay))); //0..100
    uint8_t totalm = abs(ml) + abs(mr); //0..200
    ml = 2 * total_power * ml / totalm; //0..100
    mr = 2 * total_power * mr  / totalm; //0..100
    setmotor(ml, mr);
  }
  else
  { //follow line
    int newmiddle = (analogRead(SENSOR_LEFT) - analogRead(SENSOR_RIGHT)) / 4;
    if (newmiddle > middle)
    {
      setmotor(80, 0);
    }
    else
    {
      setmotor(0, 80);
    }
  }
  delay(20);
  bool retry = true;
  if (bvalid)
  {
    bvalid = false;
    uint8_t ax = buf_valid[0];
    uint8_t ay = buf_valid[1];
    uint8_t sw = buf_valid[2];
    uint8_t chk1 = buf_valid[3];
    uint8_t chk2 = ax ^ ay ^ 0xAA ^ sw;
    if (chk1 == chk2 && sw <= 1)
    {
	  flipbit ^= 1;
	  digitalWrite(LED, flipbit);
	  tx.ax = ax;
	  tx.ay = ay;
	  if (tx.sw != sw)
	  {
	    setup();
	  }
	  tx.sw = sw;
    }
  }
}
