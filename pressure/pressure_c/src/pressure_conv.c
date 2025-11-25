#include "pressure_conv.h"
#include <stdio.h>
#include <stdlib.h>

static double calib_a = 1.0;
static double calib_b = 0.0;

int load_calibration(const char *path) {
    if (!path) return -1;
    FILE *f = fopen(path, "r");
    if (!f) {
        // fallback defaults
        calib_a = 1.0;
        calib_b = 0.0;
        return -1;
    }
    // config format: a=<double>\nb=<double>\n
    char key[64];
    double val;
    while (fscanf(f, "%63[^=]=%lf\n", key, &val) == 2) {
        if (strcmp(key, "a") == 0) calib_a = val;
        else if (strcmp(key, "b") == 0) calib_b = val;
    }
    fclose(f);
    return 0;
}

double voltage_to_pressure_kpa(double voltage) {
    // pressure in kPa
    return calib_a * voltage + calib_b;
}

