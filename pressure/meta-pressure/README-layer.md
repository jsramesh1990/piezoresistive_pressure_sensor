meta-pressure Yocto layer — skeleton
- Place this layer in your `bblayers.conf`.
- It contains:
  - linux kernel bbappend to add kernel fragment `kernel/ads1115.cfg`
  - device-tree recipe to install `dts/myboard-ads1115.dts`
  - a simple recipe skeleton for `pressure-daemon`
