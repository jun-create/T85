/*
 * The magnometer has 6-axis sense with a temperature sensor on board
 * Valued for its low power, simplicity, and accuracy
 * However, the current data read from the magnometer is not calibrated or refined
 */
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>
#include <math.h>

// Determine I2C port to be used
#define I2C_PORT i2c0
// I2C address for the accelerometer
#define ACCELEROMETER_ADDRESS 0x19
// I2C address for the magnetometer
#define MAGNETOMETER_ADDRESS 0x1E
// SDA pin (serial data) for I2C communication
#define SDA_PIN 0
// SCL pin (serial clock) for I2C communication
#define SCL_PIN 1

// accelerometer register addresses
#define CTRL_REG1_ACCELEROMETER 0x20
#define ACCELEROMETER_X_LSB 0x28
#define ACCELEROMETER_X_MSB 0x29
#define ACCELEROMETER_Y_LSB 0x2A
#define ACCELEROMETER_Y_MSB 0x2B
#define ACCELEROMETER_Z_LSB 0x2C
#define ACCELEROMETER_Z_MSB 0x2D

// magnetometer register addresses
#define CRA_REG_MAGNETOMETER 0x00
#define MR_REG_MAGNETOMETER 0x02
#define MAGNETOMETER_X_MSB 0x03
#define MAGNETOMETER_X_LSB 0x04
#define MAGNETOMETER_Z_MSB 0x05
#define MAGNETOMETER_Z_LSB 0x06
#define MAGNETOMETER_Y_MSB 0x07
#define MAGNETOMETER_Y_LSB 0x08

// Calculate the compass bearing from magnetometer data.
void calc_bearing(int16_t x, int16_t y)
{
    double heading = atan2((double)y, (double)x);
    double heading_degrees = heading * (180.0 / M_PI);

    if (heading_degrees < 0.0)
    {
        heading_degrees += 360.0;
    }

    printf("[Bearing] %.2f degrees\n", heading_degrees);
    // if (heading_degrees > 337.5 || heading_degrees < 22.5)
    // {
    //     printf("[Compass] North\n");
    // }
    // else if (heading_degrees > 22.5 && heading_degrees < 67.5)
    // {
    //     printf("[Compass] North East\n");
    // }
    // else if (heading_degrees > 67.5 && heading_degrees < 112.5)
    // {
    //     printf("[Compass] East\n");
    // }
    // else if (heading_degrees > 112.5 && heading_degrees < 157.5)
    // {
    //     printf("[Compass] South East\n");
    // }
    // else if (heading_degrees > 157.5 && heading_degrees < 202.5)
    // {
    //     printf("[Compass] South\n");
    // }
    // else if (heading_degrees > 202.5 && heading_degrees < 247.5)
    // {
    //     printf("[Compass] South West\n");
    // }
    // else if (heading_degrees > 247.5 && heading_degrees < 292.5)
    // {
    //     printf("[Compass] West\n");
    // }
    // else if (heading_degrees > 292.5 && heading_degrees < 337.5)
    // {
    //     printf("[Compass] North West\n");
    // }
    // else
    // {
    //     printf("[Compass] Error\n");
    // }
}

// Calculate acceleration from accelerometer data in m/s^2
void calc_acceleration(int16_t x, int16_t y, int16_t z)
{
    double g = 9.81;
    double acc_x = (double)x * (g / 16384.0);
    double acc_y = (double)y * (g / 16384.0);
    double acc_z = (double)z * (g / 16384.0);

    printf("[Accel]  X:%.2f m/s^2, Y:%.2f m/s^2, Z:%.2f m/s^2\n", acc_x, acc_y, acc_z);
}

// Initialize I2C communication
void init_I2C()
{
    i2c_init(I2C_PORT, 100000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    i2c_set_slave_mode(I2C_PORT, false, 0);
}

// Read data from I2C register_address
uint8_t read_I2C(uint8_t device_address, uint8_t register_address)
{
    uint8_t data;
    i2c_write_blocking(I2C_PORT, device_address, &register_address, 1, true);
    i2c_read_blocking(I2C_PORT, device_address, &data, 1, false);
    return data;
}

// Write data to I2C register
void write_I2C(uint8_t device_address, uint8_t register_address, uint8_t value)
{
    uint8_t data[2] = {register_address, value};
    i2c_write_blocking(I2C_PORT, device_address, data, 2, true);
}

// Configure accelerometer
void config_accelerometer()
{
    write_I2C(ACCELEROMETER_ADDRESS, CTRL_REG1_ACCELEROMETER, 0x5F);
}

// Read accelerometer data
void read_accelerometer(int16_t *x, int16_t *y, int16_t *z)
{
    *x = (uint16_t)((read_I2C(ACCELEROMETER_ADDRESS, ACCELEROMETER_X_MSB) << 8) | read_I2C(ACCELEROMETER_ADDRESS, ACCELEROMETER_X_LSB));
    *y = (uint16_t)((read_I2C(ACCELEROMETER_ADDRESS, ACCELEROMETER_Y_MSB) << 8) | read_I2C(ACCELEROMETER_ADDRESS, ACCELEROMETER_Y_LSB));
    *z = (uint16_t)((read_I2C(ACCELEROMETER_ADDRESS, ACCELEROMETER_Z_MSB) << 8) | read_I2C(ACCELEROMETER_ADDRESS, ACCELEROMETER_Z_LSB));
}

// Configure magnetometer
void config_magnetometer()
{
    write_I2C(MAGNETOMETER_ADDRESS, MR_REG_MAGNETOMETER, CRA_REG_MAGNETOMETER);
}

// Read magnetometer data
void read_magnetometer(int16_t *x, int16_t *y, int16_t *z)
{
    *x = (uint16_t)((read_I2C(MAGNETOMETER_ADDRESS, MAGNETOMETER_X_MSB) << 8) | read_I2C(MAGNETOMETER_ADDRESS, MAGNETOMETER_X_LSB));
    *y = (uint16_t)((read_I2C(MAGNETOMETER_ADDRESS, MAGNETOMETER_Y_MSB) << 8) | read_I2C(MAGNETOMETER_ADDRESS, MAGNETOMETER_Y_LSB));
    *z = (uint16_t)((read_I2C(MAGNETOMETER_ADDRESS, MAGNETOMETER_Z_MSB) << 8) | read_I2C(MAGNETOMETER_ADDRESS, MAGNETOMETER_Z_LSB));
}

int mag()
{
    stdio_init_all();
    init_I2C();
    config_accelerometer();
    config_magnetometer();

    // main while loop to
    while (1)
    {
        int16_t x_acc, y_acc, z_acc;
        int16_t x_mag, y_mag, z_mag;
        read_accelerometer(&x_acc, &y_acc, &z_acc);
        read_magnetometer(&x_mag, &y_mag, &z_mag);

        calc_bearing(x_mag, y_mag);
        calc_acceleration(x_acc, y_acc, z_acc);
        printf("[Accel] (X = %d, Y = %d, Z = %d)\n", x_acc, y_acc, z_acc);
        printf("[Magnet] (X = %d, Y = %d, Z = %d)\n", x_mag, y_mag, z_mag);

        sleep_ms(1000);
    }
}