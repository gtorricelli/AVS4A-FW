#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
STM32 RS-485 Bootloader Host - GUI Tkinter (hot‑plug autoscan, NO reconfigure)
- Non riconfigura MAI la porta dopo l'apertura (niente ser.timeout dinamici).
- Letture non bloccanti con polling di in_waiting e scadenze temporali.
- Hot‑plug autoscan: apre/chiude porte al volo; handshake AA BB CC DD -> D0 D0.
- Protocollo: LEN <H, payload <I, ACK 0x79 / NAK 0x1F, PING con APP_BASE in pl[2..5].
"""
import os
import time
import struct
import binascii
import threading
import queue
import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext

import serial
from serial.tools import list_ports

try:
    from serial.rs485 import RS485Settings
except Exception:
    RS485Settings = None

# ---------------- Protocollo ----------------

SOF = 0x55
BROADCAST_ADDR = 0xFF

# Comandi
CMD_PING  = 0x01
CMD_INFO  = 0x02
CMD_BEGIN = 0x10
CMD_ERASE = 0x11
CMD_WRITE = 0x12
CMD_READ  = 0x13
CMD_END   = 0x14
CMD_GO    = 0x15
CMD_ABORT = 0x1F

# Risposte
RSP_ACK = 0x79
RSP_NAK = 0x1F

# Errori base
ERR_OK       = 0x00
ERR_BAD_CRC  = 0x01

# Parametri
DEFAULT_BAUD = 115200
DEFAULT_PARITY = "None"  # "None" or "Even"
DEFAULT_ADDR = 0x01
DEFAULT_TURN_MS = 2
DEFAULT_CHUNK = 128
MAX_CHUNK = 256
DEFAULT_ACK_TIMEOUT_S = 1.0
RETRIES = 5
HANDSHAKE_TX = bytes([0xAA, 0xBB, 0xCC, 0xDD])
HANDSHAKE_RX = bytes([0xD0, 0xD0])
HANDSHAKE_PERIOD_S = 2.0
PORT_REFRESH_UI_S = 1.0
SEARCH_TICK_S = 0.1

# ---------------- Util ----------------

def crc16_ccitt_false(data: bytes) -> int:
    crc = 0xFFFF
    for b in data:
        crc ^= (b << 8) & 0xFFFF
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

def crc32_ieee(data: bytes) -> int:
    return binascii.crc32(data) & 0xFFFFFFFF

def hexdump(b: bytes, maxlen: int = 64) -> str:
    if b is None:
        return ""
    data = b if len(b) <= maxlen else (b[:maxlen] + b"...")
    return " ".join(f"{x:02X}" for x in data)

def drain_input(ser: serial.Serial, max_bytes: int = 4096) -> bytes:
    """Legge tutto ciò che è disponibile SENZA bloccare e senza toccare ser.timeout."""
    data = bytearray()
    while True:
        avail = getattr(ser, "in_waiting", 0)
        if avail <= 0:
            break
        chunk = ser.read(min(avail, max_bytes - len(data)))
        if not chunk:
            break
        data.extend(chunk)
        if len(data) >= max_bytes:
            break
    return bytes(data)

def read_exact_nb(ser: serial.Serial, n: int, timeout_s: float) -> bytes:
    """Legge esattamente n byte entro timeout_s, non bloccante (no ser.timeout)."""
    deadline = time.time() + timeout_s
    out = bytearray()
    while len(out) < n:
        if time.time() >= deadline:
            raise TimeoutError("Timeout read_exact_nb")
        avail = getattr(ser, "in_waiting", 0)
        if avail > 0:
            chunk = ser.read(min(n - len(out), avail))
            if chunk:
                out.extend(chunk)
                continue
        time.sleep(0.001)
    return bytes(out)

def read_frame_nb(ser: serial.Serial, timeout_s: float):
    """Legge un frame (SOF + header + payload + CRC) senza modificare la porta."""
    deadline = time.time() + timeout_s
    # Cerca SOF
    while True:
        if time.time() >= deadline:
            return None
        avail = getattr(ser, "in_waiting", 0)
        if avail <= 0:
            time.sleep(0.001); continue
        b = ser.read(1)
        if not b:
            time.sleep(0.001); continue
        if b[0] == SOF:
            break
    # header: DST CMD LEN(LE) SEQ
    hdr = read_exact_nb(ser, 5, max(0.0, deadline - time.time()))
    dst, cmd = hdr[0], hdr[1]
    length = struct.unpack("<H", hdr[2:4])[0]
    seq = hdr[4]
    payload = b""
    if length > 0:
        payload = read_exact_nb(ser, length, max(0.0, deadline - time.time()))
    crc_rx = read_exact_nb(ser, 2, max(0.0, deadline - time.time()))
    crc_rx_val = struct.unpack("<H", crc_rx)[0]
    calc = crc16_ccitt_false(hdr + payload)
    if calc != crc_rx_val:
        raise ValueError(f"CRC errato: got 0x{crc_rx_val:04X}, calc 0x{calc:04X}")
    return dst, cmd, seq, payload

def build_frame(dst: int, cmd: int, seq: int, payload: bytes) -> bytes:
    if payload is None:
        payload = b""
    length = len(payload)
    hdr = bytes([dst & 0xFF, cmd & 0xFF]) + struct.pack("<H", length) + bytes([seq & 0xFF])
    crc = crc16_ccitt_false(hdr + payload)
    return bytes([SOF]) + hdr + payload + struct.pack("<H", crc)

# ---------------- Client ----------------

class BLClient:
    def __init__(self, ser: serial.Serial, node_addr: int, turnaround_ms: int = DEFAULT_TURN_MS,
                 ack_timeout_s: float = DEFAULT_ACK_TIMEOUT_S, verbose: bool = False, log_fn=None):
        self.ser = ser
        self.node_addr = node_addr & 0xFF
        self.seq = 0
        self.turn_ms = max(0, int(turnaround_ms))
        self.ack_timeout_s = max(0.1, float(ack_timeout_s))
        self.verbose = bool(verbose)
        self.log_fn = log_fn or (lambda s: None)

    def _log(self, msg: str):
        if self.verbose:
            self.log_fn(msg)

    def _next_seq(self):
        self.seq = (self.seq + 1) & 0xFF
        return self.seq

    def _xfer(self, dst: int, cmd: int, payload: bytes):
        seq = self._next_seq()
        frame = build_frame(dst, cmd, seq, payload)

        for attempt in range(RETRIES):
            # Drain RX prima della TX
            junk = drain_input(self.ser)
            if junk:
                self._log(f"[drain RX] {len(junk)} B: {hexdump(junk)}")

            # RX->TX delay
            if self.turn_ms:
                time.sleep(self.turn_ms / 1000.0)

            # TX
            self._log(f"[TX] dst={dst:02X} cmd={cmd:02X} seq={seq:02X} len={len(payload)} | {hexdump(frame)}")
            self.ser.write(frame)
            self.ser.flush()

            # TX->RX delay
            if self.turn_ms:
                time.sleep(self.turn_ms / 1000.0)

            # Attesa risposta non bloccante
            deadline = time.time() + self.ack_timeout_s
            while time.time() < deadline:
                try:
                    rsp = read_frame_nb(self.ser, max(0.0, deadline - time.time()))
                except TimeoutError:
                    break
                except ValueError as e:
                    self._log(f"[RX] frame scartato (CRC): {e}")
                    continue

                if rsp is None:
                    break

                dst_r, cmd_r, seq_r, pl = rsp
                self._log(f"[RX] dst={dst_r:02X} cmd={cmd_r:02X} seq={seq_r:02X} len={len(pl)}")

                # echo puro?
                if cmd_r == cmd and seq_r == seq and len(pl) == len(payload):
                    self._log("[RX] echo del comando -> ignoro")
                    continue

                # ACK/NAK
                if cmd_r == RSP_ACK:
                    if len(pl) < 1:
                        raise RuntimeError("ACK senza codice errore")
                    return True, pl[0], pl[1:]
                if cmd_r == RSP_NAK:
                    err = pl[0] if len(pl) else ERR_BAD_CRC
                    return False, err, pl[1:] if len(pl) > 1 else b""

                # Alcuni BL rispondono con stesso CMD/SEQ ma payload differente (status)
                if cmd_r == cmd and seq_r == seq and len(pl) != len(payload):
                    err_code = pl[0] if len(pl) >= 1 else 0
                    return True, err_code, pl[1:]

                self._log("[RX] frame non riconosciuto -> ignoro")
                continue

            if attempt < RETRIES - 1:
                self._log("[RETRY] nessuna risposta valida, ritento…")
                time.sleep(0.02)

        raise TimeoutError("Ritrasmissioni esaurite / nessun ACK")

    # ---- comandi high-level ----
    def ping_direct(self):
        ok, err, pl = self._xfer(self.node_addr, CMD_PING, b"")
        if not ok or err != ERR_OK or len(pl) < 8:
            raise RuntimeError("PING diretto fallito")
        if self.verbose:
            self._log(f"[PING payload dir] {hexdump(pl)}")
        bl_ver   = pl[0] | (pl[1] << 8)
        app_base = struct.unpack("<I", pl[2:6])[0]
        state    = pl[6]
        node     = pl[7]
        return bl_ver, app_base, state, node

    def ping_broadcast(self):
        ok, err, pl = self._xfer(BROADCAST_ADDR, CMD_PING, b"")
        if not ok or err != ERR_OK or len(pl) < 8:
            raise RuntimeError("PING broadcast fallito")
        if self.verbose:
            self._log(f"[PING payload bc]  {hexdump(pl)}")
        bl_ver   = pl[0] | (pl[1] << 8)
        app_base = struct.unpack("<I", pl[2:6])[0]
        state    = pl[6]
        node     = pl[7]
        return bl_ver, app_base, state, node

    def info(self):
        ok, err, pl = self._xfer(self.node_addr, CMD_INFO, b"")
        if not ok or err != ERR_OK or len(pl) < 8:
            raise RuntimeError("INFO fallito")
        page_size = struct.unpack("<I", pl[0:4])[0]
        flash_end = struct.unpack("<I", pl[4:8])[0]
        return page_size, flash_end

    def begin(self, total_size: int, crc32_val: int):
        pl = struct.pack("<II", total_size, crc32_val)
        ok, err, _ = self._xfer(self.node_addr, CMD_BEGIN, pl)
        if not ok or err != ERR_OK:
            raise RuntimeError(f"BEGIN err={err}")

    def erase(self, addr: int, length: int):
        pl = struct.pack("<II", addr, length)
        ok, err, _ = self._xfer(self.node_addr, CMD_ERASE, pl)
        if not ok or err != ERR_OK:
            raise RuntimeError(f"ERASE err={err}")

    def write_chunk(self, addr: int, data: bytes):
        if len(data) > MAX_CHUNK:
            raise ValueError(f"Chunk troppo grande: {len(data)} > {MAX_CHUNK}")
        pl = struct.pack("<I", addr) + data
        ok, err, _ = self._xfer(self.node_addr, CMD_WRITE, pl)
        if not ok or err != ERR_OK:
            raise RuntimeError(f"WRITE err={err} @0x{addr:08X}")

    def end(self):
        ok, err, pl = self._xfer(self.node_addr, CMD_END, b"")
        if not ok:
            raise RuntimeError(f"END NAK err={err}")
        if err == ERR_BAD_CRC:
            dev_crc = struct.unpack("<I", pl[:4])[0] if len(pl) >= 4 else None
            raise RuntimeError(f"CRC device differente (device=0x{dev_crc:08X} se presente)")

    def go(self):
        ok, err, _ = self._xfer(self.node_addr, CMD_GO, b"")
        if not ok or err != ERR_OK:
            raise RuntimeError(f"GO err={err}")

# ---------------- GUI ----------------

class BootloaderGUI:
    def __init__(self, root):
        self.root = root
        root.title("STM32 RS-485 Bootloader (hot‑plug, no‑reconfig)")
        root.geometry("900x600")
        root.minsize(860, 560)

        self.queue = queue.Queue()
        self.worker_thread = None
        self.abort_flag = threading.Event()

        self._build_widgets()
        self.refresh_ports()
        # Aggiorna la lista porte periodicamente
        self.root.after(int(PORT_REFRESH_UI_S*1000), self._refresh_ports_periodic)
        # Avvia polling coda
        self.root.after(50, self.process_queue)

    def _build_widgets(self):
        frm = ttk.Frame(self.root, padding=12)
        frm.pack(fill="both", expand=True)

        # riga 1: porta + refresh
        row1 = ttk.Frame(frm)
        row1.pack(fill="x", pady=(0,8))
        ttk.Label(row1, text="Porta seriale:").pack(side="left")
        self.cmb_port = ttk.Combobox(row1, width=28, state="readonly")
        self.cmb_port.pack(side="left", padx=6)
        ttk.Button(row1, text="Aggiorna", command=self.refresh_ports).pack(side="left")
        self.autoscan_var = tk.BooleanVar(value=True)
        ttk.Checkbutton(row1, text="Ricerca automatica", variable=self.autoscan_var).pack(side="left", padx=(8,0))

        # opzioni RS-485
        self.rs485_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(row1, text="Usa RTS come DE", variable=self.rs485_var).pack(side="left", padx=(8,0))

        # baud + parità + indirizzo + turnaround + chunk + timeout + verbose
        row2 = ttk.Frame(frm)
        row2.pack(fill="x", pady=(0,8))
        ttk.Label(row2, text="Baud:").pack(side="left")
        self.ent_baud = ttk.Entry(row2, width=8)
        self.ent_baud.insert(0, str(DEFAULT_BAUD))
        self.ent_baud.pack(side="left", padx=(6,12))

        ttk.Label(row2, text="Parità:").pack(side="left")
        self.cmb_parity = ttk.Combobox(row2, values=("None", "Even"), state="readonly", width=6)
        self.cmb_parity.set(DEFAULT_PARITY)
        self.cmb_parity.pack(side="left", padx=(6,12))

        ttk.Label(row2, text="Indirizzo nodo (hex):").pack(side="left")
        self.ent_addr = ttk.Entry(row2, width=8)
        self.ent_addr.insert(0, "0x01")
        self.ent_addr.pack(side="left", padx=(6,12))

        ttk.Label(row2, text="Delay TX-RX (ms):").pack(side="left")
        self.ent_turn = ttk.Entry(row2, width=6)
        self.ent_turn.insert(0, str(DEFAULT_TURN_MS))
        self.ent_turn.pack(side="left", padx=(6,12))

        ttk.Label(row2, text="Chunk (B, max 256):").pack(side="left")
        self.ent_chunk = ttk.Entry(row2, width=6)
        self.ent_chunk.insert(0, str(DEFAULT_CHUNK))
        self.ent_chunk.pack(side="left", padx=(6,12))

        ttk.Label(row2, text="ACK timeout (s):").pack(side="left")
        self.ent_ackto = ttk.Entry(row2, width=6)
        self.ent_ackto.insert(0, str(DEFAULT_ACK_TIMEOUT_S))
        self.ent_ackto.pack(side="left", padx=(6,12))

        self.chk_verbose_var = tk.BooleanVar(value=True)
        self.chk_verbose = ttk.Checkbutton(row2, text="Verbose log (hex)", variable=self.chk_verbose_var)
        self.chk_verbose.pack(side="left", padx=(12,0))

        # file
        row3 = ttk.Frame(frm)
        row3.pack(fill="x", pady=(0,8))
        ttk.Label(row3, text="File .bin:").pack(side="left")
        self.ent_file = ttk.Entry(row3)
        self.ent_file.pack(side="left", fill="x", expand=True, padx=6)
        ttk.Button(row3, text="Sfoglia…", command=self.browse_file).pack(side="left")

        # progress + pulsanti
        row4 = ttk.Frame(frm)
        row4.pack(fill="x", pady=(0,8))
        self.pbar = ttk.Progressbar(row4, mode="determinate")
        self.pbar.pack(side="left", fill="x", expand=True, padx=(0,8))
        self.btn_start = ttk.Button(row4, text="Avvia Programmazione", command=self.start)
        self.btn_start.pack(side="left", padx=(0,8))
        self.btn_abort = ttk.Button(row4, text="Annulla", command=self.abort, state="disabled")
        self.btn_abort.pack(side="left")

        # log
        ttk.Label(frm, text="Log:").pack(anchor="w")
        self.txt_log = scrolledtext.ScrolledText(frm, height=16, wrap="word")
        self.txt_log.pack(fill="both", expand=True)
        self.txt_log.configure(font=("Consolas", 10))

        # status bar
        self.status = tk.StringVar(value="Pronto")
        ttk.Label(self.root, textvariable=self.status, anchor="w", relief="sunken").pack(side="bottom", fill="x")

    def _refresh_ports_periodic(self):
        self.refresh_ports()
        self.root.after(int(PORT_REFRESH_UI_S*1000), self._refresh_ports_periodic)

    def refresh_ports(self):
        ports = [p.device for p in list_ports.comports()]
        current = set(self.cmb_port["values"]) if self.cmb_port["values"] else set()
        if set(ports) != set(current):
            self.cmb_port["values"] = ports
            if ports and not self.cmb_port.get():
                self.cmb_port.set(ports[0])

    def browse_file(self):
        path = filedialog.askopenfilename(title="Seleziona file .bin", filetypes=[("Binary files", "*.bin"), ("Tutti i file", "*.*")])
        if path:
            self.ent_file.delete(0, tk.END)
            self.ent_file.insert(0, path)

    def append_log(self, text):
        self.txt_log.insert(tk.END, text + "\n")
        self.txt_log.see(tk.END)

    def set_busy(self, busy: bool):
        self.btn_start.configure(state="disabled" if busy else "normal")
        self.btn_abort.configure(state="normal" if busy else "disabled")
        for w in (self.cmb_port, self.ent_baud, self.cmb_parity, self.ent_addr,
                  self.ent_turn, self.ent_chunk, self.ent_ackto, self.ent_file, self.chk_verbose):
            try:
                w.configure(state="disabled" if busy else ("readonly" if isinstance(w, ttk.Combobox) else "normal"))
            except Exception:
                pass

    def start(self):
        # Forza autoscan se non c'è porta selezionata
        autoscan = bool(self.autoscan_var.get())
        if not self.cmb_port.get().strip():
            autoscan = True
            self.autoscan_var.set(True)

        port = self.cmb_port.get().strip()
        try:
            baud = int(self.ent_baud.get().strip(), 10)
        except ValueError:
            messagebox.showwarning("Baud non valido", "Inserisci un baud numerico (es. 115200).")
            return

        parity_name = self.cmb_parity.get()
        parity = serial.PARITY_NONE if parity_name == "None" else serial.PARITY_EVEN

        try:
            node_addr = int(self.ent_addr.get().strip(), 0) & 0xFF
        except ValueError:
            messagebox.showwarning("Indirizzo non valido", "Inserisci l'indirizzo in formato numerico (es. 0x01).")
            return

        try:
            turn_ms = max(0, int(self.ent_turn.get().strip(), 10))
        except ValueError:
            messagebox.showwarning("Turnaround non valido", "Inserisci un valore intero (ms).")
            return

        try:
            chunk = max(1, min(MAX_CHUNK, int(self.ent_chunk.get().strip(), 10)))
        except ValueError:
            messagebox.showwarning("Chunk non valido", "Inserisci un valore intero tra 1 e 256.")
            return

        try:
            ack_to_s = max(0.1, float(self.ent_ackto.get().strip()))
        except ValueError:
            messagebox.showwarning("ACK timeout non valido", "Inserisci un numero (secondi).")
            return

        path = self.ent_file.get().strip()
        if not path or not os.path.isfile(path):
            messagebox.showwarning("File mancante", "Seleziona un file .bin valido.")
            return

        # reset UI
        self.txt_log.delete("1.0", tk.END)
        self.pbar["value"] = 0
        self.pbar["maximum"] = 100
        self.status.set("Ricerca scheda…")
        self.abort_flag.clear()
        self.set_busy(True)

        verbose = self.chk_verbose_var.get()
        use_rs485 = bool(self.rs485_var.get())
        args = (autoscan, port, baud, parity, node_addr, path, turn_ms, chunk, ack_to_s, verbose, use_rs485)
        self.worker_thread = threading.Thread(target=self.worker, args=args, daemon=True)
        self.worker_thread.start()

    def abort(self):
        if self.worker_thread and self.worker_thread.is_alive():
            self.abort_flag.set()
            self.queue.put(("log", "[UTENTE] Annullamento richiesto…"))

    def process_queue(self):
        try:
            while True:
                kind, payload = self.queue.get_nowait()
                if kind == "log":
                    self.append_log(payload)
                elif kind == "progress_max":
                    self.pbar["maximum"] = max(1, int(payload))
                    self.pbar["value"] = 0
                elif kind == "progress":
                    self.pbar["value"] = int(payload)
                elif kind == "status":
                    self.status.set(payload)
                elif kind == "error":
                    self.set_busy(False)
                    self.status.set("Errore")
                    self.append_log(f"[ERRORE] {payload}")
                    messagebox.showerror("Errore", payload)
                elif kind == "done":
                    self.set_busy(False)
                    self.status.set("Completato")
                    messagebox.showinfo("Completato", "Programmazione completata.")
                else:
                    pass
        except queue.Empty:
            pass
        finally:
            self.root.after(50, self.process_queue)

    # --------------- HANDSHAKE ---------------
    def handshake(self, ser: serial.Serial, turn_ms: int, verbose: bool) -> None:
        """Handshake su singola porta SENZA toccare ser.timeout (polling non bloccante)."""
        self.queue.put(("status", "Handshake… (AA BB CC DD)"))
        buf = bytearray()
        last_tx = 0.0
        while not self.abort_flag.is_set():
            now = time.time()
            if now - last_tx >= HANDSHAKE_PERIOD_S:
                junk = drain_input(ser)
                if verbose and junk:
                    self.queue.put(("log", f"[HS drain] {len(junk)} B: {hexdump(junk)}"))
                if turn_ms:
                    time.sleep(turn_ms / 1000.0)
                ser.write(HANDSHAKE_TX); ser.flush()
                if verbose:
                    self.queue.put(("log", f"[HS TX] {hexdump(HANDSHAKE_TX)}"))
                if turn_ms:
                    time.sleep(turn_ms / 1000.0)
                last_tx = now

            # RX non bloccante
            avail = getattr(ser, "in_waiting", 0)
            if avail > 0:
                data = ser.read(min(256, avail))
                if data:
                    buf.extend(data)
                    if verbose:
                        self.queue.put(("log", f"[HS RX] {hexdump(data)}"))
                    if HANDSHAKE_RX in buf:
                        self.queue.put(("log", "[HS] ricevuto D0 D0, procedo"))
                        time.sleep(0.05)
                        drain_input(ser)
                        return
                    if len(buf) > 1024:
                        del buf[:512]
            time.sleep(0.005)

        raise RuntimeError("Handshake annullato dall'utente")

    def handshake_multi(self, baud: int, parity, turn_ms: int, verbose: bool, use_rs485: bool):
        """
        Ricerca continua su TUTTE le seriali di sistema (hot‑plug).
        Non modifica proprietà delle porte una volta aperte.
        """
        self.queue.put(("status", "Ricerca scheda su tutte le seriali…"))
        open_map = {}  # name -> dict(ser, rxbuf, last_tx)
        last_refresh = 0.0
        REFRESH_PERIOD_S = 1.0

        while not self.abort_flag.is_set():
            now = time.time()

            # Refresh elenco porte
            if now - last_refresh >= REFRESH_PERIOD_S:
                last_refresh = now
                current = {p.device for p in list_ports.comports()}

                # Chiudi porte scomparse
                for name in list(open_map.keys()):
                    if name not in current:
                        try:
                            if verbose: self.queue.put(("log", f"[HS] {name}: rimosso, chiudo"))
                            open_map[name]["ser"].close()
                        except Exception:
                            pass
                        del open_map[name]

                # Apri nuove porte
                for name in sorted(current):
                    if name in open_map:
                        continue
                    try:
                        ser = serial.Serial(
                            port=name, baudrate=baud, bytesize=serial.EIGHTBITS,
                            parity=parity, stopbits=serial.STOPBITS_ONE,
                            timeout=0, write_timeout=2.0,  # timeout fisso=0 (non bloccante)
                            rtscts=False, dsrdtr=False, xonxoff=False
                        )
                        if use_rs485 and RS485Settings is not None:
                            # rs485_mode impostato SOLO all'apertura
                            ser.rs485_mode = RS485Settings(
                                rts_level_for_tx=True,
                                rts_level_for_rx=False,
                                delay_before_tx=turn_ms/1000.0 if turn_ms else None,
                                delay_after_tx=turn_ms/1000.0 if turn_ms else None,
                                loopback=False
                            )
                        open_map[name] = {"ser": ser, "rxbuf": bytearray(), "last_tx": 0.0}
                        if verbose: self.queue.put(("log", f"[HS] {name}: aperta"))
                    except Exception as e:
                        if verbose: self.queue.put(("log", f"[HS] {name}: apertura fallita: {e}"))
                        continue

                if not open_map and verbose:
                    self.queue.put(("log", "[HS] Nessuna porta disponibile: attesa collegamento…"))

            # TX periodico su tutte
            for name, entry in list(open_map.items()):
                ser = entry["ser"]
                if now - entry["last_tx"] >= HANDSHAKE_PERIOD_S:
                    junk = drain_input(ser)
                    if verbose and junk:
                        self.queue.put(("log", f"[HS {name} drain] {len(junk)} B: {hexdump(junk)}"))
                    if turn_ms:
                        time.sleep(turn_ms/1000.0)
                    try:
                        ser.write(HANDSHAKE_TX); ser.flush()
                        if verbose: self.queue.put(("log", f"[HS {name} TX] {hexdump(HANDSHAKE_TX)}"))
                    except Exception as e:
                        if verbose: self.queue.put(("log", f"[HS {name}] TX errore: {e} -> chiudo"))
                        try: ser.close()
                        except Exception: pass
                        del open_map[name]
                        continue
                    if turn_ms:
                        time.sleep(turn_ms/1000.0)
                    entry["last_tx"] = now

            # RX su tutte
            for name, entry in list(open_map.items()):
                ser = entry["ser"]
                avail = getattr(ser, "in_waiting", 0)
                if avail > 0:
                    try:
                        data = ser.read(min(256, avail))
                    except Exception as e:
                        if verbose: self.queue.put(("log", f"[HS {name}] RX errore: {e} -> chiudo"))
                        try: ser.close()
                        except Exception: pass
                        del open_map[name]
                        continue

                    if data:
                        entry["rxbuf"].extend(data)
                        if verbose:
                            self.queue.put(("log", f"[HS {name} RX] {hexdump(data)}"))
                        if HANDSHAKE_RX in entry["rxbuf"]:
                            self.queue.put(("log", f"[HS] {name}: ricevuto D0 D0, userò questa porta"))
                            chosen = ser
                            chosen_name = name
                            # chiudi le altre
                            for other_name, other_entry in list(open_map.items()):
                                s2 = other_entry["ser"]
                                if s2 is not chosen:
                                    try: s2.close()
                                    except Exception: pass
                                    del open_map[other_name]
                            self.queue.put(("status", f"Trovata scheda su {chosen_name}"))
                            time.sleep(0.05)
                            drain_input(chosen)
                            return chosen

                        if len(entry["rxbuf"]) > 4096:
                            del entry["rxbuf"][:2048]

            time.sleep(SEARCH_TICK_S)

        raise RuntimeError("Ricerca annullata dall'utente.")

    # --------------- WORKER ---------------
    def worker(self, autoscan, port, baud, parity, node_addr, path, turn_ms, chunk, ack_to_s, verbose, use_rs485):
        def log_fn(msg: str):
            self.queue.put(("log", msg))

        try:
            with open(path, "rb") as f:
                image = f.read()
            total_size = len(image)
            self.queue.put(("log", f"[FILE] {os.path.basename(path)} - {total_size} bytes"))
            self.queue.put(("log", f"[SERIAL] turnaround={turn_ms} ms | chunk={chunk} B | ACK TO={int(ack_to_s*1000)} ms"))
            self.queue.put(("progress_max", total_size))
        except Exception as e:
            self.queue.put(("error", str(e)))
            return

        while not self.abort_flag.is_set():
            try:
                if autoscan:
                    ser = self.handshake_multi(baud, parity, turn_ms, verbose, use_rs485)
                else:
                    self.queue.put(("status", f"Apro {port} @ {baud} ({'8N1' if parity==serial.PARITY_NONE else '8E1'})"))
                    ser = serial.Serial(
                        port=port, baudrate=baud, bytesize=serial.EIGHTBITS,
                        parity=parity, stopbits=serial.STOPBITS_ONE,
                        timeout=0, write_timeout=2.0,
                        rtscts=False, dsrdtr=False, xonxoff=False
                    )
                    if use_rs485 and RS485Settings is not None:
                        ser.rs485_mode = RS485Settings(
                            rts_level_for_tx=True,
                            rts_level_for_rx=False,
                            delay_before_tx=turn_ms/1000.0 if turn_ms else None,
                            delay_after_tx=turn_ms/1000.0 if turn_ms else None,
                            loopback=False
                        )
                    self.handshake(ser, turn_ms, verbose)
            except Exception as e:
                self.queue.put(("error", f"Impossibile aprire/ricercare la porta: {e}"))
                return

            try:
                client = BLClient(ser, node_addr, turnaround_ms=turn_ms, ack_timeout_s=ack_to_s,
                                  verbose=verbose, log_fn=log_fn)

                try:
                    ser.reset_input_buffer()
                    ser.reset_output_buffer()
                except Exception:
                    pass

                # PING con fallback
                if self.abort_flag.is_set(): raise RuntimeError("Operazione annullata.")
                self.queue.put(("status", "PING…"))
                self.queue.put(("log", "[STEP] PING indirizzato…"))
                try:
                    bl_ver, app_base, state, node = client.ping_direct()
                except Exception as e1:
                    self.queue.put(("log", f"[PING indirizzato] nessuna risposta: {e1}"))
                    self.queue.put(("log", "[STEP] PING broadcast…"))
                    bl_ver, app_base, state, node = client.ping_broadcast()
                self.queue.put(("log", f"OK | BL v{bl_ver & 0xFF}.{(bl_ver >> 8) & 0xFF}, APP_BASE=0x{app_base:08X}, state={state}, node={node}"))

                # INFO
                if self.abort_flag.is_set(): raise RuntimeError("Operazione annullata.")
                self.queue.put(("status", "INFO…"))
                self.queue.put(("log", "[STEP] INFO…"))
                page_size, flash_end = client.info()
                self.queue.put(("log", f"OK | page={page_size} bytes, flash_end=0x{flash_end:08X}"))

                # BEGIN
                if self.abort_flag.is_set(): raise RuntimeError("Operazione annullata.")
                self.queue.put(("status", "BEGIN…"))
                self.queue.put(("log", "[STEP] BEGIN (size+crc32)…"))
                client.begin(total_size, 0)
                self.queue.put(("log", "OK"))

                # ERASE
                if self.abort_flag.is_set(): raise RuntimeError("Operazione annullata.")
                erase_len = ((total_size + page_size - 1) // page_size) * page_size
                self.queue.put(("status", "ERASE…"))
                self.queue.put(("log", f"[STEP] ERASE 0x{app_base:08X} .. +{erase_len} bytes …"))
                client.erase(app_base, erase_len)
                self.queue.put(("log", "OK"))

                # WRITE
                if self.abort_flag.is_set(): raise RuntimeError("Operazione annullata.")
                self.queue.put(("status", "WRITE…"))
                self.queue.put(("log", f"[STEP] WRITE chunks da {chunk} B…"))
                written = 0
                addr = app_base

                while written < total_size:
                    if self.abort_flag.is_set():
                        try:
                            client._xfer(node_addr, CMD_ABORT, b"")
                        except Exception:
                            pass
                        raise RuntimeError("Operazione annullata.")

                    chunk_bytes = image[written: written + chunk]
                    if verbose:
                        addr_le = struct.pack("<I", addr)
                        self.queue.put(("log", f"[WRITE] addr={addr:08X} -> bytes LE={hexdump(addr_le)} len={len(chunk_bytes)}"))
                    client.write_chunk(addr, chunk_bytes)
                    written += len(chunk_bytes)
                    addr += (len(chunk_bytes) + 3) & ~3  # allineamento 4
                    self.queue.put(("progress", written))

                # END
                if self.abort_flag.is_set(): raise RuntimeError("Operazione annullata.")
                self.queue.put(("status", "END…"))
                self.queue.put(("log", "[STEP] END (verifica CRC)…"))
                client.end()
                self.queue.put(("log", "OK"))

                # GO
                if self.abort_flag.is_set(): raise RuntimeError("Operazione annullata.")
                self.queue.put(("status", "GO…"))
                self.queue.put(("log", "[STEP] GO (avvio app)…"))
                client.go()
                self.queue.put(("log", "OK"))

                self.queue.put(("done", None))
                try: ser.close()
                except Exception: pass
                return

            except Exception as e:
                try: ser.close()
                except Exception: pass
                self.queue.put(("error", str(e)))
                return

        self.queue.put(("error", "Operazione annullata dall'utente."))

# ---------------- MAIN ----------------

def main():
    root = tk.Tk()
    try:
        root.call("tk", "scaling", 1.2)
    except Exception:
        pass

    app = BootloaderGUI(root)
    root.mainloop()

if __name__ == "__main__":
    main()
