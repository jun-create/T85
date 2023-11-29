#ifndef ultrasonic_h
#define ultrasonic_h
void echocallback(uint32_t events);
void setup_ultrasonic_pins(int trigPin, int echoPin);
double getcm(int trigPin, int echoPin);
#endif