import json
import numpy as np
import heartpy as hp
import matplotlib.pyplot as plt
import scipy


# Import the required packages
import numpy as np
from scipy.fft import rfft, rfftfreq
import matplotlib.pyplot as plt

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
file_path = '5_bpm_ajw.json'

# Step 1: Open and read the JSON file
with open(file_path, 'r') as file:
    json_data = json.load(file)

# Step 2: Extract the KRIP data
data = json_data['UsersData']
user_data = data['GS5IERwOapdXlupUOqk5y52Vb5m1']
shirt_data = user_data['shirt_data']
krip_data = separate_shirt_data(shirt_data)

# chest_coil = krip_data['chest_coil']
# abdomen_coil = krip_data['abdomen_coil']
timestamps = krip_data['timestamps']
ppg_red = krip_data['chest_coil']

# chest_coil = scipy.signal.detrend(chest_coil)
# abdomen_coil = scipy.signal.detrend(abdomen_coil)

# chest_coil = hp.filter_signal(chest_coil, cutoff = 1, sample_rate = 20.0, filtertype='lowpass', return_top = False)
# abdomen_coil = hp.filter_signal(abdomen_coil, cutoff = 1, sample_rate = 20.0, filtertype='lowpass', return_top = False)

ppg_red = scipy.signal.detrend(ppg_red)
ppg_red= hp.filter_signal(ppg_red, cutoff = 1, sample_rate = 20.0, filtertype='lowpass', return_top = False)

# Step 3: Extract the RR using FFT
sample_rate = 20

# Chest Coil
# # Calculate N/2 to normalize the FFT output
N = len(ppg_red)

# Plot the actual spectrum of the signal
freq = rfftfreq(N, d=1/sample_rate)
amplitudes = 2*np.abs(rfft(ppg_red))/N

rr_in_hz =  freq[np.argmax(amplitudes)]
rr_in_bpm = rr_in_hz*60

print("RR in Hz: ", rr_in_hz)
print("RR in BPM: ", rr_in_bpm)

# Plot the FFT
plt.figure(figsize=(8, 4))  # Create a new figure (optional: set figsize)
plt.plot(freq, amplitudes)  # Plot signal against time
plt.title('FFT of Chest Coil')  # Set plot title
plt.xlabel('Frequency (Hz)')  # Label for x-axis
plt.ylabel('Amplitude')  # Label for y-axis
plt.grid(True)  # Show grid
plt.legend()  # Show legend
plt.show()  # Display the plot

# Abdomen Coil
# # Calculate N/2 to normalize the FFT output
# N = len(abdomen_coil)

# # Plot the actual spectrum of the signal
# freq = rfftfreq(N, d=1/sample_rate)
# amplitudes = 2*np.abs(rfft(abdomen_coil))/N

# rr_in_hz =  freq[np.argmax(amplitudes)]
# rr_in_bpm = rr_in_hz*60

# print("RR in Hz: ", rr_in_hz)
# print("RR in BPM: ", rr_in_bpm)

# # Plot the FFT
# plt.figure(figsize=(8, 4))  # Create a new figure (optional: set figsize)
# plt.plot(freq, amplitudes)  # Plot signal against time
# plt.title('FFT of Abdomen Coil')  # Set plot title
# plt.xlabel('Frequency (Hz)')  # Label for x-axis
# plt.ylabel('Amplitude')  # Label for y-axis
# plt.grid(True)  # Show grid
# plt.legend()  # Show legend
# plt.show()  # Display the plot