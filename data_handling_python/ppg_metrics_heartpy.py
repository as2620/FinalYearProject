import json
import numpy as np
import heartpy as hp
import matplotlib.pyplot as plt
import scipy.signal


#TODO: Extract SPO2
# Sp02 = 110 - 25 * (R/IR) where R = RMS(red)/MEAN(red) and IR = RMS(IR)/MEAN(IR)
# Currently extratcing HR, HRV and BR

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

def extract_spo2(ir_sig, red_sig):
    # Change data type to int32 to avoid overflow errors when squaring
    ir_sig = np.array(ir_sig, dtype=np.longfloat)
    red_sig = np.array(red_sig, dtype=np.longfloat)

    red_mean = np.mean(red_sig)
    ir_mean = np.mean(ir_sig)
    
    red_rms = np.sqrt(np.mean(np.array(red_sig)**2))
    ir_rms = np.sqrt(np.mean(np.array(ir_sig)**2))

    # print("Red RMS: %f" %red_rms)
    # print("IR RMS: %f" %ir_rms)
    # print("Red Mean: %f" %red_mean)
    # print("IR Mean: %f" %ir_mean)
    
    red = red_rms/red_mean
    ir = ir_rms/ir_mean

    # print("Red: %f" %red)
    # print("IR: %f" %ir)

    R = red/ir
    
    # spo2 = 110 - (25 * R)
    spo2 =  ((-45.060*R*R)/10000) + ((30.354*R)/100) + 94.845 
    
    return spo2

# Specify the path to your JSON file
file_path = 'fyp_sensor_data_16_hold_breath.json'

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

# # Plot
# plot_signal(ppg_ir, timestamps, 'IR Signal')

# Step 3: Preprocess the PPG data
ppg_ir = preprocess_ppg_signal(ppg_ir)
ppg_red = preprocess_ppg_signal(ppg_red)

# Step 4: Extract SPO2
ppg_spo2 = extract_spo2(ppg_ir, ppg_red)

# # Plot
# plot_signal(ppg_ir, timestamps, 'IR Signal')

# Step 5: Extract HRV, HR and RR
wd, m = hp.process(ppg_ir, 20, high_precision=True, reject_segmentwise=True)

# # Plot Rejected Peaks
plt.figure(figsize=(12,4))
hp.plotter(wd, m)
plt.show()

# Display measures computed
print('HR: ',m['bpm'])
print('HRV: ', m['rmssd'])
print(m)


print('SPO2: %f' %ppg_spo2)
