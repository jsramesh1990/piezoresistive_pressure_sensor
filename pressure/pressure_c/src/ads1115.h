#pragma once
#include <stdint.h>

typedef struct {
    int bus_fd;
    int i2c_bus;   // e.g., 4 for /dev/i2c-4
    uint8_t addr; // ADS1115 I2C address (0x48..0x4B)
} ads1115_t;

// init with bus number and address, returns 0 on success
int ads1115_init(ads1115_t *dev, int i2c_bus, uint8_t addr);

// close device
void ads1115_close(ads1115_t *dev);

// read single conversion in raw counts (signed 16-bit)
int ads1115_read_raw(ads1115_t *dev, int16_t *raw);

// convert raw to voltage given PGA full-scale (default +/-4.096V -> LSB=125uV)
double ads1115_raw_to_voltage(int16_t raw, double pga_volts);

