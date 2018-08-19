-e //Keszitette: Fenyes Balazs (fenyesb.github.io)

#define X PORTA
#define Y PORTC
void setup() {
	DDRA = DDRC = ~0;
}
void loop() {
	X++;
	Y++;
	delayMicroseconds(100);
}
