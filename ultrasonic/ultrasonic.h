#ifndef ultrasonic_h
#define ultrasonic_h
void echo_callback(uint gpio, uint32_t events);
void setupUltrasonicPins(int trigPin, int echoPin);
int getCm(int trigPin, int echoPin);
int getInch(int trigPin, int echoPin);
int detect_near_object(int trigPin, int echoPin);
#endif