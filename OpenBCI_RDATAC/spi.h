/*
 * spi.h
 *
 *  Created on: Jul 29, 2014
 *      Author: Rohit Dureja
 *  Modified on: Dec 13, 2023
 *      Author: Rongxuan Du
 */

#ifndef SPI_H_
#define SPI_H_
#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"

// Defines for SPI pin mappings
//#define CE 		GPIO_PIN_6
#define CSN		GPIO_PIN_2
#define SCK		GPIO_PIN_0
#define MOSI 	GPIO_PIN_3
#define MISO 	GPIO_PIN_2
#define IRQ 	GPIO_PIN_7

//my own CE pin
//#define MY_CE   GPIO_PIN_3    //PA3

// Defines for SPI pin port mappings
#define CS_BASE 		GPIO_PORTB_BASE
#define SPI_PORTD_BASE 	GPIO_PORTD_BASE // define for SPI gpio port base
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


void ADS1299_SPI_Init(void);
uint32_t ADS1299_SPI_Read(uint32_t );
void ADS1299_SPI_Write(uint32_t, uint32_t );

#endif /* SPI_H_ */
