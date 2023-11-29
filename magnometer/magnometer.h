#ifndef magnometer_h
#define magnometer_h
#include "pico/stdlib.h"

void initializeI2C();
void initalize_acc();
void initalize_mag();

void read_mag(int16_t* x, int16_t* y, int16_t* z);
float heading(void);
typedef struct vector_f_{
    float x, y, z;
} vector_f;
typedef struct vector_i_{
    int16_t x, y, z;
} vector_i;
extern vector_i m_min;
extern vector_i m_max;

#endif