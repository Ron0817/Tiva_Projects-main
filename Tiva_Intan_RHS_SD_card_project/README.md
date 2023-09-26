# Intan RHS SD card project
The project records the sampled data on the intan headstage channel(s) and writes the data to an SD card. 
There are also 4 configurable stimulators. All project configurations are done in the **config.txt file**,
which needs to be on the SD card before inserting it into the SD card reader. 
### Config.txt file parameters
*Notes:*
1. Total sampling frequency (sampling_frequency * channel numbers) ≤ 60k
2. buffer_size of 8192 or larger won’t work
3. cutoff_upper can be any value of the following: 20000, 15000, 10000, 7500, 5000,
3000, 2500, 2000, 1500, 1000, 750, 500, 300, 250, 200, 150, 100; other values won’t
work
4. cutoff_lower can be any value of the following: 1000, 500, 300, 250, 150, 100, 75,
50, 30, 25, 20, 15, 10, 5, 3, 2, 1; other values won’t work
5. channel: can be a combination of any value from 0 to 15, no need to be sorted, but
should be separated by a comma<br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/f3f84d78-f0bd-4532-adf1-11c9f813f505" width="300"> <br />
6. Parameters for stimulators are also user-defined in the .txt file<br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/8572a7ce-7cbb-48ac-8901-a48d57cded94" width="300"> <br />

## Launchpad pinout and SD card reader connections
**The following devices are used for the project:**
- Tiva C series TM4C123G LaunchPad: https://www.ti.com/tool/EK-TM4C123GXL <br />
- Intan LVDS Adapter (RHS): https://intantech.com/LVDS_adapter_board.html <br />
- SparkFun microSD Transflash Breakout: https://www.sparkfun.com/products/544 <br />
- KOOTION 32GB Micro SD Card: https://www.amazon.ca/KOOTION-Micro-UHS-I-Speed-Memory/dp/B07WM1P1QY/ref=sr_1_6?qid=1693932447&refinements=p_89%3AKOOTION&s=pc&sr=1-6&th=1 <br />
- RHS 16-Channel Stim/Recording Headstage: https://intantech.com/RHS_headstages.html?tabSelect=RHS16ch&yPos=0 <br />
- SPI Interface Cable: https://intantech.com/RHD_SPI_cables.html?tabSelect=RHDSPIcables <br />

**Pinout:**<br />
**Intan** <br />
PA3 -> CS  <br />
PA2 -> SCLK  <br />
PA5 -> MOSI1  <br />
PA4 -> MISO1  <br />

**SD Card Reader**<br />
PF3 -> CS <br />
PF1 -> DI (MOSI) <br />
PF2 -> SCK <br />
PF0 -> D0 (MISO) <br />

<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/58db655a-663a-4d68-af1e-a9492f42a0fc" width="300"> <br />
## Decoding and displaying data/post-processing
Here is the way data is recorded and decoded to be displayed on a PC:<br />
1. Recorded data from running the program is saved on multiple files on the SD card. Insert the SD card into PC and open one file with Sublime text 3: https://www.sublimetext.com/3 <br />
2. If data is not already displayed in hex format, re-open in hex formatting:<br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/be8b48e2-761d-4e49-ae05-4922a9f267c7" width="300"> <br />
3. Copy the hex data  <br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/9dc01802-2025-4956-99ad-22b61f0a2cc0" width="300"> <br />
*Note: Each sample is 2 bytes. The SD card saves these samples **Least-Significant Byte first**, meaning the sample is formatted as: byte2byte1.* <br />
4. Open the "hexToDecInvert.py in Virtual Studio Code or your preferred IDE and create a .txt file called "input.txt". Copy the text from step 3 into the file.<br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/0dcf666b-ce68-4073-8db2-893180cf7c85" width="300"> <br />
5. Run the script and copy the output in output.txt into excel/matlab to visualize <br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/9b05b3b2-527b-4f2a-9a5d-59c7c21fb344" width="300"> <br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/8319bd60-ba19-433e-813b-f2fa14054597" width="300"> <br />

## UART connection
**Find your port number in the device manager
**Baud Rate = 115200

## TODO
- Solve issue with stimulators interfering with reading data (after entering stimulator IRQ, the next recorded sample is either 0 or 65536). This can either be
a firmware issue or an analog issue related to the setup.
- Find a way to encode channel number with recording data. Alternatively, have one SD card file for each channel and having multiple open at the same time. 
- Create a more efficient post-processing technique for retrieving data from SD card
- Data at the beginning of a new SD card file seems like it belongs to previous readings. An easy solution is just to increase the size of the SD card file (by increasing the "file_seconds" parameter in config.txt and use the data later in the file. 

*Notes for Andrew* <br />
Original Project Name: Andrew_SPI_Headstage_SD_card_06_19_stim_v10_multi_stim
