-e //Keszitette: Fenyes Balazs (fenyesb.github.io)

#define LED 0
#define TX 1
#define SW 2
#define AX_PIN 3
#define AX_ADC 3
#define AY_PIN 4
#define AY_ADC 2

/*
~~~^^|__|^^|__|^^|__ PREAMBLE
      1ms/0   1ms/0
    3ms     1ms     2ms            4ms
|^^^^^^^^|__|^^|__|^^^^|__ ... |^^^^^^^^|__
   START      0      1             END
*/

enum MSG { START = 0, BIT0 = 1, BIT1 = 2, END = 3, PREAMBLE = 1 };
void transmit(enum MSG msg)
{
  static uint8_t high[] = {3, 1, 2, 4};
  static uint8_t low[] = {3, 2, 1, 4};
  digitalWrite(TX, HIGH);
  delay(high[msg]);
  digitalWrite(TX, LOW);
  delay(low[msg]);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  pinMode(TX, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(SW, INPUT);
  digitalWrite(SW, HIGH);
  pinMode(AX_PIN, INPUT);
  pinMode(AY_PIN, INPUT);
  PCMSK |= _BV(SW);
  GIMSK |= _BV(PCIE);
}

volatile uint8_t sw_pressed = 0;
uint8_t sw_state = 0;
ISR(PCINT0_vect) {
  if (PINB & _BV(SW))
    sw_pressed = 1;
}

#define BUFLEN 4

void loop() {
  if(sw_pressed){
    for(uint8_t i = 0; i < 6; i++)
    {
      digitalWrite(LED, i&1);
      delay(60);
    }
    sw_state ^= 1;
    sw_pressed = 0;
  }
  digitalWrite(LED, LOW);
  static uint8_t buf[BUFLEN];
  uint8_t ax = analogRead(AX_ADC) >> 2;
  uint8_t ay = analogRead(AY_ADC) >> 2;
  buf[0] = ax;
  buf[1] = ay;
  buf[2] = sw_state;
  buf[3] = ax ^ ay ^ 0xAA ^ sw_state;
  digitalWrite(LED, HIGH);
  for(uint8_t i = 0; i < 4; i++)
    transmit(PREAMBLE);
  transmit(START);
  for (uint8_t byteptr = 0; byteptr < BUFLEN; byteptr++)
  {
    for (uint8_t bitptr = 0; bitptr < 8; bitptr++)
    {
      uint8_t bitval = buf[byteptr] & _BV(7 - bitptr);
      transmit(bitval?BIT1:BIT0);
    }
  }
  transmit(END);
  delay(100);
}
