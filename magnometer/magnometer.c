#include "pico/stdlib.h"  // Include the Pico standard library.
#include "hardware/i2c.h" // Include the I2C hardware library.
#include <stdio.h>        // Include the standard I/O library.
#include <math.h>         // Include the math library.
#include "magnometer.h"

#define I2C_PORT i2c0 // Define the I2C port to be used.

#define ACCELEROMETER_ADDRESS 0x19 // Define the I2C address for the accelerometer.
#define MAGNETOMETER_ADDRESS 0x1E  // Define the I2C address for the magnetometer.

// Define register addresses for the accelerometer.
#define CTRL_REG1_ACCELEROMETER 0x20
#define ACCELEROMETER_X_LSB 0x28
#define ACCELEROMETER_X_MSB 0x29
#define ACCELEROMETER_Y_LSB 0x2A
#define ACCELEROMETER_Y_MSB 0x2B
#define ACCELEROMETER_Z_LSB 0x2C
#define ACCELEROMETER_Z_MSB 0x2D

// Define register addresses for the magnetometer.
#define CRA_REG_MAGNETOMETER 0x00
#define MR_REG_MAGNETOMETER 0x02
#define MAGNETOMETER_X_MSB 0x03
#define MAGNETOMETER_X_LSB 0x04
#define MAGNETOMETER_Z_MSB 0x05
#define MAGNETOMETER_Z_LSB 0x06
#define MAGNETOMETER_Y_MSB 0x07
#define MAGNETOMETER_Y_LSB 0x08

#define SDA_PIN 0 // Define the SDA pin for I2C communication.
#define SCL_PIN 1 // Define the SCL pin for I2C communication.

vector_i m_min = {-463, -620, -621};
vector_i m_max = {516, 273, 4};
vector_f from = {1, 0, 0};

void vector_cross(vector_i *a, const vector_i *b, vector_f *out)
{
    out->x = (a->y * b->z) - (a->z * b->y);
    out->y = (a->z * b->x) - (a->x * b->z);
    out->z = (a->x * b->y) - (a->y * b->x);
}
void vector_cross2(vector_i *a, const vector_f *b, vector_f *out)
{
    out->x = (a->y * b->z) - (a->z * b->y);
    out->y = (a->z * b->x) - (a->x * b->z);
    out->z = (a->x * b->y) - (a->y * b->x);
}

float vector_dot(vector_f *a, vector_f *b)
{
    return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

void vector_normalize(vector_f *a)
{
    float mag = sqrt(vector_dot(a, a));
    a->x /= mag;
    a->y /= mag;
    a->z /= mag;
}

void calculate_acceleration(int16_t x, int16_t y, int16_t z)
{
    // Calculate acceleration from accelerometer data.
    double g = 9.81;
    double acc_x = (double)x * (g / 16384.0);
    double acc_y = (double)y * (g / 16384.0);
    double acc_z = (double)z * (g / 16384.0);

    printf("Acceleration (X, Y, Z): %.2f m/s^2, %.2f m/s^2, %.2f m/s^2\n", acc_x, acc_y, acc_z);
}

void initializeI2C()
{
    // Initialize I2C communication.
    i2c_init(I2C_PORT, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    i2c_set_slave_mode(I2C_PORT, false, 0);
}

void writeI2CRegister(uint8_t device_address, uint8_t register_address, uint8_t value)
{
    // Write data to an I2C register.
    uint8_t data[2] = {register_address, value};
    i2c_write_blocking(I2C_PORT, device_address, data, 2, true);
}

uint8_t readI2CRegister(uint8_t device_address, uint8_t register_address)
{
    // Read data from an I2C register.
    uint8_t data;
    i2c_write_blocking(I2C_PORT, device_address, &register_address, 1, true);
    i2c_read_blocking(I2C_PORT, device_address, &data, 1, false);
    return data;
}

void initalize_acc()
{
    // Configure the accelerometer.
    writeI2CRegister(ACCELEROMETER_ADDRESS, CTRL_REG1_ACCELEROMETER, 0x5F);
}

void read_acc(int16_t *x, int16_t *y, int16_t *z)
{
    // Read accelerometer data from the specified registers.
    *x = (uint16_t)((readI2CRegister(ACCELEROMETER_ADDRESS, ACCELEROMETER_X_MSB) << 8) | readI2CRegister(ACCELEROMETER_ADDRESS, ACCELEROMETER_X_LSB));
    *y = (uint16_t)((readI2CRegister(ACCELEROMETER_ADDRESS, ACCELEROMETER_Y_MSB) << 8) | readI2CRegister(ACCELEROMETER_ADDRESS, ACCELEROMETER_Y_LSB));
    *z = (uint16_t)((readI2CRegister(ACCELEROMETER_ADDRESS, ACCELEROMETER_Z_MSB) << 8) | readI2CRegister(ACCELEROMETER_ADDRESS, ACCELEROMETER_Z_LSB));
}

void initalize_mag()
{
    // Configure the magnetometer.
    writeI2CRegister(MAGNETOMETER_ADDRESS, 0x00, 0b00011100);
    writeI2CRegister(MAGNETOMETER_ADDRESS, MR_REG_MAGNETOMETER, CRA_REG_MAGNETOMETER);
}

void read_mag(int16_t *x, int16_t *y, int16_t *z)
{
    // Read magnetometer data from the specified registers.
    *x = (uint16_t)((readI2CRegister(MAGNETOMETER_ADDRESS, MAGNETOMETER_X_MSB) << 8) | readI2CRegister(MAGNETOMETER_ADDRESS, MAGNETOMETER_X_LSB));
    *y = (uint16_t)((readI2CRegister(MAGNETOMETER_ADDRESS, MAGNETOMETER_Y_MSB) << 8) | readI2CRegister(MAGNETOMETER_ADDRESS, MAGNETOMETER_Y_LSB));
    *z = (uint16_t)((readI2CRegister(MAGNETOMETER_ADDRESS, MAGNETOMETER_Z_MSB) << 8) | readI2CRegister(MAGNETOMETER_ADDRESS, MAGNETOMETER_Z_LSB));
}

float heading(void)
{
    vector_i temp_m = {};
    read_mag(&temp_m.x, &temp_m.y, &temp_m.z);
    vector_i a = {};
    read_acc(&a.x, &a.y, &a.z);

    // subtract offset (average of min and max) from magnetometer readings
    temp_m.x -= ((int32_t)m_min.x + m_max.x) / 2;
    temp_m.y -= ((int32_t)m_min.y + m_max.y) / 2;
    temp_m.z -= ((int32_t)m_min.z + m_max.z) / 2;

    // compute E and N
    vector_f E;
    vector_f N;
    vector_cross(&temp_m, &a, &E);
    vector_normalize(&E);
    vector_cross2(&a, &E, &N);
    vector_normalize(&N);

    // compute heading
    float heading = atan2(vector_dot(&E, &from), vector_dot(&N, &from)) * 180 / M_PI;
    if (heading < 0)
        heading += 360;
    return heading;
}
