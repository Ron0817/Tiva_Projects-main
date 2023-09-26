/*
 * spi.h
 *
 *  Created on: Jul 29, 2014
 *      Author: Rohit Dureja
 */

#ifndef SPI_H_
#define SPI_H_
#include <stdbool.h>
#include <stdint.h>

// Defines for SPI pin mappings
#define CE 		GPIO_PIN_6
#define CSN		GPIO_PIN_1
#define SCK		GPIO_PIN_0
#define MOSI 	GPIO_PIN_3
#define MISO 	GPIO_PIN_2
#define IRQ 	GPIO_PIN_7

//my own CE pin
#define MY_CE   GPIO_PIN_3    //PA3

// Defines for SPI pin port mappings
#define CE_BASE 		GPIO_PORTB_BASE
#define SPI_PORT_BASE 	GPIO_PORTD_BASE // define for SPI gpio port base
#define SPI_BASE 		SSI3_BASE // define for SPI base
#define IRQ_BASE 		GPIO_PORTB_BASE

// Function definitions

// initialise SPI for operation
void SPIInit(void);

// initiate data write on the SPI bus
void SPIDataWrite(uint32_t);

//initiate data read on the SPI bus
uint32_t SPIDataRead();

// flush SPI RX buffer
void SPIRXFlush();

// function to control CSN
void SPISetCSNLow(void);
void SPISetCSNHigh(void);

// function to control CE
void SPISetCELow();
void SPISetCEHigh();

uint32_t ICM_SPI_Read(uint32_t ui32Register);
uint32_t ICM_SPI_Write(uint32_t ui32Register, uint32_t ui32Value);


#endif /* SPI_H_ */
