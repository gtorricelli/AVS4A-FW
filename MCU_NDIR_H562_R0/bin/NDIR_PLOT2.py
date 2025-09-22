import serial
import serial.tools.list_ports
import tkinter as tk
from tkinter import ttk
import threading
import numpy as np
from struct import unpack
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg


class SerialReaderApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Serial Data Reader")

        # Variabili GUI
        self.port_var = tk.StringVar()
        self.baudrate_var = tk.IntVar(value=115200)
        self.status_var = tk.StringVar(value="Stopped")

        # Configurazione GUI
        control_frame = ttk.Frame(root)
        control_frame.pack(side=tk.TOP, fill=tk.X, padx=5, pady=5)

        ttk.Label(control_frame, text="Serial Port:").grid(row=0, column=0, padx=5, pady=5)
        self.port_menu = ttk.Combobox(control_frame, textvariable=self.port_var, state="readonly")
        self.port_menu.grid(row=0, column=1, padx=5, pady=5)
        self.refresh_ports()  # Popola il menu con le porte disponibili
        refresh_button = ttk.Button(control_frame, text="Refresh", command=self.refresh_ports)
        refresh_button.grid(row=0, column=2, padx=5, pady=5)

        ttk.Label(control_frame, text="Baudrate:").grid(row=1, column=0, padx=5, pady=5)
        ttk.Entry(control_frame, textvariable=self.baudrate_var).grid(row=1, column=1, padx=5, pady=5)

        self.start_button = ttk.Button(control_frame, text="Start", command=self.start_acquisition)
        self.start_button.grid(row=2, column=0, padx=5, pady=5)

        self.stop_button = ttk.Button(control_frame, text="Stop", command=self.stop_acquisition, state=tk.DISABLED)
        self.stop_button.grid(row=2, column=1, padx=5, pady=5)

        ttk.Label(control_frame, text="Status:").grid(row=3, column=0, padx=5, pady=5)
        ttk.Label(control_frame, textvariable=self.status_var).grid(row=3, column=1, padx=5, pady=5)

        # Configurazione grafica
        self.figure = Figure(figsize=(10, 6))
        self.ax = self.figure.add_subplot(1, 1, 1)

        # Linee per i dati
        self.colors = ['red', 'green', 'blue', 'purple', 'black']
        self.Text = ['HC', 'R134A', 'R1234YF', 'R22', 'black']
        self.lines = [
            #self.ax.plot([], [], color=self.colors[i], label=f"Sensore {i + 1}" if i < 4 else "Digitale")[0]
            self.ax.plot([], [], color=self.colors[i], label=self.Text[i] if i < 4 else "Digitale")[0]
            for i in range(5)
        ]
        self.ax.legend()
        self.ax.set_xlim(0, 400)
        #self.ax.set_ylim(-1.1, 1.1)  # Dati normalizzati a 1.0
        self.ax.set_ylim(-0.6, 0.6)  # Dati normalizzati a 1.0

        # Testo per temperatura e pressione in alto a sinistra
        self.temperature_text = self.ax.text(10, 0.55, "", fontsize=10, color='blue', ha='left')
        self.pressure_text = self.ax.text(10, 0.50, "", fontsize=10, color='green', ha='left')

        # Incorporazione della figura nel tkinter
        canvas = FigureCanvasTkAgg(self.figure, root)
        canvas.get_tk_widget().pack(side=tk.BOTTOM, fill=tk.BOTH, expand=True)
        self.canvas = canvas

        # Variabili per la gestione dati
        self.serial_port = None
        self.running = False

        # Buffer per i dati
        self.sensors_data = [[] for _ in range(4)]
        self.digital_data = []
        self.temperature_data = []
        self.pressure_data = []

    def refresh_ports(self):
        """Aggiorna la lista delle porte seriali disponibili."""
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_menu['values'] = ports
        if ports:
            self.port_var.set(ports[0])  # Seleziona la prima porta disponibile

    def start_acquisition(self):
        try:
            self.serial_port = serial.Serial(
                self.port_var.get(),
                self.baudrate_var.get(),
                timeout=1
            )
            self.running = True
            self.status_var.set("Running")
            self.start_button.config(state=tk.DISABLED)
            self.stop_button.config(state=tk.NORMAL)
            threading.Thread(target=self.read_data).start()
        except Exception as e:
            self.status_var.set(f"Error: {e}")

    def stop_acquisition(self):
        self.running = False
        if self.serial_port:
            self.serial_port.close()
        self.status_var.set("Stopped")
        self.start_button.config(state=tk.NORMAL)
        self.stop_button.config(state=tk.DISABLED)

    def read_data(self):
        buffer_size = 431
        num_packets = 8
        try:
            while self.running:
                # Accumula i pacchetti
                data_blocks = []
                for _ in range(num_packets):
                    data = self.serial_port.read(buffer_size)
                    #self.debug_packet(data)  # Stampa il pacchetto per debug
                    if len(data) != buffer_size or data[:2] != b'\xAA\xBB':
                        continue  # Scarta pacchetti non validi
                    data_blocks.append(data)

                if len(data_blocks) == num_packets:
                    self.process_data(data_blocks)
                    
        except Exception as e:
            self.status_var.set(f"Error: {e}")
        finally:
            if self.serial_port:
                self.serial_port.close()

    def debug_packet(self, data):
        """Stampa il pacchetto ricevuto in formato esadecimale e la sua lunghezza."""
        print(f"Packet received (length {len(data)}): {data.hex() if data else 'Empty'}")

    def process_data(self, data_blocks):
        # Processa i dati e accumula nei buffer
        for block in data_blocks:
            for i in range(4):
                sensor_offset = 2 + i * 100
                self.sensors_data[i].extend(unpack('25i', block[sensor_offset:sensor_offset + 100]))
            self.digital_data.extend(block[402:427])
            self.temperature_data.append(unpack('h', block[427:429])[0])
            self.pressure_data.append(unpack('h', block[429:431])[0])

        # Se i buffer contengono 100 dati per sensore, aggiorna il grafico
        if len(self.sensors_data[0]) >= 400 and len(self.digital_data) >= 400:
            self.update_graph()

    def update_graph(self):
        # Fondo scala per normalizzazione
        print(f"Update Plot!!!!")
        sensor_max_values = [16777216, 16777216, 16777216, 16777216]  # Cambia secondo il fondo scala effettivo
        digital_max_value = 200

        # Normalizzazione e slicing dei dati
        sensors_data = [
            np.array(self.sensors_data[i][:400]) / max_val for i, max_val in enumerate(sensor_max_values)
        ]
       
        digital_data = np.array(self.digital_data[:400]) / digital_max_value

        # Rimuovi dati già utilizzati
        for i in range(4):
            self.sensors_data[i] = self.sensors_data[i][400:]
        self.digital_data = self.digital_data[400:]

        # Calcola medie
        avg_temp = np.mean(self.temperature_data[-4:])/10.0
        avg_press = np.mean(self.pressure_data[-4:])/10.0

        # Aggiorna grafico
        x = np.arange(400)
        for i, line in enumerate(self.lines[:4]):
            line.set_data(x, sensors_data[i])
            print("Valori dati:", sensors_data[i])

        self.lines[4].set_data(x, digital_data)
        self.ax.relim()
        self.ax.autoscale_view()

        # Aggiorna testo
        self.temperature_text.set_text(f"Temp Avg: {avg_temp:.2f}")
        self.pressure_text.set_text(f"Press Avg: {avg_press:.2f}")

        # Aggiorna canvas
        self.canvas.draw()


if __name__ == "__main__":
    root = tk.Tk()
    app = SerialReaderApp(root)
    root.mainloop()
