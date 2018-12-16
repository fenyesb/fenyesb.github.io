//Keszitette: Fenyes Balazs (fenyesb.github.io)

#define X PORTA
#define Y PORTC
float wallx[6];
float wally[6];
void setup() {
  DDRA = 0xFF;
  DDRC = 0xFF;
  X = 0;
  Y = 0;
  for (int i = 0; i < 6; i++) {
    wallx[i] = i * 50;
    wally[i] = random(60) + 60;
  }
  Serial.begin(115200);
}
float y = 127, v = 0.5f;
float cnt = 0;
void loop() {
  X = 50;
  Y = y;
  v = constrain(v - 0.005f, -1, 1);
  if (Serial.available())
  {
    Serial.read();
    v = 0.6f;
  }
  y += v;
  if (y < 0 || y > 255)
  {
    y = 127;
    v = 0.5f;
    for (int i = 0; i < 6; i++) {
      wallx[i] = i * 50;
      wally[i] = random(60) + 60;
    }
  }
  delay(1);
  for (int i = 0; i < 6; i++)
  {
    wallx[i] -= 0.1f;
    if (wallx[i] > 255)
      continue;
    if (wallx[i] < 20)
    {
      for (int i = 0; i < 5; i++)
      {
        wallx[i] = wallx[i + 1];
        wally[i] = wally[i + 1];
      }
      wallx[5] = wallx[4] + 50;
      wally[5] = random(60) + 60;
    }
    X = wallx[i];
    if (abs(wallx[i] - 50) < 1 && (y < wally[i] || y > wally[i] + 60))
    {
      int c = 100;
      while (c-- > 0)
      {
        for (Y = wally[i]; Y > 0; Y--);
        for (Y = wally[i] + 60; Y < 255; Y++);
      }
    }
    static unsigned int eff = 0;
    unsigned int diag = eff/64;
    int X_ = X-16;
    for (Y = wally[i]; Y > 0; Y--) X=diag++%16+X_;
    for (Y = wally[i]; Y > 0; Y--) X=diag--%16+X_;
    diag = eff++/64;
    for (Y = wally[i] + 60; Y < 255; Y++) X=diag++%16+X_;
    for (Y = wally[i] + 60; Y < 255; Y++) X=diag--%16+X_;
  }
}
