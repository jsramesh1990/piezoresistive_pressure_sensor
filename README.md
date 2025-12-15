
# **Qualcomm Pressure Sensing System — Yocto BSP Integration**

# Qualcomm Pressure Sensing System — Yocto BSP Integration

![Yocto](https://img.shields.io/badge/Yocto-Project-blueviolet)
![Linux](https://img.shields.io/badge/Linux-Kernel-yellow)
![Qualcomm](https://img.shields.io/badge/SoC-Qualcomm%20Snapdragon-red)
![IIO](https://img.shields.io/badge/Subsystem-IIO-orange)
![ADC](https://img.shields.io/badge/ADC-ADS1115-blue)
![Sensor](https://img.shields.io/badge/Sensor-Piezoresistive-green)
![Python](https://img.shields.io/badge/Python-Daemon-3776AB)
![systemd](https://img.shields.io/badge/init-systemd-lightgrey)
![BSP](https://img.shields.io/badge/BSP-Yocto%20Meta--Layer-success)
![License](https://img.shields.io/badge/License-MIT-lightgrey)


**Piezoresistive Pressure Sensor + ADS1115 ADC + Qualcomm SoC + Yocto BSP**


# Qualcomm Pressure Sensing System — Yocto BSP Integration

![Yocto](https://img.shields.io/badge/Yocto-Project-blueviolet)
![Linux](https://img.shields.io/badge/Linux-Kernel-yellow)
![Qualcomm](https://img.shields.io/badge/SoC-Qualcomm%20Snapdragon-red)
![IIO](https://img.shields.io/badge/Subsystem-IIO-orange)
![ADC](https://img.shields.io/badge/ADC-ADS1115-blue)
![Sensor](https://img.shields.io/badge/Sensor-Piezoresistive-green)
![Python](https://img.shields.io/badge/Python-Daemon-3776AB)
![systemd](https://img.shields.io/badge/init-systemd-lightgrey)
![BSP](https://img.shields.io/badge/BSP-Yocto%20Meta--Layer-success)
![License](https://img.shields.io/badge/License-MIT-lightgrey)


This repository provides a complete implementation for integrating a **piezoresistive pressure sensor** with a **Qualcomm-based development board** using **Yocto Project Build System**.

It includes a custom Yocto meta-layer, device-tree overlay, kernel fragment, systemd service, and a Python daemon for real-time pressure monitoring.

---

#  **Project Overview**

This project reads **analog pressure** from a *piezoresistive pressure sensor* (PDF included in `docs/`) using an **ADS1115 ADC** and exposes the data via **IIO subsystem** on Qualcomm hardware.

### ✔ Works on:

* Qualcomm Snapdragon boards (C8750, APQ8096, APQ8098, RB3/RB5, etc.)
* Any Yocto-based BSP with available I²C interface

### ✔ Provides:

* Yocto Layer: `meta-pressure/`
* Kernel Config Fragment
* Device Tree Overlay for ADS1115
* Python Pressure Daemon
* Systemd service
* Wiring instructions
* Original sensor technical PDF

---

#  **Repository Structure**

```
pressure-qualcomm-project/
│
├── README.md              → Project overview & instructions
├── LICENSE                → MIT License
│
├── meta-pressure/         → Yocto layer (recipes, DT, daemon)
│   ├── recipes-kernel/
│   ├── recipes-bsp/
│   └── recipes-support/
│
├── dts/
│   └── myboard-ads1115.dts → Device tree overlay for Qualcomm board
│
├── kernel/
│   └── ads1115.cfg        → Kernel fragment enabling IIO + ADS1115
│
├── software/
│   ├── pressure_daemon.py → Python daemon (reads IIO sysfs)
│   └── pressure.service   → systemd unit
│
├── wiring/
│   └── WIRING.md          → ADS1115 + Sensor + Qualcomm wiring
│
└── docs/
    └── Fachbeitrag_Piezo_e.pdf → Technical PDF about the sensor
```

---

#  **Hardware Requirements**

### ✔ Piezoresistive pressure sensor

(Analog bridge output — described in included PDF)

### ✔ ADC (Recommended)

* Texas Instruments **ADS1115** (I2C, 16-bit, differential)

### ✔ Qualcomm Development Board

Examples:

* TurboX C8750 (Snapdragon)
* RB3 / RB5
* APQ8096 / APQ8098

### ✔ Power & Wiring

* 3.3V supply for ADS1115
* Common GND
* SDA + SCL connected to Qualcomm board I²C

---

#  **Wiring Summary**

See full wiring in `wiring/WIRING.md`

### **Basic Connection**

| Sensor | ADS1115 | Qualcomm Board |
| ------ | ------- | -------------- |
| OUT+   | AIN0    | —              |
| OUT−   | AIN1    | —              |
| VEX    | VCC     | 3.3V           |
| GND    | GND     | GND            |
| SDA    | SDA     | I2C SDA        |
| SCL    | SCL     | I2C SCL        |
| ADDR   | GND     | —              |

---

#  **Yocto BSP Integration**

## 1️⃣ Add Layer to Yocto

Edit `bblayers.conf`:

```
BBLAYERS += " \
    /path/to/meta-pressure \
"
```

---

## 2️⃣ Add Kernel Fragment

The file `kernel/ads1115.cfg` enables:

* IIO subsystem
* I2C
* ADS1015/ADS1115 driver support

Included automatically through:
`meta-pressure/recipes-kernel/linux/linux-yocto-ads1115.bbappend`

---

## 3️⃣ Add Device Tree Overlay

Located at:

```
dts/myboard-ads1115.dts
```

Edit the I²C node to match your Qualcomm DTS:

Example:

```
/soc/i2c@78b6000 {
    status = "okay";

    ads1115@48 {
        compatible = "ti,ads1115";
        reg = <0x48>;
        #io-channel-cells = <1>;
    };
};
```

The Yocto recipe compiles it into a `.dtbo`.

---

#  **Testing on Board**

After flashing the Yocto build:

### 1. Check ADC on I2C bus:

```
i2cdetect -y 0
```

→ Should show `0x48`

### 2. Check IIO device:

```
ls /sys/bus/iio/devices/
```

### 3. View raw reading:

```
cat /sys/bus/iio/devices/iio:device0/in_voltage0_raw
```

---

#  **Pressure Daemon**

Located at:

```
software/pressure_daemon.py
```

### Features:

* Reads raw ADC data from IIO
* Converts to voltage
* Applies calibration (a, b values via JSON)
* Logs pressure readings
* Triggers warnings when above threshold

Run manually:

```
python3 /usr/bin/pressure_daemon.py
```

---

#  **Systemd Integration**

Enable service:

```
systemctl enable pressure.service
systemctl start pressure.service
```

Check status:

```
systemctl status pressure.service
```

Logs:

```
journalctl -u pressure.service -f
```

---

#  **How to Build Full Yocto Image**

```
bitbake core-image-minimal
```

Or your custom image:

```
bitbake my-image
```

Flash to board → daemon starts automatically → pressure logged.

---

#  **Documentation**

The original technical PDF explaining piezoresistive sensors is included:

```
docs/Fachbeitrag_Piezo_e.pdf
```

---

#  **License**

MIT License — free for commercial and personal use.

---

#  **Support & Customization**

If you want, I can generate:

✔ A full **flowchart**
✔ System **block diagram**
✔ Professional **architecture diagrams**
✔ Advanced **DTS for your exact Qualcomm board**
✔ **MQTT / Cloud upload support**
✔ A GUI for real-time graphing

