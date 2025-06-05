import os
import sys
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

def main():
    # ────────────────────────────────────────────────────────────────────────────────
    # 1) CSV_PATH: either hardcode your CSV path here, or leave empty ("")
    #    and the script will prompt you to type it at runtime.
    CSV_PATH = ""  # e.g. r"C:\Users\Alice\Data\openbci_live.csv" or "/home/alice/openbci_live.csv"
    # ────────────────────────────────────────────────────────────────────────────────

    # If CSV_PATH is empty, ask the user
    if not CSV_PATH:
        CSV_PATH = input("D:\arduino library\libraries\OpenBCI_32bit_Library\output.csv").strip()

    if not os.path.isfile(CSV_PATH):
        print(f"ERROR: File not found: {CSV_PATH!r}")
        sys.exit(1)

    # ────────────────────────────────────────────────────────────────────────────────
    # 2) Which four channels do you want to plot?  
    #    Must match your CSV column names exactly (e.g. "Ch1", "Ch2", …).  
    #    Feel free to change to any four columns in your file (e.g. ["Ch5","Ch6","Ch7","Ch8"]).
    channels_to_plot = ["Ch1", "Ch2", "Ch3", "Ch4"]
    # ────────────────────────────────────────────────────────────────────────────────

    # 3) Read CSV into pandas, parse the "Timestamp" column as datetime
    try:
        df = pd.read_csv(
            CSV_PATH,
            parse_dates=["Timestamp"],      # parse that column as datetime
            infer_datetime_format=True,
            dayfirst=False                  # adjust if your dates are day‐month‐year
        )
    except Exception as e:
        print(f"ERROR: Could not read CSV: {e}")
        sys.exit(1)

    # Confirm that required columns exist
    missing = [c for c in (["Timestamp"] + channels_to_plot) if c not in df.columns]
    if missing:
        print(f"ERROR: The following columns are missing in CSV: {missing}")
        print(f"Available columns: {list(df.columns)}")
        sys.exit(1)

    # 4) Optionally, sort by timestamp (in case your CSV isn’t already time‐ordered)
    df = df.sort_values(by="Timestamp").reset_index(drop=True)

    # 5) Create four stacked subplots (one per channel), sharing the x‐axis (time)
    plt.style.use("seaborn‐darkgrid")
    fig, axes = plt.subplots(
        nrows=4,
        ncols=1,
        sharex=True,
        figsize=(10, 8),
        constrained_layout=True
    )

    # Define a date‐formatter for the x‐axis
    locator = mdates.AutoDateLocator(minticks=6, maxticks=12)
    formatter = mdates.ConciseDateFormatter(locator)

    for i, ch in enumerate(channels_to_plot):
        ax = axes[i]
        ax.plot(df["Timestamp"], df[ch], lw=0.8, color=f"C{i}")
        ax.set_ylabel(ch, fontsize=10)
        ax.grid(True)

        # If you want to set a fixed y‐range for all channels, uncomment and adjust below:
        # ax.set_ylim(-9e6, 9e6)

        # Only format the x‐axis on the bottom subplot
        if i == 3:
            ax.xaxis.set_major_locator(locator)
            ax.xaxis.set_major_formatter(formatter)
            ax.set_xlabel("Time", fontsize=12)
        else:
            ax.tick_params(labelbottom=False)

    # 6) Add a shared title
    fig.suptitle("OpenBCI Channels " + ", ".join(channels_to_plot) + " vs. Time",
                 fontsize=14, y=1.02)

    # 7) Show the figure
    plt.show()

if __name__ == "__main__":
    main()
