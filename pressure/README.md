# Qualcomm Pressure Sensing Project (Yocto BSP)

**What this repo contains**
- A complete scaffold to integrate a piezoresistive pressure sensor (analog bridge) into a Qualcomm board running a Yocto BSP.
- Yocto layer skeleton (`meta-pressure/`) with kernel config fragment and device-tree overlay.
- A simple Python `pressure_daemon` that reads IIO sysfs, applies calibration and triggers actions.
- Systemd unit for running the daemon on boot.
- Device-tree snippet for an ADS1115 I2C ADC.
- Wiring and calibration notes.
- The original sensor PDF (copied to `docs/`) that explains piezoresistive sensors.

## Quick start (local test)
1. Inspect files in this repo and adapt the device-tree to your board's I2C adapter.
2. Add `meta-pressure` to your Yocto `bblayers.conf`.
3. Add the kernel fragment and dtb to your image via recipes shown in `meta-pressure`.
4. Build your Yocto image and flash your Qualcomm board.
5. On the device, enable I2C, confirm the ADC with `i2cdetect -r` and check `/sys/bus/iio/devices/`.
6. Install the `pressure-daemon` (the recipe or copy `software/pressure_daemon.py`) and enable `pressure.service`.

## Deliverables
- `meta-pressure/` Yocto layer scaffold
- `software/pressure_daemon.py`
- `dts/myboard-ads1115.dts`
- `kernel/ads1115.cfg`
- `wiring/WIRING.md`
- Original sensor PDF: `docs/Fachbeitrag_Piezo_e.pdf`

## Licence
MIT — see LICENSE file.

## Contact / Next steps
If you give me your GitHub repo URL or permission, I can:
- Create the repository and push these files (you will need to provide a personal access token or grant access).
- Or provide the repo zip for you to push locally with `git init` and `git push`.
