# Tiva Intan USB sending project
This project sends Intan readings through a USB interface to a PC via a virtual serial port. 
## Running application
1. Flash the firmware onto the Tiva and connect a USB from the **Device USB** to the PC. *Note: This is different from the **Debug USB**. The Device USB is circled in the image below:* <br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/bcd9a847-ef26-4883-bedb-f236e3a85e95" width="300"> <br />
2. Open the usb_serial_no_sep.py on VS code or your preferred IDE. This script takes data from the virtual serial port and saves it in a .txt file. To make the script work, you must change the COM number to the virtual USB COM port of the Tiva:  <br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/42ab5d53-9654-4aaa-bac0-c44abbd85d6f" width="300"> <br />
If you are having trouble determining which COM port is the Tiva's virtual COM port, I suggest installing TeraTerm: https://ttssh2.osdn.jp/index.html.en because it allows you to see currently connected devices with descriptive names associating the device with the COM port: <br />
<img src="https://github.com/andrewabdel/Tiva_Projects/assets/93873993/eaf99723-9a9c-48b5-9d4d-0368eff627ae" width="300"> <br />
 *Note: do **NOT** display the data on the virtual serial port since displaying the data decreases the throughput and slows down your PC.* <br />

## Tiva launchpad connections with Intan LVDS adapter
Refer to `Tiva_Projects/Tiva_Intan_RHS_SD_card_project` for pinout. **However**, note that the **CS pin is on PA6 instead of PA3**. <br />
## Todo
-Currently, the max data rate is 10 Ksamples/sec since the USB sending operations happen inside the Timer0 IRQ. The USB-sending operations need to be brought out of the IRQ to achieve a higher throughput. There are issues when attempting to do this. <br />
-Enable 2-way USB communication to set Intan parameters from a GUI.



*Note for Andrew* <br />
Original Project Name: SPI_Headstage_SD_card_05_30_UART_no_SD_USB_v2
