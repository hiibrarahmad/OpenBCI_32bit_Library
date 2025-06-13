#!/usr/bin/env python3
"""
plot_4ch_gui_fixed.py

A Tkinter‐based GUI for plotting exactly four channels (Ch1–Ch4) from a CSV,
with the Matplotlib toolbar placed in its own Frame so pack()/grid() do not conflict.

Features:
  • Loads a CSV with columns: Timestamp, Sample, Ch1, Ch2, Ch3, Ch4
  • Four checkboxes (default: all checked) to toggle each channel on/off
  • “Plot Combined” button redraws a single figure with up to 4 subplots (one per checked channel)
  • Embedded Matplotlib toolbar (zoom/pan) always visible
  • Responsive layout: canvas and toolbar resize with window
"""

import tkinter as tk
from tkinter import filedialog, messagebox
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
import matplotlib.pyplot as plt
import pandas as pd


class FourChannelPlotter(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("4-Channel CSV Plotter")
        self.rowconfigure(0, weight=1)
        self.columnconfigure(1, weight=1)

        # Exactly four channels; adjust names here if your CSV differs
        self.channels = ["Ch1", "Ch2", "Ch3", "Ch4"]
        self.df = None
        self.vars = {}  # maps channel name → IntVar for checkbox

        self._build_left_panel()
        self._build_right_panel()

    def _build_left_panel(self):
        left = tk.Frame(self, padx=10, pady=10)
        left.grid(row=0, column=0, sticky="ns")
        left.columnconfigure(0, weight=1)

        # 1) Load CSV button
        btn_load = tk.Button(left, text="Load CSV…", command=self._on_load_csv)
        btn_load.grid(row=0, column=0, sticky="ew", pady=(0, 10))

        # 2) Four checkboxes, one per channel
        for i, ch in enumerate(self.channels, start=1):
            var = tk.IntVar(value=1)
            cb = tk.Checkbutton(
                left,
                text=ch,
                variable=var,
                command=self._refresh_plot,  # redraw when toggled
            )
            cb.grid(row=i, column=0, sticky="w", pady=2)
            self.vars[ch] = var

        # 3) Plot Combined button
        btn_plot = tk.Button(left, text="Plot Combined", command=self._plot_combined)
        btn_plot.grid(row=len(self.channels) + 1, column=0, sticky="ew", pady=(10, 0))

    def _build_right_panel(self):
        right = tk.Frame(self, padx=5, pady=5)
        right.grid(row=0, column=1, sticky="nsew")
        right.rowconfigure(1, weight=1)   # row 1 holds the canvas
        right.columnconfigure(0, weight=1)

        # Placeholder label (before data is loaded)
        self.placeholder = tk.Label(
            right,
            text="No data loaded.\nClick “Load CSV…” to select your file.",
            justify="center",
            fg="gray",
        )
        self.placeholder.grid(row=0, column=0, sticky="nsew")

        # Create Matplotlib figure
        self.fig = plt.Figure(figsize=(6, 5), tight_layout=True)

        # 1) Create a container Frame for the toolbar (so toolbar can pack itself inside it)
        self.toolbar_container = tk.Frame(right)
        self.toolbar_container.grid(row=0, column=0, sticky="ew")

        # 2) Create the canvas (but don't grid it until after CSV is loaded)
        self.canvas = FigureCanvasTkAgg(self.fig, master=right)
        self.canvas_widget = self.canvas.get_tk_widget()
        self.canvas_widget.grid(row=1, column=0, sticky="nsew")

        # Hide toolbar container + canvas until CSV is loaded
        self.toolbar_container.grid_remove()
        self.canvas_widget.grid_remove()

    def _on_load_csv(self):
        """Open file dialog, read CSV, verify columns, then show toolbar+canvas."""
        path = filedialog.askopenfilename(
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")]
        )
        if not path:
            return

        try:
            # Read CSV and parse Timestamp into datetime (if present)
            self.df = pd.read_csv(path, parse_dates=["Timestamp"])
        except Exception as e:
            messagebox.showerror("Error", f"Could not read CSV:\n{e}")
            return

        # Verify that the four required channels exist
        missing = [ch for ch in self.channels if ch not in self.df.columns]
        if missing:
            messagebox.showerror(
                "Missing Columns",
                f"The CSV is missing these columns: {missing}\n"
                "Ensure you have Timestamp, Sample, Ch1, Ch2, Ch3, Ch4.",
            )
            self.df = None
            return

        # Remove placeholder, then show toolbar container + canvas
        self.placeholder.grid_remove()
        self.toolbar_container.grid()       # now we can place the toolbar
        self.canvas_widget.grid()

        # Instantiate the toolbar (it will pack() itself inside toolbar_container)
        self.toolbar = NavigationToolbar2Tk(self.canvas, self.toolbar_container)
        self.toolbar.update()

        # Automatically draw for the first time (all four channels checked by default)
        self._plot_combined()

    def _get_checked(self):
        """Return only those channels whose checkbox is checked."""
        return [ch for ch, var in self.vars.items() if var.get() == 1]

    def _clear_fig(self):
        """Clear all axes from the figure."""
        self.fig.clf()

    def _plot_combined(self):
        """Plot each checked channel in its own subplot (stacked vertically)."""
        if self.df is None:
            return

        checked = self._get_checked()
        if not checked:
            self._clear_fig()
            self.canvas.draw()
            return

        n = len(checked)
        self._clear_fig()

        for idx, ch in enumerate(checked, start=1):
            ax = self.fig.add_subplot(n, 1, idx)
            # Plot vs. Timestamp if present; else plot index
            if "Timestamp" in self.df.columns:
                ax.plot(self.df["Timestamp"], self.df[ch], label=ch)
                ax.set_xlabel("Timestamp")
            else:
                ax.plot(self.df.index, self.df[ch], label=ch)
                ax.set_xlabel("Index")
            ax.set_ylabel(ch)
            ax.grid(True)

        self.fig.tight_layout()
        self.canvas.draw()  # redraw on the embedded canvas

    def _refresh_plot(self):
        """Called whenever a checkbox toggles—just redraw."""
        self._plot_combined()


if __name__ == "__main__":
    app = FourChannelPlotter()
    app.geometry("900x600")
    app.mainloop()
