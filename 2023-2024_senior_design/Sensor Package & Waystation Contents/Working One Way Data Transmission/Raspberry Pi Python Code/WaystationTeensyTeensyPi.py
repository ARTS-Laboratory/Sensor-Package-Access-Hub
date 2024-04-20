import serial
import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft

def read_and_process_data(ser, num_points):
    """Read data from the serial and calculate the magnitude of the accelerometer data."""
    magnitudes = []
    count = 0
    x_val, y_val, z_val = 0, 0, 0
    
    while len(magnitudes) < num_points:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            try:
                value = float(line)
                if count % 3 == 0:
                    x_val = value
                elif count % 3 == 1:
                    y_val = value
                elif count % 3 == 2:
                    z_val = value
                    magnitude = np.sqrt(x_val**2 + y_val**2 + z_val**2)
                    magnitudes.append(magnitude)
                count += 1
            except ValueError as e:
                print(f"Data conversion error for '{line}': {e}")
    return magnitudes

def calculate_fft(data, sample_rate):
    """Calculate the FFT of the accelerometer data."""
    n = len(data)
    fft_values = fft(data)
    frequencies = np.fft.fftfreq(n, d=1/sample_rate)
    return frequencies[:n//2], np.abs(fft_values)[:n//2]

def calculate_statistics(data):
    """Calculate and return the mean and standard deviation of the data."""
    mean = np.mean(data)
    std_dev = np.std(data)
    return mean, std_dev

def plot_data(frequencies, fft_values, data, title):
    """Plot the FFT results along with mean and standard deviation."""
    mean, std_dev = calculate_statistics(data)

    plt.figure(figsize=(12, 6))
    plt.plot(frequencies, fft_values, label='FFT Amplitude')
    plt.axhline(y=mean, color='r', linestyle='--', label=f'Mean: {mean:.2f}')
    plt.axhline(y=mean + std_dev, color='g', linestyle=':', label=f'Mean + Std Dev: {mean + std_dev:.2f}')
    plt.axhline(y=mean - std_dev, color='b', linestyle=':', label=f'Mean - Std Dev: {mean - std_dev:.2f}')
    plt.title(f'FFT of {title} with Mean and Std Dev')
    plt.xlabel('Frequency (Hz)')
    plt.ylabel('Amplitude')
    plt.legend()
    plt.show()

def main():
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    num_data_points = 300  # Number of total magnitude points to collect
    sample_rate = 105  # Hz

    try:
        print("Collecting data...")
        magnitudes = read_and_process_data(ser, num_data_points)

        # Calculate and plot FFT for the combined magnitude
        freqs, fft_values = calculate_fft(magnitudes, sample_rate)
        plot_data(freqs, fft_values, magnitudes, "Combined Accelerometer Magnitude")

    except KeyboardInterrupt:
        ser.close()
        print("Serial connection closed.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()