#define _GNU_SOURCE
#include "ads1115.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// ADS1115 registers
enum {
    ADS_CONVERSION = 0x00,
    ADS_CONFIG     = 0x01,
    ADS_LO_THRESH  = 0x02,
    ADS_HI_THRESH  = 0x03
};

static int write_register(int fd, uint8_t reg, uint16_t value) {
    uint8_t buf[3];
    buf[0] = reg;
    buf[1] = (value >> 8) & 0xff;
    buf[2] = value & 0xff;
    if (write(fd, buf, 3) != 3) return -1;
    return 0;
}

static int16_t read_register_16(int fd, uint8_t reg) {
    uint8_t buf[2];
    if (write(fd, &reg, 1) != 1) return 0;
    if (read(fd, buf, 2) != 2) return 0;
    return (int16_t)((buf[0] << 8) | buf[1]);
}

int ads1115_init(ads1115_t *dev, int i2c_bus, uint8_t addr) {
    if (!dev) return -1;
    dev->i2c_bus = i2c_bus;
    dev->addr = addr;
    char path[64];
    snprintf(path, sizeof(path), "/dev/i2c-%d", i2c_bus);
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("open i2c device");
        return -1;
    }
    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        perror("ioctl I2C_SLAVE");
        close(fd);
        return -1;
    }
    dev->bus_fd = fd;

    // Configure ADS1115: continuous conversion, AIN0 single-ended, PGA +-4.096V, 128SPS
    // Config bits from datasheet (16-bit register)
    // OS=1 (no effect in continuous), MUX=100 (AIN0 GND), PGA=001 (±4.096V), MODE=0 (continuous)
    // DR=100 (128SPS), COMP_MODE, COMP_POL etc disabled.
    uint16_t config = 0;
    config |= (0 << 15);              // OS bit (no)
    config |= (0x4 << 12);            // MUX[14:12] = 100 -> AIN0
    config |= (0x1 << 9);             // PGA = 001 => ±4.096V
    config |= (0 << 8);               // MODE = 0 => continuous
    config |= (0x4 << 5);             // DR = 100 => 128SPS
    config |= (0 << 4);               // COMP_MODE
    config |= (0 << 3);               // COMP_POL
    config |= (0 << 2);               // COMP_LAT
    config |= (3 << 0);               // COMP_QUE = 11 (disable comparator)

    if (write_register(fd, ADS_CONFIG, config) < 0) {
        perror("write config");
        close(fd);
        return -1;
    }
    // short delay
    usleep(1000 * 10);
    return 0;
}

void ads1115_close(ads1115_t *dev) {
    if (!dev) return;
    if (dev->bus_fd > 0) close(dev->bus_fd);
    dev->bus_fd = -1;
}

int ads1115_read_raw(ads1115_t *dev, int16_t *raw) {
    if (!dev || !raw) return -1;
    // conversion register is 16-bit signed
    int16_t val = read_register_16(dev->bus_fd, ADS_CONVERSION);
    *raw = val;
    return 0;
}

// pga_volts is full-scale positive (e.g., 4.096)
double ads1115_raw_to_voltage(int16_t raw, double pga_volts) {
    // ADS1115 is 16-bit but effectively 15-bit signed (range -32768..32767)
    double lsb = (pga_volts * 2.0) / 65536.0; // volts per LSB
    return raw * lsb;
}

