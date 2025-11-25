#!/usr/bin/env python3
"""Simple pressure daemon that reads IIO sysfs, applies calibration, and logs/publishes events."""
import time, os, json, logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s %(levelname)s: %(message)s')

# Configuration: adapt device index, channels and calibration constants
IIO_DEVICE = "iio:device0"
RAW_PATH = f"/sys/bus/iio/devices/{IIO_DEVICE}/in_voltage0_raw"
SCALE_PATH = f"/sys/bus/iio/devices/{IIO_DEVICE}/in_voltage0_scale"
CALIB_FILE = "/etc/pressure_calib.json"
SAMPLE_INTERVAL = 1.0  # seconds
PRESSURE_THRESHOLD = 1000.0  # user units (example)

def read_int(path):
    try:
        with open(path,'r') as f:
            return int(f.read().strip())
    except Exception as e:
        logging.error("read_int failed for %s: %s", path, e)
        return None

def read_float(path):
    try:
        with open(path,'r') as f:
            return float(f.read().strip())
    except Exception as e:
        logging.error("read_float failed for %s: %s", path, e)
        return None

def load_calib():
    if os.path.exists(CALIB_FILE):
        try:
            with open(CALIB_FILE,'r') as f:
                return json.load(f)
        except:
            pass
    # default linear calibration: pressure = a * voltage + b
    return {"a":1.0, "b":0.0}

def voltage_from_raw(raw, scale):
    return raw * scale

def pressure_from_voltage(voltage, calib):
    return calib.get('a',1.0)*voltage + calib.get('b',0.0)

def main():
    calib = load_calib()
    logging.info("Starting pressure daemon, using calibration: %s", calib)
    while True:
        raw = read_int(RAW_PATH)
        scale = read_float(SCALE_PATH)
        if raw is None or scale is None:
            time.sleep(1.0)
            continue
        voltage = voltage_from_raw(raw, scale)
        pressure = pressure_from_voltage(voltage, calib)
        logging.info("Pressure: %.3f (voltage=%.6f raw=%d)", pressure, voltage, raw)
        if pressure > PRESSURE_THRESHOLD:
            logging.warning("Pressure threshold exceeded: %.3f", pressure)
            # TODO: publish via MQTT or trigger GPIO, etc.
        time.sleep(SAMPLE_INTERVAL)

if __name__ == '__main__':
    main()
