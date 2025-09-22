import serial
import serial.tools.list_ports
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import threading
import numpy as np
import pandas as pd
from datetime import datetime
from struct import unpack
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg


class SerialReaderApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Serial Data Reader")

        # GUI variables
        self.port_var = tk.StringVar()
        self.baudrate_var = tk.IntVar(value=115200)
        self.status_var = tk.StringVar(value="Stopped")

        # Build GUI -----------------------------------------------------------
        control_frame = ttk.Frame(root)
        control_frame.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)

        ttk.Label(control_frame, text="Serial Port:").grid(row=0, column=0, padx=5, pady=5)
        self.port_menu = ttk.Combobox(control_frame, textvariable=self.port_var, state="readonly")
        self.port_menu.grid(row=0, column=1, padx=5, pady=5)
        self.refresh_ports()
        ttk.Button(control_frame, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=5, pady=5)

        ttk.Label(control_frame, text="Baudrate:").grid(row=1, column=0, padx=5, pady=5)
        ttk.Entry(control_frame, textvariable=self.baudrate_var).grid(row=1, column=1, padx=5, pady=5)

        self.start_button = ttk.Button(control_frame, text="Start", command=self.start_acquisition)
        self.start_button.grid(row=2, column=0, padx=5, pady=5)

        self.stop_button = ttk.Button(control_frame, text="Stop", command=self.stop_acquisition, state=tk.DISABLED)
        self.stop_button.grid(row=2, column=1, padx=5, pady=5)

        # NEW: Save button (disabled until data available)
        self.save_button = ttk.Button(control_frame, text="Save to Excel", command=self.save_to_excel, state=tk.DISABLED)
        self.save_button.grid(row=2, column=2, padx=5, pady=5)

        ttk.Label(control_frame, text="Status:").grid(row=3, column=0, padx=5, pady=5)
        ttk.Label(control_frame, textvariable=self.status_var).grid(row=3, column=1, padx=5, pady=5)

        # Plot setup ----------------------------------------------------------
        self.figure = Figure(figsize=(10, 6))
        self.ax = self.figure.add_subplot(1, 1, 1)

        self.colors = ['red', 'green', 'blue', 'purple', 'black']
        self.labels = ['HC', 'R134A', 'R1234YF', 'R22', 'Digital']
        self.lines = [
            self.ax.plot([], [], color=self.colors[i], label=self.labels[i])[0]
            for i in range(5)
        ]
        self.ax.legend()
        self.ax.set_xlim(0, 400)
        self.ax.set_ylim(-0.6, 0.6)

        self.temperature_text = self.ax.text(10, 0.55, "", fontsize=10, color='blue', ha='left')
        self.pressure_text   = self.ax.text(10, 0.50, "", fontsize=10, color='green', ha='left')

        self.canvas = FigureCanvasTkAgg(self.figure, root)
        self.canvas.get_tk_widget().pack(side=tk.BOTTOM, fill=tk.BOTH, expand=True)

        # Data handling -------------------------------------------------------
        self.serial_port = None
        self.running = False

        self.sensors_data   = [[] for _ in range(4)]  # raw sensor buffers
        self.digital_data   = []
        self.temperature_data = []
        self.pressure_data    = []

        # Holds the last dataset shown on screen (normalized)
        self.latest_snapshot = None

    # ---------------------------------------------------------------------
    # GUI helpers
    # ---------------------------------------------------------------------
    def refresh_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_menu['values'] = ports
        if ports:
            self.port_var.set(ports[0])

    def start_acquisition(self):
        try:
            self.serial_port = serial.Serial(self.port_var.get(), self.baudrate_var.get(), timeout=1)
            self.running = True
            self.status_var.set("Running")
            self.start_button.config(state=tk.DISABLED)
            self.stop_button.config(state=tk.NORMAL)
            self.save_button.config(state=tk.DISABLED)
            threading.Thread(target=self.read_data, daemon=True).start()
        except Exception as e:
            self.status_var.set(f"Error: {e}")

    def stop_acquisition(self):
        self.running = False
        if self.serial_port:
            self.serial_port.close()
        self.status_var.set("Stopped")
        self.start_button.config(state=tk.NORMAL)
        self.stop_button.config(state=tk.DISABLED)

    # ---------------------------------------------------------------------
    # Serial reading & processing
    # ---------------------------------------------------------------------
    def read_data(self):
        buffer_size = 431
        num_packets = 8
        try:
            while self.running:
                data_blocks = []
                for _ in range(num_packets):
                    data = self.serial_port.read(buffer_size)
                    if len(data) != buffer_size or data[:2] != b'\xAA\xBB':
                        continue
                    data_blocks.append(data)
                if len(data_blocks) == num_packets:
                    self.process_data(data_blocks)
        except Exception as e:
            self.status_var.set(f"Error: {e}")
        finally:
            if self.serial_port:
                self.serial_port.close()

    def process_data(self, data_blocks):
        for block in data_blocks:
            for i in range(4):
                sensor_offset = 2 + i * 100
                self.sensors_data[i].extend(unpack('25i', block[sensor_offset:sensor_offset + 100]))
            self.digital_data.extend(block[402:427])
            self.temperature_data.append(unpack('h', block[427:429])[0])
            self.pressure_data.append(unpack('h', block[429:431])[0])

        if len(self.sensors_data[0]) >= 400 and len(self.digital_data) >= 400:
            self.update_graph()

    # ---------------------------------------------------------------------
    # Plot update
    # ---------------------------------------------------------------------
    def update_graph(self):
        sensor_max_values = [16777216] * 4  # adjust if needed
        digital_max_value = 200

        sensors_norm = [np.array(self.sensors_data[i][:400]) / sensor_max_values[i] for i in range(4)]
        digital_norm = np.array(self.digital_data[:400]) / digital_max_value

        # Remove consumed data
        for i in range(4):
            self.sensors_data[i] = self.sensors_data[i][400:]
        self.digital_data = self.digital_data[400:]

        # Store snapshot for export ------------------
        self.latest_snapshot = {
            'HC': sensors_norm[0],
            'R134A': sensors_norm[1],
            'R1234YF': sensors_norm[2],
            'R22': sensors_norm[3],
            'Digital': digital_norm
        }
        # Enable save button now that we have data
        self.save_button.config(state=tk.NORMAL)

        # Plot ---------------------------------------------------------------
        x = np.arange(400)
        for i, line in enumerate(self.lines[:4]):
            line.set_data(x, sensors_norm[i])
        self.lines[4].set_data(x, digital_norm)
        self.ax.relim()
        self.ax.autoscale_view()

        avg_temp = np.mean(self.temperature_data[-4:]) / 10.0
        avg_press = np.mean(self.pressure_data[-4:]) / 10.0
        self.temperature_text.set_text(f"Temp Avg: {avg_temp:.2f}")
        self.pressure_text.set_text(f"Press Avg: {avg_press:.2f}")

        self.canvas.draw()

    # ---------------------------------------------------------------------
    # Save to Excel
    # ---------------------------------------------------------------------
    def save_to_excel(self):
        if not self.latest_snapshot:
            messagebox.showwarning("No data", "No dataset available to save yet!")
            return

        file_path = filedialog.asksaveasfilename(
            defaultextension=".xlsx",
            filetypes=[("Excel files", "*.xlsx")],
            title="Save dataset as Excel"
        )
        if not file_path:
            return  # user canceled

        try:
            df = pd.DataFrame(self.latest_snapshot)
            df.index.name = 'Sample'
            df.to_excel(file_path, index=True)
            self.status_var.set(f"Saved to {file_path}")
            messagebox.showinfo("Success", f"Data saved to:{file_path}")
        except Exception as e:
            messagebox.showerror("Error", str(e))
            self.status_var.set(f"Error: {e}")


if __name__ == "__main__":
    root = tk.Tk()
    app = SerialReaderApp(root)
    root.mainloop()
