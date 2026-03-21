#!/usr/bin/env python3
"""
TRON Vanity Address Hunter - GUI Launcher
Wraps profanity.exe with a simple graphical interface.
"""

import tkinter as tk
from tkinter import ttk, filedialog, scrolledtext
import subprocess
import threading
import os
import sys


class TronHunterGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("TRON Address Hunter")
        self.root.geometry("720x580")
        self.root.resizable(True, True)
        self.process = None

        self._build_ui()

    def _build_ui(self):
        # --- Matching Pattern ---
        frame_match = ttk.LabelFrame(self.root, text="Matching Pattern", padding=8)
        frame_match.pack(fill="x", padx=10, pady=(10, 5))

        ttk.Label(frame_match, text="Pattern (20 chars):").grid(row=0, column=0, sticky="w")
        self.var_pattern = tk.StringVar(value="TTTTTTTTTT8888888888")
        entry_pattern = ttk.Entry(frame_match, textvariable=self.var_pattern, width=40, font=("Consolas", 11))
        entry_pattern.grid(row=0, column=1, padx=5, sticky="ew")

        ttk.Label(frame_match, text="Or file:").grid(row=0, column=2, padx=(10, 0))
        self.var_file = tk.StringVar()
        ttk.Entry(frame_match, textvariable=self.var_file, width=20).grid(row=0, column=3, padx=5)
        ttk.Button(frame_match, text="Browse", command=self._browse_file).grid(row=0, column=4)

        frame_match.columnconfigure(1, weight=1)

        # --- Parameters ---
        frame_params = ttk.LabelFrame(self.root, text="Parameters", padding=8)
        frame_params.pack(fill="x", padx=10, pady=5)

        # Row 0
        ttk.Label(frame_params, text="Prefix Count:").grid(row=0, column=0, sticky="w")
        self.var_prefix = tk.IntVar(value=0)
        ttk.Spinbox(frame_params, from_=0, to=10, textvariable=self.var_prefix, width=5).grid(row=0, column=1, padx=5)

        ttk.Label(frame_params, text="Suffix Count:").grid(row=0, column=2, sticky="w", padx=(20, 0))
        self.var_suffix = tk.IntVar(value=6)
        ttk.Spinbox(frame_params, from_=0, to=10, textvariable=self.var_suffix, width=5).grid(row=0, column=3, padx=5)

        ttk.Label(frame_params, text="Quit After:").grid(row=0, column=4, sticky="w", padx=(20, 0))
        self.var_quit = tk.IntVar(value=1)
        ttk.Spinbox(frame_params, from_=0, to=9999, textvariable=self.var_quit, width=5).grid(row=0, column=5, padx=5)

        # Row 1
        ttk.Label(frame_params, text="Skip GPU:").grid(row=1, column=0, sticky="w", pady=(8, 0))
        self.var_skip = tk.StringVar(value="1")
        ttk.Entry(frame_params, textvariable=self.var_skip, width=8).grid(row=1, column=1, padx=5, pady=(8, 0))
        ttk.Label(frame_params, text="(comma separated, e.g. 1,2)").grid(row=1, column=2, columnspan=2, sticky="w", padx=(20, 0), pady=(8, 0))

        ttk.Label(frame_params, text="Output File:").grid(row=1, column=4, sticky="w", padx=(20, 0), pady=(8, 0))
        self.var_output = tk.StringVar(value="result.txt")
        ttk.Entry(frame_params, textvariable=self.var_output, width=12).grid(row=1, column=5, padx=5, pady=(8, 0))

        # --- Control Buttons ---
        frame_btn = ttk.Frame(self.root, padding=5)
        frame_btn.pack(fill="x", padx=10)

        self.btn_start = ttk.Button(frame_btn, text="Start", command=self._start)
        self.btn_start.pack(side="left", padx=5)

        self.btn_stop = ttk.Button(frame_btn, text="Stop", command=self._stop, state="disabled")
        self.btn_stop.pack(side="left", padx=5)

        self.btn_clear = ttk.Button(frame_btn, text="Clear Log", command=self._clear_log)
        self.btn_clear.pack(side="left", padx=5)

        self.label_status = ttk.Label(frame_btn, text="Ready", foreground="gray")
        self.label_status.pack(side="right", padx=5)

        # --- Log Output ---
        frame_log = ttk.LabelFrame(self.root, text="Output", padding=5)
        frame_log.pack(fill="both", expand=True, padx=10, pady=(5, 10))

        self.log = scrolledtext.ScrolledText(frame_log, wrap="word", font=("Consolas", 10), bg="#1e1e1e", fg="#00ff00")
        self.log.pack(fill="both", expand=True)

    def _browse_file(self):
        path = filedialog.askopenfilename(filetypes=[("Text files", "*.txt"), ("All files", "*.*")])
        if path:
            self.var_file.set(path)

    def _get_exe_path(self):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        exe = os.path.join(script_dir, "profanity.exe")
        if os.path.exists(exe):
            return exe
        # fallback: same directory as cwd
        if os.path.exists("profanity.exe"):
            return os.path.abspath("profanity.exe")
        return None

    def _build_command(self, matching_path):
        exe = self._get_exe_path()
        if not exe:
            return None

        cmd = [exe, "--matching", matching_path]
        cmd += ["--prefix-count", str(self.var_prefix.get())]
        cmd += ["--suffix-count", str(self.var_suffix.get())]

        quit_count = self.var_quit.get()
        if quit_count > 0:
            cmd += ["--quit-count", str(quit_count)]

        output_file = self.var_output.get().strip()
        if output_file:
            cmd += ["--output", output_file]

        skip_str = self.var_skip.get().strip()
        if skip_str:
            for s in skip_str.split(","):
                s = s.strip()
                if s.isdigit():
                    cmd += ["--skip", s]

        return cmd

    def _start(self):
        exe = self._get_exe_path()
        if not exe:
            self._log("ERROR: profanity.exe not found! Place it in the same folder as this script.\n")
            return

        # Determine matching input
        file_path = self.var_file.get().strip()
        pattern = self.var_pattern.get().strip()

        if file_path and os.path.exists(file_path):
            matching_path = file_path
        elif len(pattern) == 20:
            # Write pattern to temp file
            script_dir = os.path.dirname(os.path.abspath(__file__))
            matching_path = os.path.join(script_dir, "_gui_pattern.txt")
            with open(matching_path, "w", encoding="ascii", newline="") as f:
                f.write(pattern)
        elif len(pattern) == 34 and pattern.startswith("T"):
            matching_path = pattern
        else:
            self._log("ERROR: Pattern must be exactly 20 characters, or a 34-char TRON address, or select a file.\n")
            return

        cmd = self._build_command(matching_path)
        if not cmd:
            return

        self._log("Command: " + " ".join(cmd) + "\n\n")
        self.btn_start.config(state="disabled")
        self.btn_stop.config(state="normal")
        self.label_status.config(text="Running...", foreground="green")

        thread = threading.Thread(target=self._run_process, args=(cmd,), daemon=True)
        thread.start()

    def _run_process(self, cmd):
        try:
            startupinfo = None
            if sys.platform == "win32":
                startupinfo = subprocess.STARTUPINFO()
                startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW

            self.process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                startupinfo=startupinfo,
                cwd=os.path.dirname(os.path.abspath(__file__)),
            )

            for line in iter(self.process.stdout.readline, b""):
                text = line.decode("utf-8", errors="replace")
                # Filter out VT100 clear line codes for cleaner display
                text = text.replace("\33[2K\r", "")
                if text.strip():
                    self.root.after(0, self._log, text)

            self.process.wait()
            self.root.after(0, self._on_finished)

        except Exception as e:
            self.root.after(0, self._log, f"ERROR: {e}\n")
            self.root.after(0, self._on_finished)

    def _stop(self):
        if self.process:
            self.process.terminate()
            self._log("\n--- Stopped by user ---\n")

    def _on_finished(self):
        self.process = None
        self.btn_start.config(state="normal")
        self.btn_stop.config(state="disabled")
        self.label_status.config(text="Finished", foreground="gray")

    def _log(self, text):
        self.log.insert("end", text)
        self.log.see("end")

    def _clear_log(self):
        self.log.delete("1.0", "end")


if __name__ == "__main__":
    root = tk.Tk()
    app = TronHunterGUI(root)
    root.mainloop()
