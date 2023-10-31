#ifndef magnometer_h
#define magnometer_h

// Determine I2C port to be used
#define I2C_PORT i2c0
// I2C address for the accelerometer
#define ACCELEROMETER_ADDRESS 0x19
// I2C address for the magnetometer
#define MAGNETOMETER_ADDRESS 0x1E
// SDA pin for I2C communication
#define SDA_PIN 0
// SCL pin for I2C communication
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
void calc_bearing(int16_t x, int16_t y);

// Calculate acceleration from accelerometer data in m/s^2
void calc_acceleration(int16_t x, int16_t y, int16_t z);

// Initialize I2C communication
void init_I2C();

// Read data from I2C register_address
uint8_t read_I2C(uint8_t device_address, uint8_t register_address);

// Write data to I2C register
void write_I2C(uint8_t device_address, uint8_t register_address, uint8_t value);

// Configure accelerometer
void config_accelerometer();

// Read accelerometer data
void read_accelerometer(int16_t *x, int16_t *y, int16_t *z);

// Configure magnetometer
void config_magnetometer();

// Read magnetometer data
void read_magnetometer(int16_t *x, int16_t *y, int16_t *z);

int mag();

#endif