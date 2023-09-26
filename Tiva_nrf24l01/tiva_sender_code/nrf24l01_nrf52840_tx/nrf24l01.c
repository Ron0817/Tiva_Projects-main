/*
 * nrf24l01.c
 *
 *  Created on: Jul 29, 2014
 *      Author: Rohit Dureja
 */

#include "nrf24l01.h"
#define _BV(bit) (1 << (bit))

void delayMicroseconds(uint32_t us)
{
    SysCtlDelay(us * (SysCtlClockGet() / 50000000));
}

// initialize the RF module
bool RFInit(uint32_t ui32Mode)
{
	SPIInit();

	if(ui32Mode == 0) // RX Mode
	{
		SPISetCELow(); // disable all communication

		// ----- //
		RFWriteRegister(WRITE_REG + SETUP_AW, 0x01); // Set address width to three bytes
		// set RX pipe 0 address
		SPISetCSNLow();
		SPIDataWrite(WRITE_REG + RX_ADDR_P0);
		SPIDataRead();
		SPIDataWrite(0x2C); // LSB
		SPIDataRead();
		SPIDataWrite(0x3E);
		SPIDataRead();
		SPIDataWrite(0x3E); // MSB
		SPIDataRead();
		SPISetCSNHigh();
		RFWriteRegister(WRITE_REG + EN_AA, 0x01); // enable ACK on RX pipe 0
		RFWriteRegister(WRITE_REG + EN_RXADDR, 0x01); // enable data pipe 0
		RFWriteRegister(WRITE_REG + RF_CH, 20); // set RF channel
		RFWriteRegister(WRITE_REG + RF_SETUP, 0x0F); // set data rate at 2mbps and power at 0dBm
		RFWriteRegister(WRITE_REG + DYNPD, 0x01); // enable dynamic payload length for RX pipe 0
		RFWriteRegister(WRITE_REG + FEATURE, 0x06); // enable dynamic payload length
		RFWriteRegister(WRITE_REG + CONFIG, 0x3F); // RX_DR interrupt on IRQ pin and RX mode on

		// Flush SPI RX FIFO to remove residual data
		SPIRXFlush();
		// ----- //

		SPISetCEHigh(); // enable all communication
	}
	else if(ui32Mode == 1) // TX Mode
	{

	    //***************Arduino code that interfaces with the nrf5280 MCU***************//
//	    const byte address[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; DONE
//
//	    void setup() {
//	      radio.begin();
//	      radio.openWritingPipe(address); DONE
//	      radio.enableAckPayload();   DONE
//	      radio.enableDynamicPayloads(); //DONE by calling enableAckPayload() function, it seems
//	      radio.setRetries(5,5);
//	      radio.setCRCLength(RF24_CRC_16); DONE
//	      radio.setDataRate(RF24_2MBPS); DONE
//	      radio.setPayloadSize(32); DONE
//	      radio.setChannel(2); DONE

//	    void RF24::enableAckPayload(void)
//	    {
//	        //
//	        // enable ack payload and dynamic payload features
//	        //
//
//	        //toggle_features();
//	        write_register(FEATURE, read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));
//
//	        IF_SERIAL_DEBUG(printf("FEATURE=%i\r\n", read_register(FEATURE)));
//
//	        //
//	        // Enable dynamic payload on pipes 0 & 1
//	        //
//	        write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
//	        dynamic_payloads_enabled = true;
//	        ack_payloads_enabled = true;
//	    }

//	    void RF24::enableDynamicPayloads(void)
//	    {
//	        // Enable dynamic payload throughout the system
//
//	        //toggle_features();
//	        write_register(FEATURE, read_register(FEATURE) | _BV(EN_DPL));
//
//	        IF_SERIAL_DEBUG(printf("FEATURE=%i\r\n", read_register(FEATURE)));
//
//	        // Enable dynamic payload on all pipes
//	        //
//	        // Not sure the use case of only having dynamic payload on certain
//	        // pipes, so the library does not support it.
//	        write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0));
//
//	        dynamic_payloads_enabled = true;
//	    }
	    //*******************************************************************************//
		SPISetCELow(); // disable all communication

		// ----- //
		RFWriteRegister(WRITE_REG + SETUP_AW, 0x03); // Set address width to five bytes
        RFWriteRegister(WRITE_REG + SETUP_RETR, 0x5F); // set retries to 15 and delay to 1500us
        RFWriteRegister(WRITE_REG + RF_SETUP, 0x0F); // set data rate at 2mbps and power at 0dBm

		// set RX pipe 0 address
		SPISetCSNLow();
		SPIDataWrite(WRITE_REG + RX_ADDR_P1);
		SPIDataRead();
        SPIDataWrite(0xE7); // LSB
        SPIDataRead();
        SPIDataWrite(0xE7);
        SPIDataRead();
        SPIDataWrite(0xE7);
        SPIDataRead();
        SPIDataWrite(0xE7);
        SPIDataRead();
        SPIDataWrite(0xE7); // MSB
		SPIDataRead();
		SPISetCSNHigh();
        // set TX address
        SPISetCSNLow();
        SPIDataWrite(WRITE_REG + TX_ADDR);
        SPIDataRead();
        SPIDataWrite(0xE7); // LSB
        SPIDataRead();
        SPIDataWrite(0xE7);
        SPIDataRead();
        SPIDataWrite(0xE7);
        SPIDataRead();
        SPIDataWrite(0xE7);
        SPIDataRead();
        SPIDataWrite(0xE7); // MSB
        SPIDataRead();
        SPISetCSNHigh();
        RFWriteRegister(WRITE_REG + FEATURE, 0x0); // enable dynamic payload length  //changed
        RFWriteRegister(WRITE_REG + DYNPD, 0x0);   //changed
		RFWriteRegister(WRITE_REG + EN_AA, 0x3F);  //changed
		RFWriteRegister(WRITE_REG + EN_RXADDR, 0x03);  //changed
        RFWriteRegister(WRITE_REG + RX_PW_P0, 0x20);  // set static payload size to 32 (max) bytes by default
		RFWriteRegister(WRITE_REG + RF_CH, 2); // set RF channel

		//NEW code for nrf52840 compatibility
		//write_register(FEATURE, read_register(FEATURE) | _BV(EN_ACK_PAY) | _BV(EN_DPL));
		uint32_t feature_reg = RFReadRegister(FEATURE);
		feature_reg |= (/*_BV(1)|*/ _BV(2));
		RFWriteRegister(WRITE_REG + FEATURE, feature_reg);

		//write_register(DYNPD, read_register(DYNPD) | _BV(DPL_P1) | _BV(DPL_P0));
		uint32_t DYNPD_reg = RFReadRegister(DYNPD);
		DYNPD_reg |= (_BV(1) | _BV(0));
        RFWriteRegister(WRITE_REG + DYNPD, DYNPD_reg);



		//write_register(NRF_STATUS, _BV(RX_DR) | _BV(TX_DS) | _BV(MAX_RT));
		RFWriteRegister(WRITE_REG + STATUSREG, _BV(0x6) | _BV(0x5) | _BV(0x4));

		//RFWriteRegister(WRITE_REG + CONFIG, 0x6E); // MAX_RT interrupt on IRQ and TX mode on

		//addition
		//RFWriteRegister(WRITE_REG + RX_PW_P0 + 1, 0x20);  // set static payload size to 32 (max) bytes by default


		//write_register(EN_AA, 0x3F);  // enable auto-ack on all pipes
	    //write_register(EN_RXADDR, 3); // only open RX pipes 0 & 1
		// Flush SPI RX FIFO to remove residual data
		SPIRXFlush();
		// ----- //

		//write_register(NRF_CONFIG, (_BV(EN_CRC) | _BV(CRCO)));
        RFWriteRegister(WRITE_REG + CONFIG, (_BV(0x03) | _BV(0x02)));
        //config_reg = read_register(NRF_CONFIG);
        uint32_t config_reg = RFReadRegister(CONFIG);
        //powerUp(void)
        if (!(config_reg & _BV(1))) {
            config_reg |= _BV(1);

//            //nrf52840 addition: make crc 2 bytes
//            config_reg |= _BV(2);
            //write_register(CONFIG, config_reg);
            RFWriteRegister(WRITE_REG + CONFIG, config_reg);

            // For nRF24L01+ to go from power down mode to TX or RX mode it must first pass through stand-by mode.
            // There must be a delay of Tpd2stby (see Table 16.) after the nRF24L01+ leaves power down mode before
            // the CEis set high. - Tpd2stby can be up to 5ms per the 1.0 datasheet
            //delay 5000 microseconds
            //delayMicroseconds(RF24_POWERUP_DELAY);
            //50 Mhz

            delayMicroseconds(5000);
            return config_reg == (_BV(3) | _BV(2) | _BV(1)) ? true : false;
        }

		//SPISetCEHigh(); // enable all communication
	}
}

// write into a register. returns status
uint32_t RFWriteRegister(uint32_t ui32Register, uint32_t ui32Value)
{
	uint32_t ui32Status;
	SPISetCSNLow();
	SPIDataWrite(ui32Register); // select register to write to
	ui32Status = SPIDataRead();
	SPIDataWrite(ui32Value); // write value in register
	SPIDataRead();
	SPISetCSNHigh();
	return ui32Status;
}

// read from a RF register. returns read value
uint32_t RFReadRegister(uint32_t ui32Register)
{
	uint32_t ui32Value;
	SPISetCSNLow();
	SPIDataWrite(ui32Register); // select register to read from
	ui32Value = SPIDataRead();
	SPIDataWrite(0xFF); // push dummy bits to extract value
	ui32Value = SPIDataRead();
	SPISetCSNHigh();
	return ui32Value;
}

// write to send buffer. Returns numbers of bytes written
uint32_t RFWriteSendBuffer(uint32_t *ui32Data, uint32_t ui32Bytes)
{
	uint32_t i;

	//Flush TX buffer
	SPISetCSNLow();
	SPIDataWrite(FLUSH_TX);
	SPIDataRead();
	SPISetCSNHigh();

	//SPISetCELow(); // disable all communications
	//ROM_GPIOPinWrite(GPIO_PORTB_BASE, MY_CE, 0); // disable all communications
    ROM_GPIOPinWrite(GPIO_PORTA_BASE, MY_CE, 0); // disable all communications

	SPISetCSNLow();
	SPIDataWrite(W_TX_PAYLOAD_NO_ACK);  // choose TX payload register
	SPIDataRead();
	for(i = 0 ; i < ui32Bytes ; ++i)
	{
		SPIDataWrite(ui32Data[i]); // push bytes into TX payload
		SPIDataRead();
	}
	SPISetCSNHigh();
	//SPISetCEHigh(); // enable all communication
    //ROM_GPIOPinWrite(GPIO_PORTB_BASE, MY_CE, 1); // enable all communications
    ROM_GPIOPinWrite(GPIO_PORTA_BASE, MY_CE, MY_CE); // enable all communications



	//arduino addition
//    while (!(get_status() & (_BV(TX_DS) | _BV(MAX_RT)))) {
//#if defined(FAILURE_HANDLING) || defined(RF24_LINUX)
//        if (millis() - timer > 95) {
//            errNotify();
//    #if defined(FAILURE_HANDLING)
//            return 0;
//    #else
//            delay(100);
//    #endif
//        }
//#endif

      //while(!(RFWriteRegister(0xFF, 0xFF) & (_BV(5) | _BV(4)))){
          delayMicroseconds(280);
      //}
      //SPISetCELow(); // disable all communications
      ROM_GPIOPinWrite(GPIO_PORTA_BASE, MY_CE, 0); // disable all communications
      RFWriteRegister(WRITE_REG + STATUSREG, _BV(6) | _BV(5) | _BV(4));


	// Flush SPI RX FIFO to remove residual data
	SPIRXFlush();
	return i;
}

// read from recive buffer. Returns number of bytes read
uint32_t RFReadRecieveBuffer(uint32_t *ui32Data)
{
	uint32_t ui32Bytes;
	uint32_t i;
	// Find number of bytes to read
	SPISetCSNLow();
	SPIDataWrite(R_RX_PL_WID);
	ui32Bytes = SPIDataRead();
	SPIDataWrite(0xFF);
	ui32Bytes = SPIDataRead();
	SPISetCSNHigh();

	// if bytes > 32. Some error has occurred.
	if(ui32Bytes > 32)
	{
		// Flush RX FIFO
		SPISetCSNLow();
		SPIDataWrite(FLUSH_RX);
		SPIDataRead();
		SPISetCSNHigh();
		return 0;
	}
	else
	{
		SPISetCSNLow();
		SPIDataWrite(RD_RX_PLOAD);
		SPIDataRead(); // first bytes not important contains status
		for(i = 0 ; i < ui32Bytes ; ++i)
		{
			SPIDataWrite(0xFF);
			ui32Data[i] = SPIDataRead();
		}
		SPISetCSNHigh();
		return ui32Bytes;
	}
}
