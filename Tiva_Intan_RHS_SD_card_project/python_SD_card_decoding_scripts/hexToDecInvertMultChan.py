def invert_bytes(number):
    lower_byte = number & 0xFF
    upper_byte = (number >> 8) & 0xFF
    inverted_number = (lower_byte << 8) | upper_byte
    return inverted_number

def convert_hex_to_decimal(input_file, num_output_files):
    # Open the input file
    with open(input_file, 'r') as file:
        # Read the content of the file
        content = file.read()

    # Split the content into individual hexadecimal numbers
    hex_numbers = content.split()

    # Convert each hexadecimal number to decimal, invert the bytes
    decimal_numbers = [invert_bytes(int(hex_num, 16)) for hex_num in hex_numbers]

    # Create a list to hold the data for each output file
    output_data = [[] for _ in range(num_output_files)]

    # Distribute the decimal numbers to different output files in a round-robin fashion
    for index, decimal_num in enumerate(decimal_numbers):
        output_data[index % num_output_files].append(decimal_num)

    # Write data to each output file
    for i in range(num_output_files):
        output_file = f'output_{i+1}.txt'
        with open(output_file, 'w') as file:
            for decimal_num in output_data[i]:
                file.write(str(decimal_num) + '\n')


# Provide the input file path and number of output files to create
input_file = 'input.txt'
num_output_files = 3

# Call the function to convert hexadecimal to decimal, invert the bytes,
# and distribute the data among the specified number of output files
convert_hex_to_decimal(input_file, num_output_files)
