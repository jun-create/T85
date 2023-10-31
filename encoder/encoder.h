#ifndef ENCODER_H
#define ENCODER_H
// Distance between two holes on the encoder 21cm / 20 holes
static const float WHEEL_HOLE_DIST = 21 / 20;
/*
 * Get time in milliseconds
 * for measuring the pulse width of the barcode
 */
long long getTimeMS(void);

// callback function for encoder
void gpio_callback(uint gpio, uint32_t events);
#endif