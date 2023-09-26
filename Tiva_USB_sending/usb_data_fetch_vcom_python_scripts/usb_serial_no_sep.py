import serial

# Replace 'COM21' with your actual COM port
com_port = 'COM21'

# Open the serial port
ser = serial.Serial(com_port, baudrate=9600, timeout=1)

# Define the output file name
output_file = 'intan_readings.txt'

# Open the output file in write mode
with open(output_file, 'w') as f:
    buffer = b''  # Initialize an empty buffer
    while True:
        # Read data from the serial port
        data = ser.read(2)  # Read two bytes

        if len(data) == 2:
            # Convert the bytes to an integer (assuming little-endian order)
            adc_value = int.from_bytes(data, byteorder='little')

            # Write the value to the output file
            f.write(str(adc_value) + '\n')
            
            f.flush()  # Flush the buffer to ensure data is written immediately

# Close the serial port
ser.close()
