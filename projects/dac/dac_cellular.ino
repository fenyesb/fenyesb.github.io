//Keszitette: Fenyes Balazs (fenyesb.github.io)

#define X PORTA
#define Y PORTC
void setup() {
  DDRA = 0xFF;
  DDRC = 0xFF;
  // Serial.begin(115200);
}
float cnt = 0;
void loop() {
  // unsigned long tstart = millis();
  byte a[256] = {0};
  byte b[256] = {0};
  a[(int)(127+sin(cnt)*126)] = 1;
  a[(int)(127+sin(cnt+PI/2)*126)] = 1;
  cnt+=0.05f;
  static const unsigned char sums[4] = {0, 1, 1, 0};
  for (Y = 0; Y < 188; Y+=2)
  {
    byte rsum = a[0]+a[1]+a[2];
    for (byte x = 2; x < 255; x++)
    {
      rsum += a[x+1];
      rsum -= a[x-2];
      b[x] = sums[rsum];
      if(b[x])
        X=x;
    }
    Y+=2;
    rsum = b[0]+b[1]+b[2];
    for (byte x = 2; x < 255; x++)
    {
      rsum += b[x+1];
      rsum -= b[x-2];
      a[x] = sums[rsum];
      if(a[x])
        X=x;
    }
  }
  // unsigned dt = millis() - tstart;
  // Serial.println(dt);
}

