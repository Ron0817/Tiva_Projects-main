/*
 * spi.c
 *
 *  Created on: Jul 29, 2014
 *      Author: Rohit Dureja
 */

#include "spi.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"


void SPIInit(void)
{
	uint32_t ui32ResidualData;
	// enable SSI3 and GPIOD peripherals
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI3);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // CE and IRQ are on Port B

	//my addition
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // CE and IRQ are on Port B

	// Configure GPIO pins for special functions except CSN, CE and IRQ which are under software control
	GPIOPinConfigure(GPIO_PD0_SSI3CLK);
	GPIOPinConfigure(GPIO_PD2_SSI3RX);
	GPIOPinConfigure(GPIO_PD3_SSI3TX);
	ROM_GPIOPinTypeSSI(SPI_PORT_BASE, SCK | MISO | MOSI);

	// Configure GPIO pins for CE, CSN and IRQ
	ROM_GPIOPinTypeGPIOOutput(SPI_PORT_BASE, CSN);
	ROM_GPIOPinTypeGPIOOutput(CE_BASE, CE);

	//my addition
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, MY_CE);


	//Configure and enable SSI port
	// Use internal 16Mhz RC oscillator as SSI clock source
	ROM_SSIClockSourceSet(SPI_BASE, SSI_CLOCK_PIOSC);
	ROM_SSIConfigSetExpClk(SPI_BASE, 16000000, SSI_FRF_MOTO_MODE_0,
						SSI_MODE_MASTER, 8000000, 8);
	ROM_SSIEnable(SPI_BASE);

	// Read any residual data on the SSI port to clear buffer
	while(SSIDataGetNonBlocking(SPI_BASE, &ui32ResidualData))
	{

	}
}

// function to write a byte to the SPI port
void SPIDataWrite(uint32_t ui32DataTx)
{
    ROM_SSIDataPut(SPI_BASE, ui32DataTx);
}

// function to write a byte to the SPI port
uint32_t ICM_SPI_Write(uint32_t ui32Register, uint32_t ui32Value)
{
    uint32_t ui32ResidualData;
    ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);
    SPIDataWrite(ui32Register);
    SPIDataRead();
    SPIDataWrite(ui32Value);
    SPIDataRead();
    ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
}

uint32_t ICM_SPI_Read(uint32_t ui32Register)
{
    uint32_t readVal;
    //need the first bit to be a 1 to indicate a read operation
    ui32Register = ui32Register | 0x80;
    ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);
    SPIDataWrite(ui32Register);
    readVal = SPIDataRead();
    //need to send dummy data, this is actually when we receive the proper return value from the register
    SPIDataWrite(0x0);
    readVal = SPIDataRead();
    ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2);
    return readVal;
}

//uint32_t RFWriteRegister(uint32_t ui32Register, uint32_t ui32Value)
//{
//    uint32_t ui32Status;
//    SPISetCSNLow();
//    SPIDataWrite(ui32Register); // select register to write to
//    ui32Status = SPIDataRead();
//    SPIDataWrite(ui32Value); // write value in register
//    SPIDataRead();
//    SPISetCSNHigh();
//    return ui32Status;
//}


// function to read data from SPI port
uint32_t SPIDataRead(void)
{
	uint32_t ui32DataRx;
	ROM_SSIDataGet(SPI_BASE, &ui32DataRx);
	return ui32DataRx;
}

// function to flush RX FIFO
void SPIRXFlush()
{
	uint32_t ui32ResidualData;
	// Read any residual data on the SSI port to clear buffer
	while(SSIDataGetNonBlocking(SPI_BASE, &ui32ResidualData))
	{
	}
}

void SPISetCSNLow(void)
{
	ROM_GPIOPinWrite(SPI_PORT_BASE, CSN, 0);
}

void SPISetCSNHigh(void)
{
	ROM_GPIOPinWrite(SPI_PORT_BASE, CSN, CSN);
}

void SPISetCELow()
{
	ROM_GPIOPinWrite(CE_BASE, CE, 0);
}

void SPISetCEHigh()
{
	ROM_GPIOPinWrite(CE_BASE, CE, CE);
}
