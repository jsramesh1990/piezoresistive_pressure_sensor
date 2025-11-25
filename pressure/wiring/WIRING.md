# Wiring notes (sensor bridge -> ADS1115 -> Qualcomm board)

## Suggested connections (ADS1115 example)
- Sensor bridge OUT+ -> AIN0+
- Sensor bridge OUT- -> AIN1-
- Sensor excitation Vex -> 3.3V (or 5V depending on sensor spec)
- ADS1115 VCC -> 3.3V
- ADS1115 GND -> common GND
- ADS1115 SDA -> board SDA (I2C)
- ADS1115 SCL -> board SCL (I2C)
- ADDR pin -> GND (sets I2C address 0x48)
- ADD decoupling caps and input protection (series resistors, TVS diodes) if the sensor is remote.

## Tips
- Use twisted pair and shielding for bridge wires.
- Use a stable excitation voltage; consider LDO for sensor Vex.
- Validate ADC readings before mounting mechanically.
