#NOTE: there is a seperator of 0xFF for this code. This works quite well. If the seperator is 0x00, or 0x00, it doesn't
#seperate the data properly

import serial

# Replace 'COM21' with your actual COM port
com_port = 'COM21'

# Open the serial port
ser = serial.Serial(com_port, baudrate=9600, timeout=1)

# Define the separator value
separator = 0xffff

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

            if adc_value == separator:
                # Write the buffered data and clear the buffer
                if buffer:
                    f.write(str(int.from_bytes(buffer, byteorder='little')) + '\n')
                buffer = b''  # Clear the buffer
            else:
                # Add the data to the buffer
                buffer += data
            
            f.flush()  # Flush the buffer to ensure data is written immediately

# Close the serial port
ser.close()
