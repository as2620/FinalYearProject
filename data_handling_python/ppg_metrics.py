import json
import numpy as np
import heartpy as hp
import matplotlib.pyplot as plt
import scipy
from scipy.signal import butter, lfilter

#TODO: Extract SPO2
# Currently extratcing HR, HRV and BR

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

# Plotting the signal
plt.figure(figsize=(8, 4))  # Create a new figure (optional: set figsize)
plt.plot(timestamps, ppg_ir, label='Signal')  # Plot signal against time
plt.title('Plot of a Signal')  # Set plot title
plt.xlabel('Time')  # Label for x-axis
plt.ylabel('Amplitude')  # Label for y-axis
plt.grid(True)  # Show grid
plt.legend()  # Show legend
plt.show()  # Display the plot

# Step 3: Preprocess the PPG data
# Remove DC component
ppg_ir = scipy.signal.detrend(ppg_ir)

ppg_ir = hp.filter_signal(ppg_ir, cutoff = [0.5, 5], sample_rate = 20.0, filtertype='bandpass', return_top = False)

# Plotting the signal
plt.figure(figsize=(8, 4))  # Create a new figure (optional: set figsize)
plt.plot(timestamps, ppg_ir, label='Signal')  # Plot signal against time
plt.title('Plot of a Signal')  # Set plot title
plt.xlabel('Time')  # Label for x-axis
plt.ylabel('Amplitude')  # Label for y-axis
plt.grid(True)  # Show grid
plt.legend()  # Show legend
plt.show()  # Display the plot

# Nomalize the signal
ppg_ir = hp.scale_data(ppg_ir)

# Step 4: Extract the heart rate
# Perform PPG analysis
wd, m = hp.process(ppg_ir, 20)

#plot
plt.figure(figsize=(12,4))
hp.plotter(wd, m)
plt.show()

#display measures computed
for measure in m.keys():
    print('%s: %f' %(measure, m[measure]))
