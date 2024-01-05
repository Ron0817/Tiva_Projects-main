/*
 * spi.c
 *
 *  Created on: Jul 29, 2014
 *      Author: Rohit Dureja
 *  Modified on: Dec 13, 2023
 *      Author: Rongxuan Du
 */

#include "spi.h"

// function to write a byte to the SPI port
void SPIDataWrite(uint32_t ui32DataTx)
{
    ROM_SSIDataPut(SPI_BASE, (uint8_t)ui32DataTx);
}


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



//void SPISetCSNLow(void)
//{
//	ROM_GPIOPinWrite(SPI_PORTD_BASE, CSN, 0);
//}
//
//void SPISetCSNHigh(void)
//{
//	ROM_GPIOPinWrite(SPI_PORTD_BASE, CSN, CSN);
//}
//
//void SPISetCELow()
//{
//	ROM_GPIOPinWrite(CE_BASE, CE, 0);
//}
//
//void SPISetCEHigh()
//{
//	ROM_GPIOPinWrite(CE_BASE, CE, CE);
//}
