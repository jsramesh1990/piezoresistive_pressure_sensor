#pragma once

// Convert sensor voltage (V) to pressure (kPa).
// Implementation uses linear calibration: pressure = a * voltage + b
// a and b are loaded from config/calibration.cfg

double voltage_to_pressure_kpa(double voltage);
int load_calibration(const char *path); // return 0 on success

