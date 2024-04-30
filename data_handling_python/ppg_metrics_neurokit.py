import neurokit2 as nk
import json
import numpy as np
import heartpy as hp
import matplotlib.pyplot as plt
import scipy.signal

def plot_signal(signal, time, title):
    # Plotting the signal
    plt.figure(figsize=(8, 4))  # Create a new figure (optional: set figsize)
    plt.plot(time, signal, label=title)  # Plot signal against time
    plt.title('Plot of a' + title)  # Set plot title
    plt.xlabel('Time')  # Label for x-axis
    plt.ylabel('Amplitude')  # Label for y-axis
    plt.grid(True)  # Show grid
    plt.legend()  # Show legend
    plt.show()  # Display the plot

def separate_shirt_data(data):
    timestamps = []
    chest_coil = []
    abdomen_coil = []
    gsr = []
    ppg_ir = []
    ppg_red = []
    
    for key, values in data.items():
        timestamps.append(str(values['timestamp']))
        chest_coil.append(int(values['chest_coil']))
        abdomen_coil.append(int(values['abdomen_coil']))
        gsr.append(int(values['gsr']))
        ppg_ir.append(int(values['ppg_ir']))
        ppg_red.append(int(values['ppg_red']))
    
    return {
        'timestamps': timestamps,
        'chest_coil': chest_coil,
        'abdomen_coil': abdomen_coil,
        'gsr': gsr,
        'ppg_ir': ppg_ir,
        'ppg_red': ppg_red
    }

def preprocess_ppg_signal(signal):
    signal = scipy.signal.detrend(signal)
    signal = hp.filter_signal(signal, cutoff = [0.5, 5], sample_rate = 20.0, filtertype='bandpass', return_top = False)
    return signal

# Specify the path to your JSON file
file_path = 'fyp_sensor_data_12.json'

# Step 1: Open and read the JSON file
with open(file_path, 'r') as file:
    json_data = json.load(file)

# Step 2: Extract the PPG data
data = json_data['UsersData']
user_data = data['GS5IERwOapdXlupUOqk5y52Vb5m1']
shirt_data = user_data['shirt_data']
ppg_data = separate_shirt_data(shirt_data)

ppg_red = ppg_data['ppg_red']
ppg_ir = ppg_data['ppg_ir']
timestamps = ppg_data['timestamps']

# Step 1: Preprocess the PPG data
ppg_ir = preprocess_ppg_signal(ppg_ir)
ppg_red = preprocess_ppg_signal(ppg_red)

# Step 2: Extract HRV, HR and RR
signals, info = nk.ppg_process(ppg_signal = ppg_ir, report = "text", sampling_rate=20)

# Visualize the processing
nk.ppg_plot(signals, info)