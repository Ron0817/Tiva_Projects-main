def convert_hex_to_decimal(input_file, output_file, offset):
    # Open the input file
    with open(input_file, 'r') as file:
        # Read the content of the file
        content = file.read()

    # Split the content into individual hexadecimal numbers
    hex_numbers = content.split()

    # Convert each hexadecimal number to decimal
    decimal_numbers = [int(hex_num, 16) for hex_num in hex_numbers]

    # Swap uint16_t hex two LSBs and two MSBs to make alignment
    # decimal_numbers_aligned = [( ((num & 0x0011) << 8 ) | (num >> 8) ) for num in decimal_numbers ]
    decimal_numbers_aligned = []
    for num in decimal_numbers:
        decimal_numbers_aligned.append( (num & 0xff) << 8 | (num >> 8) )
        # print(num, (num & 0xff), (num & 0x0011) << 8, (num >> 8))

    # Open the output file
    i = 1
    with open(output_file, 'w') as file:
        # Write the decimal numbers to the file
        for decimal_num in decimal_numbers_aligned:
            file.write(str(decimal_num) + ' ')
            if i % 6 == 0:
                file.write('\n')
            i += 1    


# Provide the input and output file paths
input_file = 'input.txt'
output_file = 'output.txt'
offset = 1

# Call the function to convert hexadecimal to decimal
convert_hex_to_decimal(input_file, output_file, offset)