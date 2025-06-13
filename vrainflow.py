import time
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

from brainflow.data_filter import DataFilter, FilterTypes, AggOperations, NoiseTypes

def main():
    # 1) Load the raw OpenBCI CSV
    #    - adjust skiprows to get past the header block
    df = pd.read_csv(
        r"C:\Users\hiibr\Downloads\BrainFlow-RAW_2025-05-29_17-34-09_1.csv",
        skiprows=7  # usually OpenBCI GUI puts 7 header lines; tweak if needed
    )

    # 2) Identify your EEG columns by name
    eeg_cols = [c for c in df.columns if c.lower().startswith("eeg")]

    # 3) Convert to data[channel_index, sample_index]
    data = df[eeg_cols].to_numpy().T

    # 4) Plot before filtering
    plt.figure()
    for i, col in enumerate(eeg_cols):
        plt.subplot(len(eeg_cols), 1, i+1)
        plt.plot(data[i, :])
        plt.title(col)
    plt.tight_layout()
    plt.savefig("before_processing.png")

    # 5) Apply your filters in place
    fs = 250  # set this to whatever sampling rate you used
    for idx in range(len(eeg_cols)):
        if idx == 0:
            DataFilter.perform_bandpass(
                data[idx], fs, 2.0, 50.0, 4, FilterTypes.BESSEL_ZERO_PHASE, 0
            )
        # … your other per-channel filters …
        else:
            DataFilter.remove_environmental_noise(
                data[idx], fs, NoiseTypes.FIFTY.value
            )

    # 6) Plot after filtering
    plt.figure()
    for i, col in enumerate(eeg_cols):
        plt.subplot(len(eeg_cols), 1, i+1)
        plt.plot(data[i, :])
        plt.title(col + " (filtered)")
    plt.tight_layout()
    plt.savefig("after_processing.png")

if __name__ == "__main__":
    main()
