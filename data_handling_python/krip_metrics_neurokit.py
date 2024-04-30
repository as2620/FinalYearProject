import json
import numpy as np
import heartpy as hp
import matplotlib.pyplot as plt
import scipy
import neurokit2 as nk

def plot_signal(signal, time, title):
    # Plotting the signal
    plt.figure(figsize=(8, 4))  # Create a new figure (optional: set figsize)
    plt.plot(time, signal, label=title)  # Plot signal against time
    plt.title('Plot of a ' + title)  # Set plot title
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

# Specify the path to your JSON file
file_path = 'fyp_sensor_data_15_all.json'

# Step 1: Open and read the JSON file
with open(file_path, 'r') as file:
    json_data = json.load(file)

# Step 2: Extract the KRIP data
data = json_data['UsersData']
user_data = data['GS5IERwOapdXlupUOqk5y52Vb5m1']
shirt_data = user_data['shirt_data']
krip_data = separate_shirt_data(shirt_data)

chest_coil = krip_data['chest_coil']
abdomen_coil = krip_data['abdomen_coil']
timestamps = krip_data['timestamps']

chest_coil = scipy.signal.detrend(chest_coil)
abdomen_coil = scipy.signal.detrend(abdomen_coil)

chest_coil = hp.filter_signal(chest_coil, cutoff = 1, sample_rate = 20.0, filtertype='lowpass', return_top = False)
abdomen_coil = hp.filter_signal(abdomen_coil, cutoff = 1, sample_rate = 20.0, filtertype='lowpass', return_top = False)

# Find RR
# Process it
signals, info = nk.rsp_process(abdomen_coil, sampling_rate=20)

# Visualise the processing
nk.rsp_plot(signals, info)
plt.show()

print("RR: ", np.average(signals['RSP_Rate']))
print("Amplitude: ", np.average(signals['RSP_Amplitude']))

