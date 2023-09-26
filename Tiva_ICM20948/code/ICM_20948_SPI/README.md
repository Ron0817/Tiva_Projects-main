# Tiva ICM20948 SPI project
Project that communicates with the ICM-20948 9‑Axis MEMS MotionTracking Device through SPI. Currently, the code already takes care of the low-level SPI and communication with the device. 
## Application-specific APIs
Application-Specific APIs have been created to simplify the process of communicating with the device through SPI without needing to know low-level SPI details associated with the Ti Tiva. 
They are: <br />
`ICM_SPI_Write(uint32_t ui32Register, uint32_t ui32Value)` which writes a specific value to a register, and <br />
`ICM_SPI_Read(uint32_t ui32Register)` which returns the value of a register <br />

These APIs can be used to initialize the device and retrieve sensor data <br />

 ## Launchpad Connections
**Devices used**<br />
- Tiva C series TM4C123G LaunchPad: https://www.ti.com/tool/EK-TM4C123GXL
- GY-912 BMP388 ICM20948 10DOF Accelerometer compass nine axis sensor module: https://www.aliexpress.com/item/1005002965140683.html <br />
Pinout: <br />
- PB2 -> CS_ISM  <br />
- PD2 -> SDO/SAO <br />
- PD3 -> SDA <br/>
- PD0 -> SCL <br />
- GND -> GND <br />
- VIN -> VBUS <br />

*Notes for Andrew*
Original Project Name: ICM_20948_v2
