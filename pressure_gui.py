#!/usr/bin/env python3
import tkinter as tk
from tkinter import messagebox
from tkinter import ttk
import threading
import time
import os
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

IIO_DEVICE = "iio:device0"
RAW_PATH = f"/sys/bus/iio/devices/{IIO_DEVICE}/in_voltage0_raw"
SCALE_PATH = f"/sys/bus/iio/devices/{IIO_DEVICE}/in_voltage0_scale"

running = False
pressure_data = []

# Calibration defaults
calib_a = 1.0
calib_b = 0.0

def read_int(path):
    try:
        with open(path, 'r') as f:
            return int(f.read().strip())
    except:
        return None

def read_float(path):
    try:
        with open(path, 'r') as f:
            return float(f.read().strip())
    except:
        return None

def calculate_pressure(raw, scale):
    voltage = raw * scale
    return calib_a * voltage + calib_b

def start_reading():
    global running
    running = True
    threading.Thread(target=update_readings, daemon=True).start()

def stop_reading():
    global running
    running = False

def update_readings():
    while running:
        raw = read_int(RAW_PATH)
        scale = read_float(SCALE_PATH)

        if raw is not None and scale is not None:
            pressure = calculate_pressure(raw, scale)

            # Update label
            pressure_label.config(text=f"Current Pressure: {pressure:.3f}")

            # Add data to graph
            pressure_data.append(pressure)
            if len(pressure_data) > 200:
                pressure_data.pop(0)

            update_plot()

            # Threshold example
            if pressure > float(threshold_entry.get()):
                messagebox.showwarning("Pressure Alert", f"Pressure too high: {pressure:.2f}")

        time.sleep(1)

def apply_calibration():
    global calib_a, calib_b
    try:
        calib_a = float(calib_a_entry.get())
        calib_b = float(calib_b_entry.get())
        messagebox.showinfo("Success", "Calibration values updated!")
    except:
        messagebox.showerror("Error", "Invalid calibration values")

def update_plot():
    ax.clear()
    ax.plot(pressure_data)
    ax.set_title("Pressure over Time")
    ax.set_ylabel("Pressure")
    ax.set_xlabel("Sample")
    canvas.draw()

# ===== GUI Layout =====

root = tk.Tk()
root.title("Qualcomm Pressure Monitoring GUI")
root.geometry("800x600")

title = tk.Label(root, text="Pressure Sensor Monitoring - Qualcomm Yocto", font=("Arial", 18))
title.pack(pady=10)

pressure_label = tk.Label(root, text="Current Pressure: 0.000", font=("Arial", 16))
pressure_label.pack(pady=10)

# Controls frame
control_frame = tk.Frame(root)
control_frame.pack(pady=10)

start_btn = ttk.Button(control_frame, text="Start", command=start_reading)
start_btn.grid(row=0, column=0, padx=10)

stop_btn = ttk.Button(control_frame, text="Stop", command=stop_reading)
stop_btn.grid(row=0, column=1, padx=10)

# Threshold input
threshold_label = tk.Label(control_frame, text="Threshold:")
threshold_label.grid(row=1, column=0)
threshold_entry = tk.Entry(control_frame)
threshold_entry.insert(0, "1000")
threshold_entry.grid(row=1, column=1)

# Calibration inputs
calib_label = tk.Label(root, text="Calibration Settings (Pressure = a*Voltage + b)", font=("Arial", 14))
calib_label.pack()

calib_frame = tk.Frame(root)
calib_frame.pack()

tk.Label(calib_frame, text="a:").grid(row=0, column=0)
calib_a_entry = tk.Entry(calib_frame)
calib_a_entry.insert(0, "1.0")
calib_a_entry.grid(row=0, column=1)

tk.Label(calib_frame, text="b:").grid(row=0, column=2)
calib_b_entry = tk.Entry(calib_frame)
calib_b_entry.insert(0, "0.0")
calib_b_entry.grid(row=0, column=3)

apply_btn = ttk.Button(calib_frame, text="Apply", command=apply_calibration)
apply_btn.grid(row=0, column=4, padx=10)

# Graph frame
fig = plt.Figure(figsize=(8, 4), dpi=100)
ax = fig.add_subplot(111)
canvas = FigureCanvasTkAgg(fig, master=root)
canvas.get_tk_widget().pack()

root.mainloop()

